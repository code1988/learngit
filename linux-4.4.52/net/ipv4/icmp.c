/*
 *	NET3:	Implementation of the ICMP protocol layer.
 *
 *		Alan Cox, <alan@lxorguk.ukuu.org.uk>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *	Some of the function names and the icmp unreach table for this
 *	module were derived from [icmp.c 1.0.11 06/02/93] by
 *	Ross Biro, Fred N. van Kempen, Mark Evans, Alan Cox, Gerhard Koerting.
 *	Other than that this module is a complete rewrite.
 *
 *	Fixes:
 *	Clemens Fruhwirth	:	introduce global icmp rate limiting
 *					with icmp type masking ability instead
 *					of broken per type icmp timeouts.
 *		Mike Shaver	:	RFC1122 checks.
 *		Alan Cox	:	Multicast ping reply as self.
 *		Alan Cox	:	Fix atomicity lockup in ip_build_xmit
 *					call.
 *		Alan Cox	:	Added 216,128 byte paths to the MTU
 *					code.
 *		Martin Mares	:	RFC1812 checks.
 *		Martin Mares	:	Can be configured to follow redirects
 *					if acting as a router _without_ a
 *					routing protocol (RFC 1812).
 *		Martin Mares	:	Echo requests may be configured to
 *					be ignored (RFC 1812).
 *		Martin Mares	:	Limitation of ICMP error message
 *					transmit rate (RFC 1812).
 *		Martin Mares	:	TOS and Precedence set correctly
 *					(RFC 1812).
 *		Martin Mares	:	Now copying as much data from the
 *					original packet as we can without
 *					exceeding 576 bytes (RFC 1812).
 *	Willy Konynenberg	:	Transparent proxying support.
 *		Keith Owens	:	RFC1191 correction for 4.2BSD based
 *					path MTU bug.
 *		Thomas Quinot	:	ICMP Dest Unreach codes up to 15 are
 *					valid (RFC 1812).
 *		Andi Kleen	:	Check all packet lengths properly
 *					and moved all kfree_skb() up to
 *					icmp_rcv.
 *		Andi Kleen	:	Move the rate limit bookkeeping
 *					into the dest entry and use a token
 *					bucket filter (thanks to ANK). Make
 *					the rates sysctl configurable.
 *		Yu Tianli	:	Fixed two ugly bugs in icmp_send
 *					- IP option length was accounted wrongly
 *					- ICMP header length was not accounted
 *					  at all.
 *              Tristan Greaves :       Added sysctl option to ignore bogus
 *              			broadcast responses from broken routers.
 *
 * To Fix:
 *
 *	- Should use skb_pull() instead of all the manual checking.
 *	  This would also greatly simply some upper layer error handlers. --AK
 *
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/types.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/fcntl.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include <linux/string.h>
#include <linux/netfilter_ipv4.h>
#include <linux/slab.h>
#include <net/snmp.h>
#include <net/ip.h>
#include <net/route.h>
#include <net/protocol.h>
#include <net/icmp.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/raw.h>
#include <net/ping.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/errno.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <net/checksum.h>
#include <net/xfrm.h>
#include <net/inet_common.h>
#include <net/ip_fib.h>
#include <net/l3mdev.h>

/*
 *	Build xmit assembly blocks
 *	用于构造icmpv4发送报文的配置结构
 */
struct icmp_bxm {
	struct sk_buff *skb;    // 在icmp_reply中指向承载了收到的请求数据包的skb；在icmp_send中指向承载了要发送数据包的skb
	int offset;             // 网络层头部和skb->data指针间的偏移量
	int data_len;           // icmp数据包的有效载荷长度

	struct {
		struct icmphdr icmph;       // icmp头信息
		__be32	       times[3];    // 包含3个时间戳的数组
	} data;
	int head_len;                   // icmp头长，通常就是icmphdr长(对于时间戳报文，还包括12字节时间戳信息)
	struct ip_options_data replyopts;
};

/* An array of errno for error messages from dest unreach. */
/* RFC 1122: 3.2.2.1 States that NET_UNREACH, HOST_UNREACH and SR_FAILED MUST be considered 'transient errs'. */

const struct icmp_err icmp_err_convert[] = {
	{
		.errno = ENETUNREACH,	/* ICMP_NET_UNREACH */
		.fatal = 0,
	},
	{
		.errno = EHOSTUNREACH,	/* ICMP_HOST_UNREACH */
		.fatal = 0,
	},
	{
		.errno = ENOPROTOOPT	/* ICMP_PROT_UNREACH */,
		.fatal = 1,
	},
	{
		.errno = ECONNREFUSED,	/* ICMP_PORT_UNREACH */
		.fatal = 1,
	},
	{
		.errno = EMSGSIZE,	/* ICMP_FRAG_NEEDED */
		.fatal = 0,
	},
	{
		.errno = EOPNOTSUPP,	/* ICMP_SR_FAILED */
		.fatal = 0,
	},
	{
		.errno = ENETUNREACH,	/* ICMP_NET_UNKNOWN */
		.fatal = 1,
	},
	{
		.errno = EHOSTDOWN,	/* ICMP_HOST_UNKNOWN */
		.fatal = 1,
	},
	{
		.errno = ENONET,	/* ICMP_HOST_ISOLATED */
		.fatal = 1,
	},
	{
		.errno = ENETUNREACH,	/* ICMP_NET_ANO	*/
		.fatal = 1,
	},
	{
		.errno = EHOSTUNREACH,	/* ICMP_HOST_ANO */
		.fatal = 1,
	},
	{
		.errno = ENETUNREACH,	/* ICMP_NET_UNR_TOS */
		.fatal = 0,
	},
	{
		.errno = EHOSTUNREACH,	/* ICMP_HOST_UNR_TOS */
		.fatal = 0,
	},
	{
		.errno = EHOSTUNREACH,	/* ICMP_PKT_FILTERED */
		.fatal = 1,
	},
	{
		.errno = EHOSTUNREACH,	/* ICMP_PREC_VIOLATION */
		.fatal = 1,
	},
	{
		.errno = EHOSTUNREACH,	/* ICMP_PREC_CUTOFF */
		.fatal = 1,
	},
};
EXPORT_SYMBOL(icmp_err_convert);

