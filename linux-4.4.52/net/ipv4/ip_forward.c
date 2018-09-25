/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		The IP forwarding functionality.
 *
 * Authors:	see ip.c
 *
 * Fixes:
 *		Many		:	Split from ip.c , see ip_input.c for
 *					history.
 *		Dave Gregorich	:	NULL ip_rt_put fix for multicast
 *					routing.
 *		Jos Vos		:	Add call_out_firewall before sending,
 *					use output device for accounting.
 *		Jos Vos		:	Call forward firewall after routing
 *					(always use output device).
 *		Mike McLagan	:	Routing by source
 */

#include <linux/types.h>
#include <linux/mm.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <linux/netdevice.h>
#include <linux/slab.h>
#include <net/sock.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/icmp.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/netfilter_ipv4.h>
#include <net/checksum.h>
#include <linux/route.h>
#include <net/route.h>
#include <net/xfrm.h>

/* 判断skb中承载的ipv4报文是否超出mtu大小
 * @返回值：    true    - 超出
 *              false   - 不超出
 */
static bool ip_exceeds_mtu(const struct sk_buff *skb, unsigned int mtu)
{
    // 如果skb中数据总厂不超过mtu，则肯定不超
	if (skb->len <= mtu)
		return false;

    // 数据总长超出mtu的情况下，如果该ipv4报文中没有置位DF标志，意味着允许被分片，所以相当于不超
	if (unlikely((ip_hdr(skb)->frag_off & htons(IP_DF)) == 0))
		return false;

    // 程序运行到这里意味着数据总长超出mtu、DF置位
	/* original fragment exceeds mtu and DF is set 
     * 如果分片的最大长度超过mtu，则判定超出
     * */
	if (unlikely(IPCB(skb)->frag_max_size > mtu))
		return true;

    // 如果该skb设置了忽略DF位，意味着允许被分片，所以也相当于不超
	if (skb->ignore_df)
		return false;

    // 如果该skb支持gso，且gso数据包在L3层中的单个分段长不超过mtu，则认为不超
	if (skb_is_gso(skb) && skb_gso_network_seglen(skb) <= mtu)
		return false;

    // 其他情况下都认为超出
	return true;
}


static int ip_forward_finish(struct net *net, struct sock *sk, struct sk_buff *skb)
{
	struct ip_options *opt	= &(IPCB(skb)->opt);

	IP_INC_STATS_BH(net, IPSTATS_MIB_OUTFORWDATAGRAMS);
	IP_ADD_STATS_BH(net, IPSTATS_MIB_OUTOCTETS, skb->len);

	if (unlikely(opt->optlen))
		ip_forward_options(skb);

	skb_sender_cpu_clear(skb);
	return dst_output(net, sk, skb);
}

// ip报文转发
int ip_forward(struct sk_buff *skb)
{
	u32 mtu;
	struct iphdr *iph;	/* Our header */
	struct rtable *rt;	/* Route we use */
    // 获取该ip报文的选项集合
	struct ip_options *opt	= &(IPCB(skb)->opt);
	struct net *net;

	/* that should never happen  要转发的ip报文其目的mac必然是发往本机的 */
	if (skb->pkt_type != PACKET_HOST)
		goto drop;

	if (unlikely(skb->sk))
		goto drop;

    // 如果该skb关联的网络设备设置了LRO，则丢弃
	if (skb_warn_if_lro(skb))
		goto drop;

    // ipsec安全规则检查
	if (!xfrm4_policy_check(NULL, XFRM_POLICY_FWD, skb))
		goto drop;

    // 若设置了Router Alert选项，则调用ip_call_ra_chain方法，若处理成功则直接返回
	if (IPCB(skb)->opt.router_alert && ip_call_ra_chain(skb))
		return NET_RX_SUCCESS;

	skb_forward_csum(skb);
	net = dev_net(skb->dev);

	/*
	 *	According to the RFC, we must first decrease the TTL field. If
	 *	that reaches zero, we must reply an ICMP control message telling
	 *	that the packet's lifetime expired.
     *	如果TTL<=1，意味着该报文寿命已到，必须丢弃，并且将发送一个发送TTL耗尽的icmp消息给源地址
	 */
	if (ip_hdr(skb)->ttl <= 1)
		goto too_many_hops;

    // 检查是否由ipsec策略要转发该ip报文
	if (!xfrm4_route_forward(skb))
		goto drop;

    // 获取路由表项
	rt = skb_rtable(skb);

	if (opt->is_strictroute && rt->rt_uses_gateway)
		goto sr_failed;

	IPCB(skb)->flags |= IPSKB_FORWARDED;
    // 获取该路由表项出口网络设备支持的mtu
	mtu = ip_dst_mtu_maybe_forward(&rt->dst, true);
    // 判断该ipv4报文是否超出mtu大小，对于超出的报文，这里会发送一个"目的不可达-需要分片"的icmp报文
	if (ip_exceeds_mtu(skb, mtu)) {
		IP_INC_STATS(net, IPSTATS_MIB_FRAGFAILS);
		icmp_send(skb, ICMP_DEST_UNREACH, ICMP_FRAG_NEEDED,
			  htonl(mtu));
		goto drop;
	}

	/* We are about to mangle packet. Copy it! */
	if (skb_cow(skb, LL_RESERVED_SPACE(rt->dst.dev)+rt->dst.header_len))
		goto drop;
	iph = ip_hdr(skb);

	/* Decrease ttl after skb cow done */
	ip_decrease_ttl(iph);

	/*
	 *	We now generate an ICMP HOST REDIRECT giving the route
	 *	we calculated.
	 */
	if (IPCB(skb)->flags & IPSKB_DOREDIRECT && !opt->srr &&
	    !skb_sec_path(skb))
		ip_rt_send_redirect(skb);

	skb->priority = rt_tos2priority(iph->tos);

	return NF_HOOK(NFPROTO_IPV4, NF_INET_FORWARD,
		       net, NULL, skb, skb->dev, rt->dst.dev,
		       ip_forward_finish);

sr_failed:
	/*
	 *	Strict routing permits no gatewaying
	 */
	 icmp_send(skb, ICMP_DEST_UNREACH, ICMP_SR_FAILED, 0);
	 goto drop;

too_many_hops:
	/* Tell the sender its packet died... */
	IP_INC_STATS_BH(net, IPSTATS_MIB_INHDRERRORS);
	icmp_send(skb, ICMP_TIME_EXCEEDED, ICMP_EXC_TTL, 0);
drop:
	kfree_skb(skb);
	return NET_RX_DROP;
}
