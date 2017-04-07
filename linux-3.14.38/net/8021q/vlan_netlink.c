/*
 *	VLAN netlink control interface
 *	基于netlink接口的vlan操作集合
 *
 * 	Copyright (c) 2007 Patrick McHardy <kaber@trash.net>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	version 2 as published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/if_vlan.h>
#include <linux/module.h>
#include <net/net_namespace.h>
#include <net/netlink.h>
#include <net/rtnetlink.h>
#include "vlan.h"

// 定义了一个网络接口链路中有关vlan部分的策略组
static const struct nla_policy vlan_policy[IFLA_VLAN_MAX + 1] = {
	[IFLA_VLAN_ID]		= { .type = NLA_U16 },
	[IFLA_VLAN_FLAGS]	= { .len = sizeof(struct ifla_vlan_flags) },
	[IFLA_VLAN_EGRESS_QOS]	= { .type = NLA_NESTED },
	[IFLA_VLAN_INGRESS_QOS] = { .type = NLA_NESTED },
	[IFLA_VLAN_PROTOCOL]	= { .type = NLA_U16 },
};

static const struct nla_policy vlan_map_policy[IFLA_VLAN_QOS_MAX + 1] = {
	[IFLA_VLAN_QOS_MAPPING] = { .len = sizeof(struct ifla_vlan_qos_mapping) },
};


static inline int vlan_validate_qos_map(struct nlattr *attr)
{
	if (!attr)
		return 0;
	return nla_validate_nested(attr, IFLA_VLAN_QOS_MAX, vlan_map_policy);
}

// 验证用于vlan的netlink接口参数是否有效
static int vlan_validate(struct nlattr *tb[], struct nlattr *data[])
{
	struct ifla_vlan_flags *flags;
	u16 id;
	int err;

	if (tb[IFLA_ADDRESS]) {
		if (nla_len(tb[IFLA_ADDRESS]) != ETH_ALEN)
			return -EINVAL;
		if (!is_valid_ether_addr(nla_data(tb[IFLA_ADDRESS])))
			return -EADDRNOTAVAIL;
	}

	if (!data)
		return -EINVAL;

	if (data[IFLA_VLAN_PROTOCOL]) {
		switch (nla_get_be16(data[IFLA_VLAN_PROTOCOL])) {
		case __constant_htons(ETH_P_8021Q):
		case __constant_htons(ETH_P_8021AD):
			break;
		default:
			return -EPROTONOSUPPORT;
		}
	}

	if (data[IFLA_VLAN_ID]) {
		id = nla_get_u16(data[IFLA_VLAN_ID]);
		if (id >= VLAN_VID_MASK)
			return -ERANGE;
	}
	if (data[IFLA_VLAN_FLAGS]) {
		flags = nla_data(data[IFLA_VLAN_FLAGS]);
		if ((flags->flags & flags->mask) &
		    ~(VLAN_FLAG_REORDER_HDR | VLAN_FLAG_GVRP |
		      VLAN_FLAG_LOOSE_BINDING | VLAN_FLAG_MVRP))
			return -EINVAL;
	}

	err = vlan_validate_qos_map(data[IFLA_VLAN_INGRESS_QOS]);
	if (err < 0)
		return err;
	err = vlan_validate_qos_map(data[IFLA_VLAN_EGRESS_QOS]);
	if (err < 0)
		return err;
	return 0;
}

// 对一个已经存在的vlan设备进行参数修改(处理来自用户空间设置的参数)
static int vlan_changelink(struct net_device *dev,
			   struct nlattr *tb[], struct nlattr *data[])
{
	struct ifla_vlan_flags *flags;
	struct ifla_vlan_qos_mapping *m;
	struct nlattr *attr;
	int rem;

    // 如果传入了IFLA_VLAN_FLAGS属性则从中获取ifla_vlan_flags结构
	if (data[IFLA_VLAN_FLAGS]) {
		flags = nla_data(data[IFLA_VLAN_FLAGS]);
        // 进而修改vlan_dev_priv->flags标志
		vlan_dev_change_flags(dev, flags->flags, flags->mask);
	}
    // 如果传入了IFLA_VLAN_INGRESS_QOS/IFLA_VLAN_EGRESS_QOS属性，则从中获取ifla_vlan_qos_mapping结构
	if (data[IFLA_VLAN_INGRESS_QOS]) {
		nla_for_each_nested(attr, data[IFLA_VLAN_INGRESS_QOS], rem) {
			m = nla_data(attr);
			vlan_dev_set_ingress_priority(dev, m->to, m->from);
		}
	}
	if (data[IFLA_VLAN_EGRESS_QOS]) {
		nla_for_each_nested(attr, data[IFLA_VLAN_EGRESS_QOS], rem) {
			m = nla_data(attr);
			vlan_dev_set_egress_priority(dev, m->from, m->to);
		}
	}
	return 0;
}

/* 注册并配置一个新的vlan设备
 * @src_net     - 所处的网络命名空间
 * @dev         - 新创建的网络设备
 * @tb[]        - ?
 * @data[]      - ?
 *
 * 备注：对应ioctl接口的函数register_vlan_device
 */
