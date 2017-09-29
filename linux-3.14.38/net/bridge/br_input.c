/*
 *	Handle incoming frames
 *	Linux ethernet bridge
 *	linux网桥处理收到的数据包
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/netfilter_bridge.h>
#include <linux/export.h>
#include <linux/rculist.h>
#include "br_private.h"

/* Hook for brouter */
br_should_route_hook_t __rcu *br_should_route_hook __read_mostly;
EXPORT_SYMBOL(br_should_route_hook);

/* 将指定报文由桥端口传递给网桥，实际就是将该skb关联到网桥设备
 *
 * 备注：该报文可能是发往本机的单播、组播、广播，甚至是开启混杂模式后的所有收到的报文
 */
static int br_pass_frame_up(struct sk_buff *skb)
{
	struct net_device *indev, *brdev = BR_INPUT_SKB_CB(skb)->brdev;
	struct net_bridge *br = netdev_priv(brdev);
	struct pcpu_sw_netstats *brstats = this_cpu_ptr(br->stats);
	struct net_port_vlans *pv;

	u64_stats_update_begin(&brstats->syncp);
	brstats->rx_packets++;
	brstats->rx_bytes += skb->len;
	u64_stats_update_end(&brstats->syncp);

	/* Bridge is just like any other port.  Make sure the
	 * packet is allowed except in promisc modue when someone
	 * may be running packet capture.
     * 网桥作为一个特殊的桥端口，在接收来自其他桥端口的skb前，需要确认是否允许接收
	 */
	pv = br_get_vlan_info(br);
	if (!(brdev->flags & IFF_PROMISC) &&
	    !br_allowed_egress(br, pv, skb)) {
		kfree_skb(skb);
		return NET_RX_DROP;
	}

	indev = skb->dev;
	skb->dev = brdev;       // 在这里，skb关联的设备由桥端口设备变成了网桥设备!

    // 再次对报文进行检测，在内核配置了CONFIG_BRIDGE_VLAN_FILTERING时会进行vlan检测
	skb = br_handle_vlan(br, pv, skb);
	if (!skb)
		return NET_RX_DROP;

    /* 在进入网桥设备的接收流程之前，本函数这里对该skb进行了netfilter过滤(前提是开启了netfilter过滤功能，否则就是直接放行)
     * 被netfilter拦截掉的skb已经被释放;
     * 被netfilter放行的skb会再次调用netif_receive_skb，只是这次skb关联的设备发生了变化
     */
	return NF_HOOK(NFPROTO_BRIDGE, NF_BR_LOCAL_IN, skb, indev, NULL,
		       netif_receive_skb);
}

/* note: already called with rcu_read_lock 
 *
 * */
