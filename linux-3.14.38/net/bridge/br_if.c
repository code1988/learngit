/*
 *	Userspace interface
 *	Linux ethernet bridge
 *	用户空间对应的功能接口
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
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/netpoll.h>
#include <linux/ethtool.h>
#include <linux/if_arp.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/rtnetlink.h>
#include <linux/if_ether.h>
#include <linux/slab.h>
#include <net/sock.h>
#include <linux/if_vlan.h>

#include "br_private.h"

/*
 * Determine initial path cost based on speed.
 * 计算桥端口的路径成本（跟设备的速率相关）
 * using recommendations from 802.1d standard
 *
 * Since driver might sleep need to not be holding any locks.
 */
static int port_cost(struct net_device *dev)
{
	struct ethtool_cmd ecmd;

	if (!__ethtool_get_settings(dev, &ecmd)) {
		switch (ethtool_cmd_speed(&ecmd)) {
		case SPEED_10000:
			return 2;
		case SPEED_1000:
			return 4;
		case SPEED_100:
			return 19;
		case SPEED_10:
			return 100;
		}
	}

	/* Old silly heuristics based on name */
	if (!strncmp(dev->name, "lec", 3))
		return 7;

	if (!strncmp(dev->name, "plip", 4))
		return 2500;

	return 100;	/* assume old 10Mbps */
}


/* Check for port carrier transitions. */
void br_port_carrier_check(struct net_bridge_port *p)
{
	struct net_device *dev = p->dev;
	struct net_bridge *br = p->br;

	if (!(p->flags & BR_ADMIN_COST) &&
	    netif_running(dev) && netif_oper_up(dev))
		p->path_cost = port_cost(dev);

	if (!netif_running(br->dev))
		return;

	spin_lock_bh(&br->lock);
	if (netif_running(dev) && netif_oper_up(dev)) {
		if (p->state == BR_STATE_DISABLED)
			br_stp_enable_port(p);
	} else {
		if (p->state != BR_STATE_DISABLED)
			br_stp_disable_port(p);
	}
	spin_unlock_bh(&br->lock);
}

static void release_nbp(struct kobject *kobj)
{
	struct net_bridge_port *p
		= container_of(kobj, struct net_bridge_port, kobj);
	kfree(p);
}

static struct kobj_type brport_ktype = {
#ifdef CONFIG_SYSFS
	.sysfs_ops = &brport_sysfs_ops,
#endif
	.release = release_nbp,
};

static void destroy_nbp(struct net_bridge_port *p)
{
	struct net_device *dev = p->dev;

	p->br = NULL;
	p->dev = NULL;
	dev_put(dev);

	kobject_put(&p->kobj);
}

static void destroy_nbp_rcu(struct rcu_head *head)
{
	struct net_bridge_port *p =
			container_of(head, struct net_bridge_port, rcu);
	destroy_nbp(p);
}

/* Delete port(interface) from bridge is done in two steps.
 * via RCU. First step, marks device as down. That deletes
 * all the timers and stops new packets from flowing through.
 *
 * Final cleanup doesn't occur until after all CPU's finished
 * processing packets.
 *
 * Protected from multiple admin operations by RTNL mutex
 */
static void del_nbp(struct net_bridge_port *p)
{
	struct net_bridge *br = p->br;
	struct net_device *dev = p->dev;

	sysfs_remove_link(br->ifobj, p->dev->name);

	dev_set_promiscuity(dev, -1);

	spin_lock_bh(&br->lock);
	br_stp_disable_port(p);
	spin_unlock_bh(&br->lock);

	br_ifinfo_notify(RTM_DELLINK, p);

	nbp_vlan_flush(p);
	br_fdb_delete_by_port(br, p, 1);

	list_del_rcu(&p->list);

	dev->priv_flags &= ~IFF_BRIDGE_PORT;

	netdev_rx_handler_unregister(dev);

	netdev_upper_dev_unlink(dev, br->dev);

	br_multicast_del_port(p);

	kobject_uevent(&p->kobj, KOBJ_REMOVE);
	kobject_del(&p->kobj);

	br_netpoll_disable(p);

	call_rcu(&p->rcu, destroy_nbp_rcu);
}

