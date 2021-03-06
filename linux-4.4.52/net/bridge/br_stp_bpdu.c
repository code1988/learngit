/*
 *	Spanning tree protocol; BPDU handling
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/netfilter_bridge.h>
#include <linux/etherdevice.h>
#include <linux/llc.h>
#include <linux/slab.h>
#include <linux/pkt_sched.h>
#include <net/net_namespace.h>
#include <net/llc.h>
#include <net/llc_pdu.h>
#include <net/stp.h>
#include <asm/unaligned.h>

#include "br_private.h"
#include "br_private_stp.h"

#define STP_HZ		256

#define LLC_RESERVE sizeof(struct llc_pdu_un)

static int br_send_bpdu_finish(struct net *net, struct sock *sk,
			       struct sk_buff *skb)
{
	return dev_queue_xmit(skb);
}

/* 发BPDU包
 * @p       指向要发包的桥端口
 * @data    指向要发送的BPDU数据
 * @length  要发送的BPDU数据长度，对于STP就是35
 */
static void br_send_bpdu(struct net_bridge_port *p,
			 const unsigned char *data, int length)
{
	struct sk_buff *skb;

    // 为BPDU包分配一个无主的skb
	skb = dev_alloc_skb(length+LLC_RESERVE);
	if (!skb)
		return;

    // 将新创建的skb跟桥端口设备关联
	skb->dev = p->dev;
	skb->protocol = htons(ETH_P_802_2);
	skb->priority = TC_PRIO_CONTROL;

    // 从skb的线性缓冲区中划出LLC_RESERVE长度headroom
	skb_reserve(skb, LLC_RESERVE);
    // 往skb中写入BPDU数据
	memcpy(__skb_put(skb, length), data, length);

    // 往skb中填充LLC头
	llc_pdu_header_init(skb, LLC_PDU_TYPE_U, LLC_SAP_BSPAN,
			    LLC_SAP_BSPAN, LLC_PDU_CMD);
	llc_pdu_init_as_ui_cmd(skb);

    // 往skb中填充MAC头
	llc_mac_hdr_init(skb, p->dev->dev_addr, p->br->group_addr);

	skb_reset_mac_header(skb);

    /* 最后在调用设备的发送数据入口函数前会执行netfilter的NF_BR_LOCAL_OUT钩子(如果存在)
     * 没有通过netfilter的报文就会被拦截而不会执行dev_queue_xmit
     */
	NF_HOOK(NFPROTO_BRIDGE, NF_BR_LOCAL_OUT,
		dev_net(p->dev), NULL, skb, NULL, skb->dev,
		br_send_bpdu_finish);
}

// 设置STP定时器时间
static inline void br_set_ticks(unsigned char *dest, int j)
{
	unsigned long ticks = (STP_HZ * j)/ HZ;

	put_unaligned_be16(ticks, dest);
}

// 获取STP定时器时间
static inline int br_get_ticks(const unsigned char *src)
{
	unsigned long ticks = get_unaligned_be16(src);

	return DIV_ROUND_UP(ticks * HZ, STP_HZ);
}

/* called under bridge lock 
 * 生成配置BPDU并发送
 * */
void br_send_config_bpdu(struct net_bridge_port *p, struct br_config_bpdu *bpdu)
{
	unsigned char buf[35];

    // 确保所在网桥使能了内核STP
	if (p->br->stp_enabled != BR_KERNEL_STP)
		return;

    // 组包
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = BPDU_TYPE_CONFIG;
	buf[4] = (bpdu->topology_change ? 0x01 : 0) |
		(bpdu->topology_change_ack ? 0x80 : 0);
	buf[5] = bpdu->root.prio[0];
	buf[6] = bpdu->root.prio[1];
	buf[7] = bpdu->root.addr[0];
	buf[8] = bpdu->root.addr[1];
	buf[9] = bpdu->root.addr[2];
	buf[10] = bpdu->root.addr[3];
	buf[11] = bpdu->root.addr[4];
	buf[12] = bpdu->root.addr[5];
	buf[13] = (bpdu->root_path_cost >> 24) & 0xFF;
	buf[14] = (bpdu->root_path_cost >> 16) & 0xFF;
	buf[15] = (bpdu->root_path_cost >> 8) & 0xFF;
	buf[16] = bpdu->root_path_cost & 0xFF;
	buf[17] = bpdu->bridge_id.prio[0];
	buf[18] = bpdu->bridge_id.prio[1];
	buf[19] = bpdu->bridge_id.addr[0];
	buf[20] = bpdu->bridge_id.addr[1];
	buf[21] = bpdu->bridge_id.addr[2];
	buf[22] = bpdu->bridge_id.addr[3];
	buf[23] = bpdu->bridge_id.addr[4];
	buf[24] = bpdu->bridge_id.addr[5];
	buf[25] = (bpdu->port_id >> 8) & 0xFF;
	buf[26] = bpdu->port_id & 0xFF;

	br_set_ticks(buf+27, bpdu->message_age);
	br_set_ticks(buf+29, bpdu->max_age);
	br_set_ticks(buf+31, bpdu->hello_time);
	br_set_ticks(buf+33, bpdu->forward_delay);

    // 发BPDU
	br_send_bpdu(p, buf, 35);
}