int br_handle_frame_finish(struct sk_buff *skb)
{
	const unsigned char *dest = eth_hdr(skb)->h_dest;       // 获取报文的目的地址
	struct net_bridge_port *p = br_port_get_rcu(skb->dev);  // 从该skb关联的设备上进而获取关联的桥端口结构
	struct net_bridge *br;
	struct net_bridge_fdb_entry *dst;
	struct net_bridge_mdb_entry *mdst;
	struct sk_buff *skb2;
	bool unicast = true;
	u16 vid = 0;

    // 如果该桥端口不存在，或者该桥端口处于disable状态，则直接丢弃
	if (!p || p->state == BR_STATE_DISABLED)
		goto drop;

    // 内核配置了CONFIG_BRIDGE_VLAN_FILTERING时，这里需要对该skb进行入口vlan检测和处理，没有通过检测的直接丢弃
	if (!br_allowed_ingress(p->br, nbp_get_vlan_info(p), skb, &vid))
		goto out;

	/* insert into forwarding database after filtering to avoid spoofing */
	br = p->br;
    // 如果该桥端口具备学习能力, 则从中学习源mac地址到转发表
	if (p->flags & BR_LEARNING)
		br_fdb_update(br, p, eth_hdr(skb)->h_source, vid, false);

    /* 如果是一个以太网多播帧，并且内核开启了CONFIG_BRIDGE_IGMP_SNOOPING配置，则进入多播接收流程
     * 如果没有开启CONFIG_BRIDGE_IGMP_SNOOPING，或者多播接收成功，则继续执行下面的流程
     * 如果多播接收失败，则直接丢弃
     */
	if (!is_broadcast_ether_addr(dest) && is_multicast_ether_addr(dest) &&
	    br_multicast_rcv(br, p, skb, vid))
		goto drop;

    // 如果该桥端口处于learning状态(显然，在这之前已经尝试学习了源mac)，则丢弃该skb
	if (p->state == BR_STATE_LEARNING)
		goto drop;

    // 记录下该skb所属的网桥设备
	BR_INPUT_SKB_CB(skb)->brdev = br->dev;

	/* The packet skb2 goes to the local host (NULL to skip). 
     * skb2用于指向发送到本地的数据包元，skb用于指向要往其他桥端口转发的数据包元
     * */
	skb2 = NULL;

    // 当该网桥设备开启了混杂模式时，所有进入该网桥的skb都会被额外发往本地
	if (br->dev->flags & IFF_PROMISC)
		skb2 = skb;

	dst = NULL;

	if (is_broadcast_ether_addr(dest)) {
        // 显然广播帧肯定是会泛洪到本地
		skb2 = skb;
		unicast = false;
	} else if (is_multicast_ether_addr(dest)) {
        // 对于多播帧需要作进一步判断

        /* 当内核配置了CONFIG_BRIDGE_IGMP_SNOOPING选项时，如果还同时满足以下2个条件：
         *      该多播帧对应的组播组存在或者mrouters_only标志置位
         *      该多播帧对应的IGMP查询器存在
         * 则交由IGMP模块处理。
         * 其他情况下，多播帧都会被泛洪处理
         */
		mdst = br_mdb_get(br, skb, vid);
		if ((mdst || BR_INPUT_SKB_CB_MROUTERS_ONLY(skb)) &&
		    br_multicast_querier_exists(br, eth_hdr(skb))) {
			if ((mdst && mdst->mglist) ||
			    br_multicast_is_router(br))
				skb2 = skb;
			br_multicast_forward(mdst, skb, skb2);
			skb = NULL;
			if (!skb2)
				goto out;
		} else
			skb2 = skb;

		unicast = false;
		br->dev->stats.multicast++;
	} else if ((dst = __br_fdb_get(br, dest, vid)) &&
			dst->is_local) {
        // 显然这里是直接发往本地的单播
		skb2 = skb;
		/* Do not forward the packet since it's local.  发往本地的单播意味着无须再转发 */
		skb = NULL;
	}

    // skb有效意味着需要往其他桥端口转发
	if (skb) {
        /* 如果存在目的地址对应的转发表项(已知单播)，则转发到对应的桥端口
         * 否则泛洪到除了入口之外的所有桥端口(未知单播、组播、广播)
         */
		if (dst) {
			dst->used = jiffies;
			br_forward(dst->dst, skb, skb2);
		} else
			br_flood_forward(br, skb, skb2, unicast);
	}

    // skb2有效意味着需要发往本机协议栈
	if (skb2)
		return br_pass_frame_up(skb2);

out:
	return 0;
drop:
	kfree_skb(skb);
	goto out;
}

/* note: already called with rcu_read_lock 
 * 本函数只会在收到01:80:c2:00:00:x系列保留组播地址报文时可能被调用
 * 这里实际做了尝试更新转发表的操作
 * */
static int br_handle_local_finish(struct sk_buff *skb)
{
    // 从该skb关联的设备上进而获取关联的桥端口结构net_bridge_port
	struct net_bridge_port *p = br_port_get_rcu(skb->dev);
	u16 vid = 0;

	/* check if vlan is allowed, to avoid spoofing 
     * 如果该桥端口具备学习能力(内核配置了CONFIG_BRIDGE_VLAN_FILTERING时，还需要检查是否允许对该skb携带的报文进行学习)，
     * 则从中学习源mac地址到转发表
     * */
	if (p->flags & BR_LEARNING && br_should_learn(p, skb, &vid))
		br_fdb_update(p->br, p, eth_hdr(skb)->h_source, vid, false);
	return 0;	 /* process further */
}

/* 桥端口设备接收数据包处理入口，
 * Return NULL if skb is handled
 * note: already called with rcu_read_lock
 *
 * 备注：本函数作为钩子注册在桥端口设备的net_device->rx_handler，在__netif_receive_skb_core中被执行
 */
