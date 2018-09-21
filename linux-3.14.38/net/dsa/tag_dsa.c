/*
 * net/dsa/tag_dsa.c - (Non-ethertype) DSA tagging
 * Copyright (c) 2008-2009 Marvell Semiconductor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/etherdevice.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/slab.h>
#include "dsa_priv.h"

#define DSA_HLEN	4

/* DSA从设备驱动的发送回调函数
 * @skb 承载了要发送数据的skb，注意该skb的写指针位于以太网头
 * @dev 执行该发送操作的DSA从设备
 */
netdev_tx_t dsa_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct dsa_slave_priv *p = netdev_priv(dev);    // 获取该DSA从设备的私有空间，也就是switch物理口实例
	u8 *dsa_header;

	dev->stats.tx_packets++;
	dev->stats.tx_bytes += skb->len;

	/*
	 * Convert the outermost 802.1q tag to a DSA tag for tagged
	 * packets, or insert a DSA tag between the addresses and
	 * the ethertype field for untagged packets.
     * 将标准的以太网帧转换成DSA帧，根据是否携带vlan信息各自执行转换流程 
	 */
	if (skb->protocol == htons(ETH_P_8021Q)) {
        // 如果需要会首先拷贝一份skb
		if (skb_cow_head(skb, 0) < 0)
			goto out_free;

		/*
		 * Construct tagged FROM_CPU DSA tag from 802.1q tag.
         * 构造FROM_CPU类型的dsa头
		 */
		dsa_header = skb->data + 2 * ETH_ALEN;
		dsa_header[0] = 0x60 | p->parent->index;
		dsa_header[1] = p->port << 3;

		/*
		 * Move CFI field from byte 2 to byte 1.
		 */
		if (dsa_header[2] & 0x10) {
			dsa_header[1] |= 0x01;
			dsa_header[2] &= ~0x10;
		}
	} else {
        // 如果需要会首先拷贝一份skb
		if (skb_cow_head(skb, DSA_HLEN) < 0)
			goto out_free;
		skb_push(skb, DSA_HLEN);

		memmove(skb->data, skb->data + DSA_HLEN, 2 * ETH_ALEN);

		/*
		 * Construct untagged FROM_CPU DSA tag.
		 */
		dsa_header = skb->data + 2 * ETH_ALEN;
		dsa_header[0] = 0x40 | p->parent->index;
		dsa_header[1] = p->port << 3;
		dsa_header[2] = 0x00;
		dsa_header[3] = 0x00;
	}

    // 程序运行到这里该skb中已经是一个DSA帧
	skb->protocol = htons(ETH_P_DSA);   // 将该skb承载的帧类型改为ETH_P_DSA

	skb->dev = p->parent->dst->master_netdev;   // 将该skb关联的设备重定向为所属DSA实例的宿主netdev  
    // 再次执行设备层的完整发送流程
	dev_queue_xmit(skb);

	return NETDEV_TX_OK;

out_free:
	kfree_skb(skb);
	return NETDEV_TX_OK;
}

/* ETH_P_DSA协议被注册到内核中后，从关联netdev(通常是eth0)收到该协议报文后的入口
 * @skb     承载了接收到的DSA帧的skb，注意该skb的读指针已经跳过了标准以太网头长度
 * @dev     该skb当前关联的netdev
 * @pt      该入口所属的DSA协议管理块
 * @orig_dev 最初接收到该skb的netdev
 */
static int dsa_rcv(struct sk_buff *skb, struct net_device *dev,
		   struct packet_type *pt, struct net_device *orig_dev)
{
	struct dsa_switch_tree *dst = dev->dsa_ptr; // 获取该宿主netdev上寄宿的DSA实例
	struct dsa_switch *ds;
	u8 *dsa_header;
	int source_device;
	int source_port;

	if (unlikely(dst == NULL))
		goto out_drop;

    // 拷贝skb
	skb = skb_unshare(skb, GFP_ATOMIC);
	if (skb == NULL)
		goto out;

	if (unlikely(!pskb_may_pull(skb, DSA_HLEN)))
		goto out_drop;