/* Delete bridge device */
void br_dev_delete(struct net_device *dev, struct list_head *head)
{
	struct net_bridge *br = netdev_priv(dev);
	struct net_bridge_port *p, *n;

	list_for_each_entry_safe(p, n, &br->port_list, list) {
		del_nbp(p);
	}

	br_fdb_delete_by_port(br, NULL, 1);

	br_vlan_flush(br);
	del_timer_sync(&br->gc_timer);

	br_sysfs_delbr(br->dev);
	unregister_netdevice_queue(br->dev, head);
}

/* find an available port number 
 * 分配一个未占用的桥端口号
 * */
static int find_portno(struct net_bridge *br)
{
	int index;
	struct net_bridge_port *p;
	unsigned long *inuse;

	inuse = kcalloc(BITS_TO_LONGS(BR_MAX_PORTS), sizeof(unsigned long),
			GFP_KERNEL);
	if (!inuse)
		return -ENOMEM;

	set_bit(0, inuse);	/* zero is reserved */
	list_for_each_entry(p, &br->port_list, list) {
		set_bit(p->port_no, inuse);
	}
	index = find_first_zero_bit(inuse, BR_MAX_PORTS);
	kfree(inuse);

	return (index >= BR_MAX_PORTS) ? -EXFULL : index;
}

/* called with RTNL but without bridge lock 
 * 为加入网桥的设备分配一个net_bridge_port结构，并进行了基本的初始化
 * @br  - 指向要加入的网桥
 * @dev - 指向要加入网桥的设备
 * */
static struct net_bridge_port *new_nbp(struct net_bridge *br,
				       struct net_device *dev)
{
	int index;
	struct net_bridge_port *p;

    // 分配一个未占用的桥端口号
	index = find_portno(br);
	if (index < 0)
		return ERR_PTR(index);

	p = kzalloc(sizeof(*p), GFP_KERNEL);
	if (p == NULL)
		return ERR_PTR(-ENOMEM);

	p->br = br;     // 跟所属的bridge设备关联
	dev_hold(dev);  // 要加入网桥设备中的引用计数加1
	p->dev = dev;   // 跟对应的网络设备关联
	p->path_cost = port_cost(dev);  // 计算该桥端口的路径成本
	p->priority = 0x8000 >> BR_PORT_BITS;   // 设置桥端口缺省优先级
	p->port_no = index;                     // 设置桥端口号
	p->flags = BR_LEARNING | BR_FLOOD;      // 设置桥端口的缺省属性：支持学习和泛洪
	br_init_port(p);                        // 桥端口的一部分初始化
	p->state = BR_STATE_DISABLED;           // 桥端口缺省处于disable状态
	br_stp_port_timer_init(p);              // 桥端口stp相关定时器初始化
	br_multicast_add_port(p);               // 初始化桥端口的igmp-snooping功能          

	return p;
}

/* 创建一个网桥
 * @name    - 网桥设备名
 */
int br_add_bridge(struct net *net, const char *name)
{
	struct net_device *dev;
	int res;

    /* 创建一个bridge设备，附带一个net_bridge结构的私有空间
     * 创建完毕后会执行br_dev_setup来完成一些基本的初始化
     */
	dev = alloc_netdev(sizeof(struct net_bridge), name,
			   br_dev_setup);

	if (!dev)
		return -ENOMEM;

    // 将新创建的bridge设备关联到当前网络命名空间
	dev_net_set(dev, net);
    // 为该bridge设备绑定一组用于处理link事件的rtnetlink操作集合
	dev->rtnl_link_ops = &br_link_ops;

    // 注册bridge设备到内核中，注册结果会从通知链中反馈
	res = register_netdev(dev);
	if (res)
		free_netdev(dev);
	return res;
}