rx_handler_result_t br_handle_frame(struct sk_buff **pskb)
{
	struct net_bridge_port *p;
	struct sk_buff *skb = *pskb;
	const unsigned char *dest = eth_hdr(skb)->h_dest;
	br_should_route_hook_t *rhook;

    // 如果是个loopback包不做任何处理直接返回RX_HANDLER_PASS
	if (unlikely(skb->pkt_type == PACKET_LOOPBACK))
		return RX_HANDLER_PASS;

    // 如果报文中的源地址不是一个合法的以太网地址，则直接丢弃并返回RX_HANDLER_CONSUMED
	if (!is_valid_ether_addr(eth_hdr(skb)->h_source))
		goto drop;

    // 这一步确保了当前处理的skb没有被其他用户操作中
	skb = skb_share_check(skb, GFP_ATOMIC);
	if (!skb)
		return RX_HANDLER_CONSUMED;

    // 从该skb关联的设备上进而获取关联的桥端口结构net_bridge_port
	p = br_port_get_rcu(skb->dev);

    // 如果目的地址属于01:80:c2:00:00:0x系列，这些都是保留组播地址，通常承载了特定的协议，这里需要进行相关处理
	if (unlikely(is_link_local_ether_addr(dest))) {
		/*
		 * See IEEE 802.1D Table 7-10 Reserved addresses
		 *
		 * Assignment		 		Value
		 * Bridge Group Address		01-80-C2-00-00-00
		 * (MAC Control) 802.3		01-80-C2-00-00-01
		 * (Link Aggregation) 802.3	01-80-C2-00-00-02
		 * 802.1X PAE address		01-80-C2-00-00-03
		 *
		 * 802.1AB LLDP 		01-80-C2-00-00-0E
		 *
		 * Others reserved for future standardization
		 */
		switch (dest[5]) {
		case 0x00:	/* Bridge Group Address 末字节为0x00表示stp协议报文 */
			/* If STP is turned off,
			   then must forward to keep loop detection 
               当该桥端口所在网桥的stp功能处于关闭状态时，直接转发收到的stp协议报文
               */
			if (p->br->stp_enabled == BR_NO_STP)
				goto forward;
			break;

		case 0x01:	/* IEEE MAC (Pause) */
			goto drop;

		default:
			/* Allow selective forwarding for most other protocols 
             * 有选择性的对其他协议类型的报文进行直接转发
             * */
			if (p->br->group_fwd_mask & (1u << dest[5]))
				goto forward;
		}

        // 程序运行到这里意味着该桥端口无法对这个承载特定协议的skb简单的进行转发或丢弃
		/* Deliver packet to local host only 
         * 在交由后面的接收流程作进一步处理之前，本函数这里对该skb进行了netfilter过滤(前提是开启了netfilter过滤功能，否则就是直接放行)
         * 被netfilter拦截掉的skb已经被释放，所以直接返回RX_HANDLER_CONSUMED;
         * 被netfilter放行的skb最后会尝试更新转发表，然后直接返回RX_HANDLER_PASS，意味着系统将继续对该skb执行接收流程(上协议栈)
         * */
		if (NF_HOOK(NFPROTO_BRIDGE, NF_BR_LOCAL_IN, skb, skb->dev,
			    NULL, br_handle_local_finish)) {
			return RX_HANDLER_CONSUMED; /* consumed by filter */
		} else {
			*pskb = skb;
			return RX_HANDLER_PASS;	/* continue processing */
		}
	}

forward:        // 对该skb进行直接转发流程
	switch (p->state) {
	case BR_STATE_FORWARDING:       // 桥端口处于forwarding状态下的转发流程
        // 处于forwarding状态下的桥端口会额外执行一个ebtables钩子
		rhook = rcu_dereference(br_should_route_hook);
		if (rhook) {
			if ((*rhook)(skb)) {
				*pskb = skb;
				return RX_HANDLER_PASS;
			}
			dest = eth_hdr(skb)->h_dest;
		}
		/* fall through */
	case BR_STATE_LEARNING:         // 桥端口处于learnging状态下的转发流程
        // 比较该网桥设备地址和该skb中承载报文的目的地址，如果相等，意味着这是一个发往本机的报文
		if (ether_addr_equal(p->br->dev->dev_addr, dest))
			skb->pkt_type = PACKET_HOST;

        /* 桥端口不论处于forwarding还是learnging状态，其转发流程都会合流到这里
         * 这里对该skb进行了netfilter过滤(前提是开启了netfilter过滤功能，否则就是直接放行)
         * 被netfilter拦截掉的skb已经被释放，所以直接返回RX_HANDLER_CONSUMED;
         * 被netfilter放行的skb则会执行br_handle_frame_finish完成真正的转发动作(完成后会释放该skb)，最后返回RX_HANDLER_CONSUMED
         */
		NF_HOOK(NFPROTO_BRIDGE, NF_BR_PRE_ROUTING, skb, skb->dev, NULL,
			br_handle_frame_finish);
		break;
	default:
drop:           // 对该skb进行直接丢弃流程
		kfree_skb(skb);
	}
	return RX_HANDLER_CONSUMED;
}