/*
 *	ICMP control array. This specifies what to do with each ICMP.
 * icmpv4消息类型相关的处理单元
 */
struct icmp_control {
	bool (*handler)(struct sk_buff *skb);
	short   error;		/* This ICMP is classed as an error message 
                            为1时表示这类icmpv4消息属于错误信息
                            为0时表示这类icmpv4消息属于查询消息
                            */
};

static const struct icmp_control icmp_pointers[NR_ICMP_TYPES+1];

/*
 *	The ICMP socket(s). This is the most convenient way to flow control
 *	our ICMP output as well as maintain a clean interface throughout
 *	all layers. All Socketless IP sends will soon be gone.
 *	返回指定net命名空间中当前cpu对应的icmp内核套接字
 *
 *	On SMP we have one ICMP socket per-cpu.
 */
static struct sock *icmp_sk(struct net *net)
{
	return *this_cpu_ptr(net->ipv4.icmp_sk);
}

// 返回指定net命名空间中当前cpu对应的icmp内核套接字
static inline struct sock *icmp_xmit_lock(struct net *net)
{
	struct sock *sk;

    // 关闭软中断?
	local_bh_disable();

    // 返回该net命名空间中当前cpu对应的icmp内核套接字
	sk = icmp_sk(net);

    // 对该icmp内核套接字上自旋锁
	if (unlikely(!spin_trylock(&sk->sk_lock.slock))) {
		/* This can happen if the output path signals a
		 * dst_link_failure() for an outgoing ICMP packet.
		 */
		local_bh_enable();
		return NULL;
	}
	return sk;
}

static inline void icmp_xmit_unlock(struct sock *sk)
{
	spin_unlock_bh(&sk->sk_lock.slock);
}

int sysctl_icmp_msgs_per_sec __read_mostly = 1000;
int sysctl_icmp_msgs_burst __read_mostly = 50;

static struct {
	spinlock_t	lock;
	u32		credit;
	u32		stamp;
} icmp_global = {
	.lock		= __SPIN_LOCK_UNLOCKED(icmp_global.lock),
};

/**
 * icmp_global_allow - Are we allowed to send one more ICMP message ?
 *
 * Uses a token bucket to limit our ICMP messages to sysctl_icmp_msgs_per_sec.
 * Returns false if we reached the limit and can not send another packet.
 * Note: called with BH disabled
 */
bool icmp_global_allow(void)
{
	u32 credit, delta, incr = 0, now = (u32)jiffies;
	bool rc = false;

	/* Check if token bucket is empty and cannot be refilled
	 * without taking the spinlock.
	 */
	if (!icmp_global.credit) {
		delta = min_t(u32, now - icmp_global.stamp, HZ);
		if (delta < HZ / 50)
			return false;
	}

	spin_lock(&icmp_global.lock);
	delta = min_t(u32, now - icmp_global.stamp, HZ);
	if (delta >= HZ / 50) {
		incr = sysctl_icmp_msgs_per_sec * delta / HZ ;
		if (incr)
			icmp_global.stamp = now;
	}
	credit = min_t(u32, icmp_global.credit + incr, sysctl_icmp_msgs_burst);
	if (credit) {
		credit--;
		rc = true;
	}
	icmp_global.credit = credit;
	spin_unlock(&icmp_global.lock);
	return rc;
}
EXPORT_SYMBOL(icmp_global_allow);

/*
 *	Send an ICMP frame.
 */
/* 检查是否当前允许发送icmp报文，显然本函数用来实现icmp的速率限制功能
 * @返回值：    true    - 允许发送
 *              false   - 不允许发送
 */
static bool icmpv4_xrlim_allow(struct net *net, struct rtable *rt,
			       struct flowi4 *fl4, int type, int code)
{
	struct dst_entry *dst = &rt->dst;
	bool rc = true;

	if (type > NR_ICMP_TYPES)
		goto out;

	/* Don't limit PMTU discovery. */
	if (type == ICMP_DEST_UNREACH && code == ICMP_FRAG_NEEDED)
		goto out;

	/* No rate limit on loopback */
	if (dst->dev && (dst->dev->flags&IFF_LOOPBACK))
		goto out;

	/* Limit if icmp type is enabled in ratemask. */
	if (!((1 << type) & net->ipv4.sysctl_icmp_ratemask))
		goto out;

	rc = false;
	if (icmp_global_allow()) {
		int vif = l3mdev_master_ifindex(dst->dev);
		struct inet_peer *peer;

		peer = inet_getpeer_v4(net->ipv4.peers, fl4->daddr, vif, 1);
		rc = inet_peer_xrlim_allow(peer,
					   net->ipv4.sysctl_icmp_ratelimit);
		if (peer)
			inet_putpeer(peer);
	}
out:
	return rc;
}

/*
 *	Maintain the counters used in the SNMP statistics for outgoing ICMP
 */
void icmp_out_count(struct net *net, unsigned char type)
{
	ICMPMSGOUT_INC_STATS(net, type);
	ICMP_INC_STATS(net, ICMP_MIB_OUTMSGS);
}

/*
 *	Checksum each fragment, and on the first include the headers and final
 *	checksum.
 */
static int icmp_glue_bits(void *from, char *to, int offset, int len, int odd,
			  struct sk_buff *skb)
{
	struct icmp_bxm *icmp_param = (struct icmp_bxm *)from;
	__wsum csum;