static int vlan_newlink(struct net *src_net, struct net_device *dev,
			struct nlattr *tb[], struct nlattr *data[])
{
    // 获取vlan设备附属的私有空间
	struct vlan_dev_priv *vlan = vlan_dev_priv(dev);
	struct net_device *real_dev;
	__be16 proto;
	int err;

    // 检查是否有传入IFLA_VLAN_ID属性
	if (!data[IFLA_VLAN_ID])
		return -EINVAL;

    // 检查是否有传入IFLA_LINK属性
	if (!tb[IFLA_LINK])
		return -EINVAL;

    // 从IFLA_LINK属性中获取宿主设备接口序号，再根据接口序号索引得到对应的宿主设备
	real_dev = __dev_get_by_index(src_net, nla_get_u32(tb[IFLA_LINK]));
	if (!real_dev)
		return -ENODEV;

    // 如果传入了IFLA_VLAN_PROTOCOL属性则从中获取vlan的协议类型，否则采用缺省的vlan协议类型
	if (data[IFLA_VLAN_PROTOCOL])
		proto = nla_get_be16(data[IFLA_VLAN_PROTOCOL]);
	else
		proto = htons(ETH_P_8021Q);

	vlan->vlan_proto = proto;       // 记录vlan协议类型
	vlan->vlan_id	 = nla_get_u16(data[IFLA_VLAN_ID]); // 记录vlan id
	vlan->real_dev	 = real_dev;    // 记录宿主设备
	vlan->flags	 = VLAN_FLAG_REORDER_HDR;   // vlan设备缺省都会打上VLAN_FLAG_REORDER_HDR标志

    // 检查宿主设备是否支持在其上面创建vlan设备以及要创建的vlan id在该设备上是否已经存在
	err = vlan_check_real_dev(real_dev, vlan->vlan_proto, vlan->vlan_id);
	if (err < 0)
		return err;

    // 如果没有传入IFLA_MTU属性则从宿主设备继承mtu值，否则意味着使用了自定义的mtu值，这时候需要确保自定义值不大于宿主设备上的mtu值
	if (!tb[IFLA_MTU])
		dev->mtu = real_dev->mtu;
	else if (dev->mtu > real_dev->mtu)
		return -EINVAL;

    // 该vlan设备在这里继续处理来自用户空间设置的参数
	err = vlan_changelink(dev, tb, data);
	if (err < 0)
		return err;

    // 进一步注册该vlan设备
	return register_vlan_dev(dev);
}

static inline size_t vlan_qos_map_size(unsigned int n)
{
	if (n == 0)
		return 0;
	/* IFLA_VLAN_{EGRESS,INGRESS}_QOS + n * IFLA_VLAN_QOS_MAPPING */
	return nla_total_size(sizeof(struct nlattr)) +
	       nla_total_size(sizeof(struct ifla_vlan_qos_mapping)) * n;
}

