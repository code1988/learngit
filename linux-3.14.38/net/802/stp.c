/*
 *	STP SAP demux
 *
 *	Copyright (c) 2008 Patrick McHardy <kaber@trash.net>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	version 2 as published by the Free Software Foundation.
 */
#include <linux/mutex.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/llc.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <net/llc.h>
#include <net/llc_pdu.h>
#include <net/stp.h>

/* 01:80:c2:00:00:20 - 01:80:c2:00:00:2F */
#define GARP_ADDR_MIN	0x20
#define GARP_ADDR_MAX	0x2F
#define GARP_ADDR_RANGE	(GARP_ADDR_MAX - GARP_ADDR_MIN)

static const struct stp_proto __rcu *garp_protos[GARP_ADDR_RANGE + 1] __read_mostly;    // 这个生成树表记录了末字节为0x20~0x2f的生成树协议
static const struct stp_proto __rcu *stp_proto __read_mostly;                           // 指向一个末字节为0x00的生成树协议

static struct llc_sap *sap __read_mostly;
static unsigned int sap_registered;         // 用来记录当前已经注册的生成树协议数量
static DEFINE_MUTEX(stp_proto_mutex);       // 专门用于增删生成树协议时的互斥锁

/* Called under rcu_read_lock from LLC 
 * 所有生成树协议共用的LLC层BPDU接收函数
 * @skb     - 成在了BPDU的skb
 * @dev     - 收到该BPDU的设备
 * @pt      - 
 * */
static int stp_pdu_rcv(struct sk_buff *skb, struct net_device *dev,
		       struct packet_type *pt, struct net_device *orig_dev)
{
	const struct ethhdr *eh = eth_hdr(skb);                 // 获取BPDU中的以太网头字段
	const struct llc_pdu_un *pdu = llc_pdu_un_hdr(skb);     // 获取BPDU中的LLC字段
	const struct stp_proto *proto;

    // LLC字段合法性检测
	if (pdu->ssap != LLC_SAP_BSPAN ||
	    pdu->dsap != LLC_SAP_BSPAN ||
	    pdu->ctrl_1 != LLC_PDU_TYPE_U)
		goto err;

    // 根据末字节决定调用哪个生成树协议(缺省就是stp_proto指向的那个)
	if (eh->h_dest[5] >= GARP_ADDR_MIN && eh->h_dest[5] <= GARP_ADDR_MAX) {
		proto = rcu_dereference(garp_protos[eh->h_dest[5] -
						    GARP_ADDR_MIN]);
		if (proto &&
		    !ether_addr_equal(eh->h_dest, proto->group_address))
			goto err;
	} else
		proto = rcu_dereference(stp_proto);

	if (!proto)
		goto err;

    // 执行该生成树协议关联的接收钩子
	proto->rcv(proto, skb, dev);
	return 0;

err:
	kfree_skb(skb);
	return 0;
}

// 注册指定的生成树协议到LLC模块
int stp_proto_register(const struct stp_proto *proto)
{
	int err = 0;

	mutex_lock(&stp_proto_mutex);
    // 注册第一个生成树协议时，需要创建对应的LLC接口(同时注册了一个接收函数stp_pdu_rcv)，后续注册的生成树协议共用该LLC接口
	if (sap_registered++ == 0) {
		sap = llc_sap_open(LLC_SAP_BSPAN, stp_pdu_rcv);
		if (!sap) {
			err = -ENOMEM;
			goto out;
		}
	}
	if (is_zero_ether_addr(proto->group_address))
		rcu_assign_pointer(stp_proto, proto);
	else
		rcu_assign_pointer(garp_protos[proto->group_address[5] -
					       GARP_ADDR_MIN], proto);
out:
	mutex_unlock(&stp_proto_mutex);
	return err;
}
EXPORT_SYMBOL_GPL(stp_proto_register);

// 从LLC模块中注销指定的生成树协议
void stp_proto_unregister(const struct stp_proto *proto)
{
	mutex_lock(&stp_proto_mutex);
	if (is_zero_ether_addr(proto->group_address))
		RCU_INIT_POINTER(stp_proto, NULL);
	else
		RCU_INIT_POINTER(garp_protos[proto->group_address[5] -
					       GARP_ADDR_MIN], NULL);
	synchronize_rcu();

	if (--sap_registered == 0)
		llc_sap_put(sap);
	mutex_unlock(&stp_proto_mutex);
}
EXPORT_SYMBOL_GPL(stp_proto_unregister);

MODULE_LICENSE("GPL");