/* 删除一个指定的网桥
 * @name    - 网桥设备名
 */
int br_del_bridge(struct net *net, const char *name)
{
	struct net_device *dev;
	int ret = 0;

	rtnl_lock();
	dev = __dev_get_by_name(net, name);
	if (dev == NULL)
		ret =  -ENXIO; 	/* Could not find device */

	else if (!(dev->priv_flags & IFF_EBRIDGE)) {
		/* Attempt to delete non bridge device! */
		ret = -EPERM;
	}

	else if (dev->flags & IFF_UP) {
		/* Not shutdown yet. */
		ret = -EBUSY;
	}

	else
		br_dev_delete(dev, NULL);

	rtnl_unlock();
	return ret;
}

/* MTU of the bridge pseudo-device: ETH_DATA_LEN or the minimum of the ports 
 * 返回指定网桥的MTU
 * */
int br_min_mtu(const struct net_bridge *br)
{
	const struct net_bridge_port *p;
	int mtu = 0;

	ASSERT_RTNL();

    // 如果不存在桥端口，那么就是缺省值
	if (list_empty(&br->port_list))
		mtu = ETH_DATA_LEN;
	else {
        // 如果存在桥端口，那么就是最小的桥端口设备MTU
		list_for_each_entry(p, &br->port_list, list) {
			if (!mtu  || p->dev->mtu < mtu)
				mtu = p->dev->mtu;
		}
	}
	return mtu;
}

/*
 * Recomputes features using slave's features
 */
netdev_features_t br_features_recompute(struct net_bridge *br,
	netdev_features_t features)
{
	struct net_bridge_port *p;
	netdev_features_t mask;

	if (list_empty(&br->port_list))
		return features;

	mask = features;
	features &= ~NETIF_F_ONE_FOR_ALL;

	list_for_each_entry(p, &br->port_list, list) {
		features = netdev_increment_features(features,
						     p->dev->features, mask);
	}

	return features;
}

/* called with RTNL 
 * 将指定设备作为桥端口加入指定网桥
 * @br  - 指向一个网桥
 * @dev - 指向要加入网桥的设备
 * */