	csum = skb_copy_and_csum_bits(icmp_param->skb,
				      icmp_param->offset + offset,
				      to, len, 0);

	skb->csum = csum_block_add(skb->csum, csum, odd);
	if (icmp_pointers[icmp_param->data.icmph.type].error)
		nf_ct_attach(skb, icmp_param->skb);
	return 0;
}

// 发送icmp报文，icmp查询消息和错误消息最终都要调用本函数完成icmp报文发送
static void icmp_push_reply(struct icmp_bxm *icmp_param,
			    struct flowi4 *fl4,
			    struct ipcm_cookie *ipc, struct rtable **rt)
{
	struct sock *sk;
	struct sk_buff *skb;

    // 获取发送该icmp报文的内核icmp套接字
	sk = icmp_sk(dev_net((*rt)->dst.dev));
    // 将数据包递交给L3层
	if (ip_append_data(sk, fl4, icmp_glue_bits, icmp_param,
			   icmp_param->data_len+icmp_param->head_len,
			   icmp_param->head_len,
			   ipc, rt, MSG_DONTWAIT) < 0) {
		ICMP_INC_STATS_BH(sock_net(sk), ICMP_MIB_OUTERRORS);
		ip_flush_pending_frames(sk);
	} else if ((skb = skb_peek(&sk->sk_write_queue)) != NULL) {
		struct icmphdr *icmph = icmp_hdr(skb);
		__wsum csum = 0;
		struct sk_buff *skb1;

		skb_queue_walk(&sk->sk_write_queue, skb1) {
			csum = csum_add(csum, skb1->csum);
		}
		csum = csum_partial_copy_nocheck((void *)&icmp_param->data,
						 (char *)icmph,
						 icmp_param->head_len, csum);
		icmph->checksum = csum_fold(csum);
		skb->ip_summed = CHECKSUM_NONE;
		ip_push_pending_frames(sk, fl4);
	}
}

/*
 *	Driving logic for building and sending ICMP messages.
 *	构建icmp回复报文并发送
 */
static void icmp_reply(struct icmp_bxm *icmp_param, struct sk_buff *skb)
{
	struct ipcm_cookie ipc;
    // 获取该skb关联的路由表项
	struct rtable *rt = skb_rtable(skb);
	struct net *net = dev_net(rt->dst.dev);
	struct flowi4 fl4;
	struct sock *sk;
	struct inet_sock *inet;
	__be32 daddr, saddr;
	u32 mark = IP4_REPLY_MARK(net, skb->mark);

    // 收集ip选项字段的信息
	if (ip_options_echo(&icmp_param->replyopts.opt.opt, skb))
		return;

    // 返回该net命名空间中当前cpu对应的icmp内核套接字
	sk = icmp_xmit_lock(net);
	if (!sk)
		return;
    // 获取该sock结构对应的inet_sock结构
	inet = inet_sk(sk);

	icmp_param->data.icmph.checksum = 0;

	inet->tos = ip_hdr(skb)->tos;
	sk->sk_mark = mark;
	daddr = ipc.addr = ip_hdr(skb)->saddr;
	saddr = fib_compute_spec_dst(skb);
	ipc.opt = NULL;
	ipc.tx_flags = 0;
	ipc.ttl = 0;
	ipc.tos = -1;

	if (icmp_param->replyopts.opt.opt.optlen) {
		ipc.opt = &icmp_param->replyopts.opt;
		if (ipc.opt->opt.srr)
			daddr = icmp_param->replyopts.opt.opt.faddr;
	}
	memset(&fl4, 0, sizeof(fl4));
	fl4.daddr = daddr;
	fl4.saddr = saddr;
	fl4.flowi4_mark = mark;
	fl4.flowi4_tos = RT_TOS(ip_hdr(skb)->tos);
	fl4.flowi4_proto = IPPROTO_ICMP;
	fl4.flowi4_oif = l3mdev_master_ifindex(skb->dev);
	security_skb_classify_flow(skb, flowi4_to_flowi(&fl4));
	rt = ip_route_output_key(net, &fl4);
	if (IS_ERR(rt))
		goto out_unlock;
	if (icmpv4_xrlim_allow(net, rt, &fl4, icmp_param->data.icmph.type,
			       icmp_param->data.icmph.code))
		icmp_push_reply(icmp_param, &fl4, &ipc, &rt);
	ip_rt_put(rt);
out_unlock:
	icmp_xmit_unlock(sk);
}

#ifdef CONFIG_IP_ROUTE_MULTIPATH

/* Source and destination is swapped. See ip_multipath_icmp_hash */
static int icmp_multipath_hash_skb(const struct sk_buff *skb)
{
	const struct iphdr *iph = ip_hdr(skb);

	return fib_multipath_hash(iph->daddr, iph->saddr);
}

#else

#define icmp_multipath_hash_skb(skb) (-1)

#endif

static struct rtable *icmp_route_lookup(struct net *net,
					struct flowi4 *fl4,
					struct sk_buff *skb_in,
					const struct iphdr *iph,
					__be32 saddr, u8 tos, u32 mark,
					int type, int code,
					struct icmp_bxm *param)
{
	struct rtable *rt, *rt2;
	struct flowi4 fl4_dec;
	int err;

	memset(fl4, 0, sizeof(*fl4));
	fl4->daddr = (param->replyopts.opt.opt.srr ?
		      param->replyopts.opt.opt.faddr : iph->saddr);
	fl4->saddr = saddr;
	fl4->flowi4_mark = mark;
	fl4->flowi4_tos = RT_TOS(tos);
	fl4->flowi4_proto = IPPROTO_ICMP;
	fl4->fl4_icmp_type = type;
	fl4->fl4_icmp_code = code;
	fl4->flowi4_oif = l3mdev_master_ifindex(skb_in->dev);