// 计算转储vlan设备netlink属性所需空间大小
static size_t vlan_get_size(const struct net_device *dev)
{
	struct vlan_dev_priv *vlan = vlan_dev_priv(dev);

	return nla_total_size(2) +	/* IFLA_VLAN_PROTOCOL */
	       nla_total_size(2) +	/* IFLA_VLAN_ID */
	       nla_total_size(sizeof(struct ifla_vlan_flags)) + /* IFLA_VLAN_FLAGS */
	       vlan_qos_map_size(vlan->nr_ingress_mappings) +
	       vlan_qos_map_size(vlan->nr_egress_mappings);
}

// 转储vlan设备的netlink属性
static int vlan_fill_info(struct sk_buff *skb, const struct net_device *dev)
{
	struct vlan_dev_priv *vlan = vlan_dev_priv(dev);
	struct vlan_priority_tci_mapping *pm;
	struct ifla_vlan_flags f;
	struct ifla_vlan_qos_mapping m;
	struct nlattr *nest;
	unsigned int i;

	if (nla_put_be16(skb, IFLA_VLAN_PROTOCOL, vlan->vlan_proto) ||
	    nla_put_u16(skb, IFLA_VLAN_ID, vlan->vlan_id))
		goto nla_put_failure;
	if (vlan->flags) {
		f.flags = vlan->flags;
		f.mask  = ~0;
		if (nla_put(skb, IFLA_VLAN_FLAGS, sizeof(f), &f))
			goto nla_put_failure;
	}
	if (vlan->nr_ingress_mappings) {
		nest = nla_nest_start(skb, IFLA_VLAN_INGRESS_QOS);
		if (nest == NULL)
			goto nla_put_failure;

		for (i = 0; i < ARRAY_SIZE(vlan->ingress_priority_map); i++) {
			if (!vlan->ingress_priority_map[i])
				continue;

			m.from = i;
			m.to   = vlan->ingress_priority_map[i];
			if (nla_put(skb, IFLA_VLAN_QOS_MAPPING,
				    sizeof(m), &m))
				goto nla_put_failure;
		}
		nla_nest_end(skb, nest);
	}

	if (vlan->nr_egress_mappings) {
		nest = nla_nest_start(skb, IFLA_VLAN_EGRESS_QOS);
		if (nest == NULL)
			goto nla_put_failure;

		for (i = 0; i < ARRAY_SIZE(vlan->egress_priority_map); i++) {
			for (pm = vlan->egress_priority_map[i]; pm;
			     pm = pm->next) {
				if (!pm->vlan_qos)
					continue;

				m.from = pm->priority;
				m.to   = (pm->vlan_qos >> 13) & 0x7;
				if (nla_put(skb, IFLA_VLAN_QOS_MAPPING,
					    sizeof(m), &m))
					goto nla_put_failure;
			}
		}
		nla_nest_end(skb, nest);
	}
	return 0;

nla_put_failure:
	return -EMSGSIZE;
}

// 定义一个用于vlan的rtnetlink接口操作集合
struct rtnl_link_ops vlan_link_ops __read_mostly = {
	.kind		= "vlan",
	.maxtype	= IFLA_VLAN_MAX,
	.policy		= vlan_policy,
	.priv_size	= sizeof(struct vlan_dev_priv),
	.setup		= vlan_setup,
	.validate	= vlan_validate,
	.newlink	= vlan_newlink,
	.changelink	= vlan_changelink,
	.dellink	= unregister_vlan_dev,
	.get_size	= vlan_get_size,
	.fill_info	= vlan_fill_info,
};

// 初始化操作vlan用的netlink接口
int __init vlan_netlink_init(void)
{
    // 将定义好的vlan_link_ops注册到rtnetlink接口中
	return rtnl_link_register(&vlan_link_ops);
}

// 注销操作vlan用的netlink接口
void __exit vlan_netlink_fini(void)
{
    // 将vlan_link_ops从rtnetlink接口中注销
	rtnl_link_unregister(&vlan_link_ops);
}

MODULE_ALIAS_RTNL_LINK("vlan");