	/*
	 * The ethertype field is part of the DSA header.
     * 计算dsa头的位置
	 */
	dsa_header = skb->data - 2;

	/*
	 * Check that frame type is either TO_CPU or FORWARD.
     * 只接收处理FORWARD类型的dsa-tag
	 */
	if ((dsa_header[0] & 0xc0) != 0x00 && (dsa_header[0] & 0xc0) != 0xc0)
		goto out_drop;

	/*
	 * Determine source device and port.
     * 计算该DSA帧的源switch和源物理口
	 */
	source_device = dsa_header[0] & 0x1f;
	source_port = (dsa_header[1] >> 3) & 0x1f;

	/*
	 * Check that the source device exists and that the source
	 * port is a registered DSA port.
     * 检查源switch和源物理口是否合法
	 */
	if (source_device >= dst->pd->nr_chips)
		goto out_drop;
	ds = dst->ds[source_device];
	if (source_port >= DSA_MAX_PORTS || ds->ports[source_port] == NULL)
		goto out_drop;

	/*
	 * Convert the DSA header to an 802.1q header if the 'tagged'
	 * bit in the DSA header is set.  If the 'tagged' bit is clear,
	 * delete the DSA header entirely.
     * 将DSA帧转换成标准的以太网帧，根据是否携带vlan信息各自执行转换流程
	 */
	if (dsa_header[0] & 0x20) {
		u8 new_header[4];

		/*
		 * Insert 802.1q ethertype and copy the VLAN-related
		 * fields, but clear the bit that will hold CFI (since
		 * DSA uses that bit location for another purpose).
         * 构造vlan-tag字段
		 */
		new_header[0] = (ETH_P_8021Q >> 8) & 0xff;
		new_header[1] = ETH_P_8021Q & 0xff;
		new_header[2] = dsa_header[2] & ~0x10;
		new_header[3] = dsa_header[3];

		/*
		 * Move CFI bit from its place in the DSA header to
		 * its 802.1q-designated place.
		 */
		if (dsa_header[1] & 0x01)
			new_header[2] |= 0x10;

		/*
		 * Update packet checksum if skb is CHECKSUM_COMPLETE.
         * 更新skb的校验和
		 */
		if (skb->ip_summed == CHECKSUM_COMPLETE) {
			__wsum c = skb->csum;
			c = csum_add(c, csum_partial(new_header + 2, 2, 0));
			c = csum_sub(c, csum_partial(dsa_header + 2, 2, 0));
			skb->csum = c;
		}

		memcpy(dsa_header, new_header, DSA_HLEN);
	} else {
		/*
		 * Remove DSA tag and update checksum.
         * 读指针掠过DSA字段，同时更新skb的校验和
		 */
		skb_pull_rcsum(skb, DSA_HLEN);
        // 将不带vlan信息的DSA帧转换成标准以太网帧
		memmove(skb->data - ETH_HLEN,
			skb->data - ETH_HLEN - DSA_HLEN,
			2 * ETH_ALEN);
	}

    // 程序运行到这里该skb中已经是一个标准的以太网帧
	skb->dev = ds->ports[source_port];              // 将该skb关联的设备重定向为源端口对应的netdev
	skb_push(skb, ETH_HLEN);                        // 将该skb的读指针重新指向以太网头
	skb->pkt_type = PACKET_HOST;                    // 将该skb中的包类型初始化为PACKET_HOST
	skb->protocol = eth_type_trans(skb, skb->dev);  // 重新计算该skb中的帧的协议类型

	skb->dev->stats.rx_packets++;
	skb->dev->stats.rx_bytes += skb->len;

    // 再次执行设备层的完整接收流程
	netif_receive_skb(skb);

	return 0;

out_drop:
	kfree_skb(skb);
out:
	return 0;
}

// 定义了ETH_P_DSA协议的收包方法，在通用DSA驱动初始化函数中会被注册到内核中
struct packet_type dsa_packet_type __read_mostly = {
	.type	= cpu_to_be16(ETH_P_DSA),
	.func	= dsa_rcv,
};