/* called under bridge lock */
void br_send_tcn_bpdu(struct net_bridge_port *p)
{
	unsigned char buf[4];

	if (p->br->stp_enabled != BR_KERNEL_STP)
		return;

	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = BPDU_TYPE_TCN;
	br_send_bpdu(p, buf, 4);
}

/*
 * Called from llc.
 * BPDU报文接收的入口函数，初始化生成树时被注册到LLC层
 * @proto   - 指向该BPDU所属的生成树协议
 * @skb     - 指向承载了该BPDU的数据包元
 * @dev     - 收到该BPDU的设备
 *
 * 备注：进入本函数时，skb->data指针已经指向BPDU字段首地址
 *
 * NO locks, but rcu_read_lock
 */
void br_stp_rcv(const struct stp_proto *proto, struct sk_buff *skb,
		struct net_device *dev)
{
	const unsigned char *dest = eth_hdr(skb)->h_dest;
	struct net_bridge_port *p;
	struct net_bridge *br;
	const unsigned char *buf;

    // 确保skb->data指针控制的数据长度不小于4
	if (!pskb_may_pull(skb, 4))
		goto err;

	/* compare of protocol id and version */
	buf = skb->data;
    // 内核默认只支持stp，stp的协议ID字段为0x0000,协议版本ID字段为0x00
	if (buf[0] != 0 || buf[1] != 0 || buf[2] != 0)
		goto err;

    // 返回该设备的桥端口结构
	p = br_port_get_check_rcu(dev);
	if (!p)
		goto err;

    // 进而获取桥端口所属的网桥结构
	br = p->br;
	spin_lock(&br->lock);

    // 确保该网桥使能了stp
	if (br->stp_enabled != BR_KERNEL_STP)
		goto out;

    // 确保该网桥设备处于UP状态
	if (!(br->dev->flags & IFF_UP))
		goto out;

    // 确保该桥端口没有处于disabled状态
	if (p->state == BR_STATE_DISABLED)
		goto out;

    // 确保该BPDU的目的地址跟网桥设置的stp地址匹配
	if (!ether_addr_equal(dest, br->group_addr))
		goto out;

	if (p->flags & BR_BPDU_GUARD) {
		br_notice(br, "BPDU received on blocked port %u(%s)\n",
			  (unsigned int) p->port_no, p->dev->name);
		br_stp_disable_port(p);
		goto out;
	}

    // data指针后移3个字节，指向BPDU-TYPE字段
	buf = skb_pull(skb, 3);

    // 对不同的BPDU TYPE做相应处理
	if (buf[0] == BPDU_TYPE_CONFIG) {
        // 以下是对普通配置BPDU报文进行处理
		struct br_config_bpdu bpdu;

        // 确保data指针控制的数据长度不小于32
		if (!pskb_may_pull(skb, 32))
			goto out;

		buf = skb->data;
		bpdu.topology_change = (buf[1] & 0x01) ? 1 : 0;
		bpdu.topology_change_ack = (buf[1] & 0x80) ? 1 : 0;

		bpdu.root.prio[0] = buf[2];
		bpdu.root.prio[1] = buf[3];
		bpdu.root.addr[0] = buf[4];
		bpdu.root.addr[1] = buf[5];
		bpdu.root.addr[2] = buf[6];
		bpdu.root.addr[3] = buf[7];
		bpdu.root.addr[4] = buf[8];
		bpdu.root.addr[5] = buf[9];
		bpdu.root_path_cost =
			(buf[10] << 24) |
			(buf[11] << 16) |
			(buf[12] << 8) |
			buf[13];
		bpdu.bridge_id.prio[0] = buf[14];
		bpdu.bridge_id.prio[1] = buf[15];
		bpdu.bridge_id.addr[0] = buf[16];
		bpdu.bridge_id.addr[1] = buf[17];
		bpdu.bridge_id.addr[2] = buf[18];
		bpdu.bridge_id.addr[3] = buf[19];
		bpdu.bridge_id.addr[4] = buf[20];
		bpdu.bridge_id.addr[5] = buf[21];
		bpdu.port_id = (buf[22] << 8) | buf[23];

		bpdu.message_age = br_get_ticks(buf+24);
		bpdu.max_age = br_get_ticks(buf+26);
		bpdu.hello_time = br_get_ticks(buf+28);
		bpdu.forward_delay = br_get_ticks(buf+30);

        // 如果该BPDU的消息年龄超过了最大年龄，则丢弃
		if (bpdu.message_age > bpdu.max_age) {
			if (net_ratelimit())
				br_notice(p->br,
					  "port %u config from %pM"
					  " (message_age %ul > max_age %ul)\n",
					  p->port_no,
					  eth_hdr(skb)->h_source,
					  bpdu.message_age, bpdu.max_age);
			goto out;
		}

        // 对合法的配置BPDU真正进行处理
		br_received_config_bpdu(p, &bpdu);
	} else if (buf[0] == BPDU_TYPE_TCN) {
        // 以下是对TCN-BPDU报文进行处理
		br_received_tcn_bpdu(p);
	}
 out:
	spin_unlock(&br->lock);
 err:
	kfree_skb(skb);
}