	security_skb_classify_flow(skb_in, flowi4_to_flowi(fl4));
	rt = __ip_route_output_key_hash(net, fl4,
					icmp_multipath_hash_skb(skb_in));
	if (IS_ERR(rt))
		return rt;

	/* No need to clone since we're just using its address. */
	rt2 = rt;

	rt = (struct rtable *) xfrm_lookup(net, &rt->dst,
					   flowi4_to_flowi(fl4), NULL, 0);
	if (!IS_ERR(rt)) {
		if (rt != rt2)
			return rt;
	} else if (PTR_ERR(rt) == -EPERM) {
		rt = NULL;
	} else
		return rt;

	err = xfrm_decode_session_reverse(skb_in, flowi4_to_flowi(&fl4_dec), AF_INET);
	if (err)
		goto relookup_failed;

	if (inet_addr_type_dev_table(net, skb_in->dev,
				     fl4_dec.saddr) == RTN_LOCAL) {
		rt2 = __ip_route_output_key(net, &fl4_dec);
		if (IS_ERR(rt2))
			err = PTR_ERR(rt2);
	} else {
		struct flowi4 fl4_2 = {};
		unsigned long orefdst;

		fl4_2.daddr = fl4_dec.saddr;
		rt2 = ip_route_output_key(net, &fl4_2);
		if (IS_ERR(rt2)) {
			err = PTR_ERR(rt2);
			goto relookup_failed;
		}
		/* Ugh! */
		orefdst = skb_in->_skb_refdst; /* save old refdst */
		skb_dst_set(skb_in, NULL);
		err = ip_route_input(skb_in, fl4_dec.daddr, fl4_dec.saddr,
				     RT_TOS(tos), rt2->dst.dev);

		dst_release(&rt2->dst);
		rt2 = skb_rtable(skb_in);
		skb_in->_skb_refdst = orefdst; /* restore old refdst */
	}

	if (err)
		goto relookup_failed;

	rt2 = (struct rtable *) xfrm_lookup(net, &rt2->dst,
					    flowi4_to_flowi(&fl4_dec), NULL,
					    XFRM_LOOKUP_ICMP);
	if (!IS_ERR(rt2)) {
		dst_release(&rt->dst);
		memcpy(fl4, &fl4_dec, sizeof(*fl4));
		rt = rt2;
	} else if (PTR_ERR(rt2) == -EPERM) {
		if (rt)
			dst_release(&rt->dst);
		return rt2;
	} else {
		err = PTR_ERR(rt2);
		goto relookup_failed;
	}
	return rt;

relookup_failed:
	if (rt)
		return rt;
	return ERR_PTR(err);
}

/*
 *	Send an ICMP message in response to a situation
 *	发送一个指定类型指定编号的icmpv4错误消息
 *	@skb_in 承载的是触发源消息
 *	@info   本参数只用于下列情况之一：
 *	            对于ICMP_PARAMETERPROB类型消息，本参数表示ipv4头中发生分析问题的位置的偏移量;
 *	            对于ICMP_DEST_UNREACH-ICMP_FRAG_NEEDED类型消息，本参数表示mtu
 *	            对于ICMP_REDIRECT-ICMP_REDIR_HOST类型消息，本参数表示ipv4头中的目标ip地址
 *
 *	RFC 1122: 3.2.2	MUST send at least the IP header and 8 bytes of header.
 *		  MAY send more (we do).
 *			MUST NOT change this header information.
 *			MUST NOT reply to a multicast/broadcast IP address.
 *			MUST NOT reply to a multicast/broadcast MAC address.
 *			MUST reply to only the first fragment.
 */