int br_add_if(struct net_bridge *br, struct net_device *dev)
{
	struct net_bridge_port *p;
	int err = 0;
	bool changed_addr;

	/* Don't allow bridging non-ethernet like devices 
     * 不能将非以太网设备加入网桥
     * */
	if ((dev->flags & IFF_LOOPBACK) ||
	    dev->type != ARPHRD_ETHER || dev->addr_len != ETH_ALEN ||
	    !is_valid_ether_addr(dev->dev_addr))
		return -EINVAL;

	/* No bridging of bridges 
     * 不能将网桥设备作为端口加入一个网桥
     * */
	if (dev->netdev_ops->ndo_start_xmit == br_dev_xmit)
		return -ELOOP;

	/* Device is already being bridged 
     * 不能将已经加入网桥的设备再加入网桥
     * */
	if (br_port_exists(dev))
		return -EBUSY;

	/* No bridging devices that dislike that (e.g. wireless) 
     * 不能将带了IFF_DONT_BRIDGE标志的设备加入网桥
     * */
	if (dev->priv_flags & IFF_DONT_BRIDGE)
		return -EOPNOTSUPP;

    // 为加入网桥的设备分配一个网桥端口对象net_bridge_port，并进行了基本的初始化
	p = new_nbp(br, dev);
	if (IS_ERR(p))
		return PTR_ERR(p);

    // 调用桥端口设备关联的所有网络通知块
	call_netdevice_notifiers(NETDEV_JOIN, dev);

    // 桥端口设备请求进入混杂模式
	err = dev_set_promiscuity(dev, 1);
	if (err)
		goto put_back;

    // 初始化桥端口中的kobject结构，并将其加入到kobject的层次结构中
	err = kobject_init_and_add(&p->kobj, &brport_ktype, &(dev->dev.kobj),
				   SYSFS_BRIDGE_PORT_ATTR);
	if (err)
		goto err1;

    // 桥端口加入sysfs中，暂略
	err = br_sysfs_addif(p);
	if (err)
		goto err2;

    // 暂略
	err = br_netpoll_enable(p, GFP_KERNEL);
	if (err)
		goto err3;

    // 将桥设备作为master节点加入到桥端口设备的直接上级设备中
	err = netdev_master_upper_dev_link(dev, br->dev);
	if (err)
		goto err4;

    // 将br_handle_frame注册到桥端口设备的rx_handler
	err = netdev_rx_handler_register(dev, br_handle_frame, p);
	if (err)
		goto err5;

    // 桥端口设备都会有IFF_BRIDGE_PORT标志
	dev->priv_flags |= IFF_BRIDGE_PORT;

    // 禁用桥端口设备的LRO功能(因为桥端口可能会对收到的报文进行转发)
	dev_disable_lro(dev);

    // 将该桥端口加入桥的端口链表
	list_add_rcu(&p->list, &br->port_list);

    // 重新计算桥端口设备的features字段，如果有变化还将发送通知
	netdev_update_features(br->dev);

    // 确保桥设备的needed_headroom不小于桥端口的needed_headroom
	if (br->dev->needed_headroom < dev->needed_headroom)
		br->dev->needed_headroom = dev->needed_headroom;

    // 将桥端口设备的mac地址作为本地地址加入到转发表中
	if (br_fdb_insert(br, p, dev->dev_addr, 0))
		netdev_err(dev, "failed insert local address bridge forwarding table\n");

	spin_lock_bh(&br->lock);
    // 重新计算网桥ID
	changed_addr = br_stp_recalculate_bridge_id(br);

    /* 如果同时满足以下3个条件，则执行该网桥端口的stp功能
     *      [1]. 该桥端口设备处于启用状态
     *      [2]. 该桥端口设备处于可操作状态
     *      [3]. 该网桥设备处于UP状态
     */
	if (netif_running(dev) && netif_oper_up(dev) &&
	    (br->dev->flags & IFF_UP))
		br_stp_enable_port(p);
	spin_unlock_bh(&br->lock);

    // 将"新端口加入桥"事件通过调用rtnetlink接口通知相关的用户进程
	br_ifinfo_notify(RTM_NEWLINK, p);

    // 如果网桥ID有变化，则调用网桥的设备通知链
	if (changed_addr)
		call_netdevice_notifiers(NETDEV_CHANGEADDR, br->dev);

    // 更新网桥的mtu
	dev_set_mtu(br->dev, br_min_mtu(br));

    // 通知用户空间有一个新的kobject加入，显然该kobject关联了一个网桥的新端口
	kobject_uevent(&p->kobj, KOBJ_ADD);

	return 0;

err5:
	netdev_upper_dev_unlink(dev, br->dev);
err4:
	br_netpoll_disable(p);
err3:
	sysfs_remove_link(br->ifobj, p->dev->name);
err2:
	kobject_put(&p->kobj);
	p = NULL; /* kobject_put frees */
err1:
	dev_set_promiscuity(dev, -1);
put_back:
	dev_put(dev);
	kfree(p);
	return err;
}

/* called with RTNL */
int br_del_if(struct net_bridge *br, struct net_device *dev)
{
	struct net_bridge_port *p;
	bool changed_addr;

	p = br_port_get_rtnl(dev);
	if (!p || p->br != br)
		return -EINVAL;

	/* Since more than one interface can be attached to a bridge,
	 * there still maybe an alternate path for netconsole to use;
	 * therefore there is no reason for a NETDEV_RELEASE event.
	 */
	del_nbp(p);

	spin_lock_bh(&br->lock);
	changed_addr = br_stp_recalculate_bridge_id(br);
	spin_unlock_bh(&br->lock);

	if (changed_addr)
		call_netdevice_notifiers(NETDEV_CHANGEADDR, br->dev);

	netdev_update_features(br->dev);

	return 0;
}