void icmp_send(struct sk_buff *skb_in, int type, int code, __be32 info)
{
	struct iphdr *iph;
	int room;
	struct icmp_bxm *icmp_param;
	struct rtable *rt = skb_rtable(skb_in);
	struct ipcm_cookie ipc;
	struct flowi4 fl4;
	__be32 saddr;
	u8  tos;
	u32 mark;
	struct net *net;
	struct sock *sk;

	if (!rt)
		goto out;
	net = dev_net(rt->dst.dev);

	/*
	 *	Find the original header. It is expected to be valid, of course.
	 *	Check this, icmp_send is called from the most obscure devices
	 *	sometimes.
	 */
	iph = ip_hdr(skb_in);

    // skb中原始数据合法性检查
	if ((u8 *)iph < skb_in->head ||
	    (skb_network_header(skb_in) + sizeof(*iph)) >
	    skb_tail_pointer(skb_in))
		goto out;

	/*
	 *	No replies to physical multicast/broadcast
     *	如果skb中原始报文的目的mac不是本机，意味着是mac组播或mac广播报文，则终止本次icmp发送
	 */
	if (skb_in->pkt_type != PACKET_HOST)
		goto out;

	/*
	 *	Now check at the protocol level
	 */
	if (rt->rt_flags & (RTCF_BROADCAST | RTCF_MULTICAST))
		goto out;

	/*
	 *	Only reply to fragment 0. We byte re-order the constant
	 *	mask for efficiency.
     *	icmp只会针对分片偏移量为0的报文进行回复
	 */
	if (iph->frag_off & htons(IP_OFFSET))
		goto out;

	/*
	 *	If we send an ICMP error to an ICMP error a mess would result..
     *	如果收到原始报文是icmp错误消息，则不需要发送icmp错误消息
	 */
	if (icmp_pointers[type].error) {
		/*
		 *	We are an error, check if we are replying to an
		 *	ICMP error
		 */
		if (iph->protocol == IPPROTO_ICMP) {
			u8 _inner_type, *itp;

            // 获取原始icmp头中的type字段 
			itp = skb_header_pointer(skb_in,
						 skb_network_header(skb_in) +
						 (iph->ihl << 2) +
						 offsetof(struct icmphdr,
							  type) -
						 skb_in->data,
						 sizeof(_inner_type),
						 &_inner_type);
			if (!itp)
				goto out;

			/*
			 *	Assume any unknown ICMP type is an error. This
			 *	isn't specified by the RFC, but think about it..
             *	如果原始icmp报文包含不支持的icmp类型，或者是icmp错误消息，都将终止本次icmp错误消息的发送
			 */
			if (*itp > NR_ICMP_TYPES ||
			    icmp_pointers[*itp].error)
				goto out;
		}
	}

	icmp_param = kmalloc(sizeof(*icmp_param), GFP_ATOMIC);
	if (!icmp_param)
		return;

    // 获取出口网络设备所属net命名空间中当前cpu对应的icmp内核套接字
	sk = icmp_xmit_lock(net);
	if (!sk)
		goto out_free;

	/*
	 *	Construct source address and options.
	 */

	saddr = iph->daddr;
	if (!(rt->rt_flags & RTCF_LOCAL)) {
		struct net_device *dev = NULL;

		rcu_read_lock();
		if (rt_is_input_route(rt) &&
		    net->ipv4.sysctl_icmp_errors_use_inbound_ifaddr)
			dev = dev_get_by_index_rcu(net, inet_iif(skb_in));

		if (dev)
			saddr = inet_select_addr(dev, 0, RT_SCOPE_LINK);
		else
			saddr = 0;
		rcu_read_unlock();
	}

	tos = icmp_pointers[type].error ? ((iph->tos & IPTOS_TOS_MASK) |
					   IPTOS_PREC_INTERNETCONTROL) :
					  iph->tos;
	mark = IP4_REPLY_MARK(net, skb_in->mark);

    // 从原始ipv4报文中收集ip选项字段的信息
	if (ip_options_echo(&icmp_param->replyopts.opt.opt, skb_in))
		goto out_unlock;


	/*
	 *	Prepare data for ICMP header.
	 */

	icmp_param->data.icmph.type	 = type;
	icmp_param->data.icmph.code	 = code;
	icmp_param->data.icmph.un.gateway = info;
	icmp_param->data.icmph.checksum	 = 0;
	icmp_param->skb	  = skb_in;
	icmp_param->offset = skb_network_offset(skb_in);
	inet_sk(sk)->tos = tos;
	sk->sk_mark = mark;
	ipc.addr = iph->saddr;
	ipc.opt = &icmp_param->replyopts.opt;
	ipc.tx_flags = 0;
	ipc.ttl = 0;
	ipc.tos = -1;

	rt = icmp_route_lookup(net, &fl4, skb_in, iph, saddr, tos, mark,
			       type, code, icmp_param);
	if (IS_ERR(rt))
		goto out_unlock;

	if (!icmpv4_xrlim_allow(net, rt, &fl4, type, code))
		goto ende;

	/* RFC says return as much as we can without exceeding 576 bytes. */

	room = dst_mtu(&rt->dst);
	if (room > 576)
		room = 576;
	room -= sizeof(struct iphdr) + icmp_param->replyopts.opt.opt.optlen;
	room -= sizeof(struct icmphdr);

	icmp_param->data_len = skb_in->len - icmp_param->offset;
	if (icmp_param->data_len > room)
		icmp_param->data_len = room;
	icmp_param->head_len = sizeof(struct icmphdr);

	icmp_push_reply(icmp_param, &fl4, &ipc, &rt);
ende:
	ip_rt_put(rt);
out_unlock:
	icmp_xmit_unlock(sk);
out_free:
	kfree(icmp_param);
out:;
}
EXPORT_SYMBOL(icmp_send);

// 将icmp报文传递给ip层原始套接字
static void icmp_socket_deliver(struct sk_buff *skb, u32 info)
{
	const struct iphdr *iph = (const struct iphdr *) skb->data;
	const struct net_protocol *ipprot;
	int protocol = iph->protocol;

	/* Checkin full IP header plus 8 bytes of protocol to
	 * avoid additional coding at protocol handlers.
     * 确保ip头和icmp头都位于skb->data指针控制的线性缓冲区
	 */
	if (!pskb_may_pull(skb, iph->ihl * 4 + 8)) {
		ICMP_INC_STATS_BH(dev_net(skb->dev), ICMP_MIB_INERRORS);
		return;
	}

	raw_icmp_error(skb, protocol, info);

    // 调用icmp协议收到错误消息时的处理函数，也就是icmp_err
	ipprot = rcu_dereference(inet_protos[protocol]);
	if (ipprot && ipprot->err_handler)
		ipprot->err_handler(skb, info);
}

static bool icmp_tag_validation(int proto)
{
	bool ok;

	rcu_read_lock();
	ok = rcu_dereference(inet_protos[proto])->icmp_strict_tag_validation;
	rcu_read_unlock();
	return ok;
}

/*
 *	Handle ICMP_DEST_UNREACH, ICMP_TIME_EXCEED, ICMP_QUENCH, and
 *	ICMP_PARAMETERPROB.
 */

static bool icmp_unreach(struct sk_buff *skb)
{
	const struct iphdr *iph;
	struct icmphdr *icmph;
	struct net *net;
	u32 info = 0;

	net = dev_net(skb_dst(skb)->dev);

	/*
	 *	Incomplete header ?
	 * 	Only checks for the IP header, there should be an
	 *	additional check for longer headers in upper levels.
	 */

	if (!pskb_may_pull(skb, sizeof(struct iphdr)))
		goto out_err;

	icmph = icmp_hdr(skb);
	iph   = (const struct iphdr *)skb->data;

	if (iph->ihl < 5) /* Mangled header, drop. */
		goto out_err;

	if (icmph->type == ICMP_DEST_UNREACH) {
		switch (icmph->code & 15) {
		case ICMP_NET_UNREACH:
		case ICMP_HOST_UNREACH:
		case ICMP_PROT_UNREACH:
		case ICMP_PORT_UNREACH:
			break;
		case ICMP_FRAG_NEEDED:
			/* for documentation of the ip_no_pmtu_disc
			 * values please see
			 * Documentation/networking/ip-sysctl.txt
			 */
			switch (net->ipv4.sysctl_ip_no_pmtu_disc) {
			default:
				net_dbg_ratelimited("%pI4: fragmentation needed and DF set\n",
						    &iph->daddr);
				break;
			case 2:
				goto out;
			case 3:
				if (!icmp_tag_validation(iph->protocol))
					goto out;
				/* fall through */
			case 0:
				info = ntohs(icmph->un.frag.mtu);
			}
			break;
		case ICMP_SR_FAILED:
			net_dbg_ratelimited("%pI4: Source Route Failed\n",
					    &iph->daddr);
			break;
		default:
			break;
		}
		if (icmph->code > NR_ICMP_UNREACH)
			goto out;
	} else if (icmph->type == ICMP_PARAMETERPROB)
		info = ntohl(icmph->un.gateway) >> 24;

	/*
	 *	Throw it at our lower layers
	 *
	 *	RFC 1122: 3.2.2 MUST extract the protocol ID from the passed
	 *		  header.
	 *	RFC 1122: 3.2.2.1 MUST pass ICMP unreach messages to the
	 *		  transport layer.
	 *	RFC 1122: 3.2.2.2 MUST pass ICMP time expired messages to
	 *		  transport layer.
	 */

	/*
	 *	Check the other end isn't violating RFC 1122. Some routers send
	 *	bogus responses to broadcast frames. If you see this message
	 *	first check your netmask matches at both ends, if it does then
	 *	get the other vendor to fix their kit.
	 */

	if (!net->ipv4.sysctl_icmp_ignore_bogus_error_responses &&
	    inet_addr_type_dev_table(net, skb->dev, iph->daddr) == RTN_BROADCAST) {
		net_warn_ratelimited("%pI4 sent an invalid ICMP type %u, code %u error to a broadcast: %pI4 on %s\n",
				     &ip_hdr(skb)->saddr,
				     icmph->type, icmph->code,
				     &iph->daddr, skb->dev->name);
		goto out;
	}

	icmp_socket_deliver(skb, info);

out:
	return true;
out_err:
	ICMP_INC_STATS_BH(net, ICMP_MIB_INERRORS);
	return false;
}


/*
 *	Handle ICMP_REDIRECT.
 *	收到ICMP_REDIRECT类型icmpv4/6消息时的处理方法(软中断上下文)
 */
static bool icmp_redirect(struct sk_buff *skb)
{
    // 长度检查
	if (skb->len < sizeof(struct iphdr)) {
		ICMP_INC_STATS_BH(dev_net(skb->dev), ICMP_MIB_INERRORS);
		return false;
	}

    // 确保ip头位于skb->data指针控制的线性缓冲区
	if (!pskb_may_pull(skb, sizeof(struct iphdr))) {
		/* there aught to be a stat */
		return false;
	}

	icmp_socket_deliver(skb, icmp_hdr(skb)->un.gateway);
	return true;
}

/*
 *	Handle ICMP_ECHO ("ping") requests.
 *  收到ICMP_ECHO类型icmpv4消息时的处理方法
 *	RFC 1122: 3.2.2.6 MUST have an echo server that answers ICMP echo
 *		  requests.
 *	RFC 1122: 3.2.2.6 Data received in the ICMP_ECHO request MUST be
 *		  included in the reply.
 *	RFC 1812: 4.3.3.6 SHOULD have a config option for silently ignoring
 *		  echo requests, MUST have default=NOT.
 *	See also WRT handling of options once they are done and working.
 */

static bool icmp_echo(struct sk_buff *skb)
{
	struct net *net;

	net = dev_net(skb_dst(skb)->dev);
    // 如果该net命名空间开启了ping应答功能，则对收到的ping请求进行回复
	if (!net->ipv4.sysctl_icmp_echo_ignore_all) {
		struct icmp_bxm icmp_param;

        // 收集用于构造ping回复报文的参数
		icmp_param.data.icmph	   = *icmp_hdr(skb);
		icmp_param.data.icmph.type = ICMP_ECHOREPLY;
		icmp_param.skb		   = skb;
		icmp_param.offset	   = 0;
		icmp_param.data_len	   = skb->len;
		icmp_param.head_len	   = sizeof(struct icmphdr);
        // 回复ping请求
		icmp_reply(&icmp_param, skb);
	}
	/* should there be an ICMP stat for ignored echos? */
	return true;
}

/*
 *	Handle ICMP Timestamp requests.
 *  收到ICMP_TIMESTAMP类型icmpv4消息时的处理方法
 *	RFC 1122: 3.2.2.8 MAY implement ICMP timestamp requests.
 *		  SHOULD be in the kernel for minimum random latency.
 *		  MUST be accurate to a few minutes.
 *		  MUST be updated at least at 15Hz.
 */
static bool icmp_timestamp(struct sk_buff *skb)
{
	struct timespec tv;
	struct icmp_bxm icmp_param;
	/*
	 *	Too short.
	 */
	if (skb->len < 4)
		goto out_err;

    // 收集用于构造icmp时间戳回复报文的参数
	/*
	 *	Fill in the current time as ms since midnight UT:
	 */
	getnstimeofday(&tv);
	icmp_param.data.times[1] = htonl((tv.tv_sec % 86400) * MSEC_PER_SEC +
					 tv.tv_nsec / NSEC_PER_MSEC);
	icmp_param.data.times[2] = icmp_param.data.times[1];
	if (skb_copy_bits(skb, 0, &icmp_param.data.times[0], 4))
		BUG();
	icmp_param.data.icmph	   = *icmp_hdr(skb);
	icmp_param.data.icmph.type = ICMP_TIMESTAMPREPLY;
	icmp_param.data.icmph.code = 0;
	icmp_param.skb		   = skb;
	icmp_param.offset	   = 0;
	icmp_param.data_len	   = 0;
	icmp_param.head_len	   = sizeof(struct icmphdr) + 12;
    // 回复icmp时间戳请求
	icmp_reply(&icmp_param, skb);
	return true;

out_err:
	ICMP_INC_STATS_BH(dev_net(skb_dst(skb)->dev), ICMP_MIB_INERRORS);
	return false;
}

// 收到不支持/不需要做任何处理的icmpv4消息类型时的处理方法，实质就是一个空处理程序
static bool icmp_discard(struct sk_buff *skb)
{
	/* pretend it was a success */
	return true;
}

/*
 *	Deal with incoming ICMP packets.
 *	ICMPv4报文网络层->传输层的入口函数(软中断上下文)，被注册在icmp_protocol中
 */
int icmp_rcv(struct sk_buff *skb)
{
	struct icmphdr *icmph;
	struct rtable *rt = skb_rtable(skb);
	struct net *net = dev_net(rt->dst.dev);
	bool success;

	if (!xfrm4_policy_check(NULL, XFRM_POLICY_IN, skb)) {
		struct sec_path *sp = skb_sec_path(skb);
		int nh;

		if (!(sp && sp->xvec[sp->len - 1]->props.flags &
				 XFRM_STATE_ICMP))
			goto drop;

		if (!pskb_may_pull(skb, sizeof(*icmph) + sizeof(struct iphdr)))
			goto drop;

		nh = skb_network_offset(skb);
		skb_set_network_header(skb, sizeof(*icmph));

		if (!xfrm4_policy_check_reverse(NULL, XFRM_POLICY_IN, skb))
			goto drop;

		skb_set_network_header(skb, nh);
	}

    // 将snmp计数器ICMP_MIB_INMSGS加1
	ICMP_INC_STATS_BH(net, ICMP_MIB_INMSGS);

    // 简单检查校验和是否正确
	if (skb_checksum_simple_validate(skb))
		goto csum_error;

    // 首先确保skb->data指针控制的数据长度不小于len，然后data指针后移len长度
	if (!pskb_pull(skb, sizeof(*icmph)))
		goto error;

    // 获取icmp头
	icmph = icmp_hdr(skb);

	ICMPMSGIN_INC_STATS_BH(net, icmph->type);
	/*
	 *	18 is the highest 'known' ICMP type. Anything else is a mystery
	 *
	 *	RFC 1122: 3.2.2  Unknown ICMP messages types MUST be silently
	 *		  discarded.
     *	确保icmp报文类型合法
	 */
	if (icmph->type > NR_ICMP_TYPES)
		goto error;


	/*
	 *	Parse the ICMP message
	 */

    // 处理该icmp报文为IP广播包或IP组播包的情况
	if (rt->rt_flags & (RTCF_BROADCAST | RTCF_MULTICAST)) {
		/*
		 *	RFC 1122: 3.2.2.6 An ICMP_ECHO to broadcast MAY be
		 *	  silently ignored (we let user decide with a sysctl).
		 *	RFC 1122: 3.2.2.8 An ICMP_TIMESTAMP MAY be silently
		 *	  discarded if to broadcast/multicast.
         *	  如果系统开启了icmp广播包丢弃功能，则丢弃广播/组播地址的ping请求或者时间戳请求报文
		 */
		if ((icmph->type == ICMP_ECHO ||
		     icmph->type == ICMP_TIMESTAMP) &&
		    net->ipv4.sysctl_icmp_echo_ignore_broadcasts) {
			goto error;
		}
        // 如果该广播/组播地址的icmp报文不属于ping请求、时间戳请求、地址请求/回复类报文，则丢弃
		if (icmph->type != ICMP_ECHO &&
		    icmph->type != ICMP_TIMESTAMP &&
		    icmph->type != ICMP_ADDRESS &&
		    icmph->type != ICMP_ADDRESSREPLY) {
			goto error;
		}
	}

    // 从全局的icmp_pointers数组中查找该icmp报文所属类型的处理单元，然后执行其接收回调
	success = icmp_pointers[icmph->type].handler(skb);

    // 处理完毕后释放该skb
	if (success)  {
		consume_skb(skb);
		return 0;
	}

drop:
	kfree_skb(skb);
	return 0;
csum_error:
	ICMP_INC_STATS_BH(net, ICMP_MIB_CSUMERRORS);
error:
	ICMP_INC_STATS_BH(net, ICMP_MIB_INERRORS);
	goto drop;
}

// icmpv4协议收到错误消息时的处理函数
void icmp_err(struct sk_buff *skb, u32 info)
{
	struct iphdr *iph = (struct iphdr *)skb->data;
	int offset = iph->ihl<<2;
	struct icmphdr *icmph = (struct icmphdr *)(skb->data + offset);
	int type = icmp_hdr(skb)->type;
	int code = icmp_hdr(skb)->code;
	struct net *net = dev_net(skb->dev);

	/*
	 * Use ping_err to handle all icmp errors except those
	 * triggered by ICMP_ECHOREPLY which sent from kernel.
     * 所有icmp错误消息都使用ping_err方法处理
	 */
	if (icmph->type != ICMP_ECHOREPLY) {
		ping_err(skb, offset, info);
		return;
	}

    // 程序运行到这里必然是ICMP_ECHOREPLY类型，所以下面代码似乎都不会执行?
	if (type == ICMP_DEST_UNREACH && code == ICMP_FRAG_NEEDED)
		ipv4_update_pmtu(skb, net, info, 0, 0, IPPROTO_ICMP, 0);
	else if (type == ICMP_REDIRECT)
		ipv4_redirect(skb, net, 0, 0, IPPROTO_ICMP, 0);
}

/*
 *	This table is the definition of how we handle ICMP.
 *	这张表定义了icmpv4每个消息类型对应的处理单元
 */
static const struct icmp_control icmp_pointers[NR_ICMP_TYPES + 1] = {
	[ICMP_ECHOREPLY] = {
		.handler = ping_rcv,
	},
	[1] = {
		.handler = icmp_discard,
		.error = 1,
	},
	[2] = {
		.handler = icmp_discard,
		.error = 1,
	},
	[ICMP_DEST_UNREACH] = {
		.handler = icmp_unreach,
		.error = 1,
	},
	[ICMP_SOURCE_QUENCH] = {
		.handler = icmp_unreach,
		.error = 1,
	},
	[ICMP_REDIRECT] = {
		.handler = icmp_redirect,
		.error = 1,
	},
	[6] = {
		.handler = icmp_discard,
		.error = 1,
	},
	[7] = {
		.handler = icmp_discard,
		.error = 1,
	},
	[ICMP_ECHO] = {
		.handler = icmp_echo,
	},
	[9] = {
		.handler = icmp_discard,
		.error = 1,
	},
	[10] = {
		.handler = icmp_discard,
		.error = 1,
	},
	[ICMP_TIME_EXCEEDED] = {
		.handler = icmp_unreach,
		.error = 1,
	},
	[ICMP_PARAMETERPROB] = {
		.handler = icmp_unreach,
		.error = 1,
	},
	[ICMP_TIMESTAMP] = {
		.handler = icmp_timestamp,
	},
	[ICMP_TIMESTAMPREPLY] = {
		.handler = icmp_discard,
	},
	[ICMP_INFO_REQUEST] = {
		.handler = icmp_discard,
	},
	[ICMP_INFO_REPLY] = {
		.handler = icmp_discard,
	},
	[ICMP_ADDRESS] = {
		.handler = icmp_discard,
	},
	[ICMP_ADDRESSREPLY] = {
		.handler = icmp_discard,
	},
};

static void __net_exit icmp_sk_exit(struct net *net)
{
	int i;

	for_each_possible_cpu(i)
		inet_ctl_sock_destroy(*per_cpu_ptr(net->ipv4.icmp_sk, i));
	free_percpu(net->ipv4.icmp_sk);
	net->ipv4.icmp_sk = NULL;
}

// 具体的icmpv4功能初始化，实际主要就是创建icmpv4内核套接字以及icmpv4在所属网络命名空间层面的初始化
static int __net_init icmp_sk_init(struct net *net)
{
	int i, err;

    // 为每个cpu分配一个用于icmpv4的struct sock结构
	net->ipv4.icmp_sk = alloc_percpu(struct sock *);
	if (!net->ipv4.icmp_sk)
		return -ENOMEM;

    // 遍历每个cpu
	for_each_possible_cpu(i) {
		struct sock *sk;

        // 为每个cpu创建一个icmpv4内核套接字
		err = inet_ctl_sock_create(&sk, PF_INET,
					   SOCK_RAW, IPPROTO_ICMP, net);
		if (err < 0)
			goto fail;

		*per_cpu_ptr(net->ipv4.icmp_sk, i) = sk;

		/* Enough space for 2 64K ICMP packets, including
		 * sk_buff/skb_shared_info struct overhead.
         * 确保发送空间的长度能存放2*64K的icmpv4报文
		 */
		sk->sk_sndbuf =	2 * SKB_TRUESIZE(64 * 1024);

		/*
		 * Speedup sock_wfree()
		 */
		sock_set_flag(sk, SOCK_USE_WRITE_QUEUE);
        // 设置icmpv4内核套接字不进行pmtu发现
		inet_sk(sk)->pmtudisc = IP_PMTUDISC_DONT;
	}

    // 以下基本就是设置icmpv4在该net命名空间层面的参数
	/* Control parameters for ECHO replies. */
	net->ipv4.sysctl_icmp_echo_ignore_all = 0;
	net->ipv4.sysctl_icmp_echo_ignore_broadcasts = 1;

	/* Control parameter - ignore bogus broadcast responses? */
	net->ipv4.sysctl_icmp_ignore_bogus_error_responses = 1;

	/*
	 * 	Configurable global rate limit.
	 *
	 *	ratelimit defines tokens/packet consumed for dst->rate_token
	 *	bucket ratemask defines which icmp types are ratelimited by
	 *	setting	it's bit position.
	 *
	 *	default:
	 *	dest unreachable (3), source quench (4),
	 *	time exceeded (11), parameter problem (12)
	 */

	net->ipv4.sysctl_icmp_ratelimit = 1 * HZ;
	net->ipv4.sysctl_icmp_ratemask = 0x1818;
	net->ipv4.sysctl_icmp_errors_use_inbound_ifaddr = 0;

	return 0;

fail:
	for_each_possible_cpu(i)
		inet_ctl_sock_destroy(*per_cpu_ptr(net->ipv4.icmp_sk, i));
	free_percpu(net->ipv4.icmp_sk);
	return err;
}

// 定义了icmpv4模块在网络命名空间层面的操作集合
static struct pernet_operations __net_initdata icmp_sk_ops = {
       .init = icmp_sk_init,
       .exit = icmp_sk_exit,
};

// icmpv4模块的初始化入口
int __init icmp_init(void)
{
    /* 将icmp模块注册到每一个网络命名空间，并且执行了icmp_sk_init
     * 需要注意的是，由于icmp_sk_ops.size = 0，意味着icmp模块没有私有空间
     */
	return register_pernet_subsys(&icmp_sk_ops);
}
