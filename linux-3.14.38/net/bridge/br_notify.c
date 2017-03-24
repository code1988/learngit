/*
 *	Device event handling
 *	Linux ethernet bridge
 *	操作网桥用的通知链
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
#include <linux/rtnetlink.h>
#include <net/net_namespace.h>

#include "br_private.h"

static int br_device_event(struct notifier_block *unused, unsigned long event, void *ptr);

// 定义了一个用于网桥的通知链节点
struct notifier_block br_device_notifier = {
	.notifier_call = br_device_event
};

/*
 * Handle changes in state of network devices enslaved to a bridge.
 * 属于网桥的网络设备状态变化通知回调函数
 *
 * Note: don't care about up/down if bridge itself is down, because
 *     port state is checked when bridge is brought up.
 * 备注： 只有当网桥本身处于up状态时，它才会去关心所属设备的up/down事件
 */
static int br_device_event(struct notifier_block *unused, unsigned long event, void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);  // 获取该事件源所属的网络设备
	struct net_bridge_port *p;
	struct net_bridge *br;
	bool changed_addr;
	int err;

	/* register of bridge completed, add sysfs entries */
	if ((dev->priv_flags & IFF_EBRIDGE) && event == NETDEV_REGISTER) {
		br_sysfs_addbr(dev);
		return NOTIFY_DONE;
	}

	/* not a port of a bridge 
     * 获取该网络设备对应的桥端口
     * */
	p = br_port_get_rtnl(dev);
	if (!p)
		return NOTIFY_DONE;

    // 获取该桥端口所属的网桥
	br = p->br;

    // 判断触发本回调的事件源
	switch (event) {
	case NETDEV_CHANGEMTU:
		dev_set_mtu(br->dev, br_min_mtu(br));
		break;

	case NETDEV_CHANGEADDR:
		spin_lock_bh(&br->lock);
		br_fdb_changeaddr(p, dev->dev_addr);
		changed_addr = br_stp_recalculate_bridge_id(br);
		spin_unlock_bh(&br->lock);

		if (changed_addr)
			call_netdevice_notifiers(NETDEV_CHANGEADDR, br->dev);

		break;

	case NETDEV_CHANGE:
		br_port_carrier_check(p);
		break;

	case NETDEV_FEAT_CHANGE:
		netdev_update_features(br->dev);
		break;

	case NETDEV_DOWN:       // 设备down事件
		spin_lock_bh(&br->lock);
        // 如果网桥本身处于up状态，则会disable桥端口
		if (br->dev->flags & IFF_UP)
			br_stp_disable_port(p);
		spin_unlock_bh(&br->lock);
		break;

	case NETDEV_UP:         // 设备up事件
        // 如果网桥本身处于up状态，并且该网络设备处于可操作状态，则会enable桥端口
		if (netif_running(br->dev) && netif_oper_up(dev)) {
			spin_lock_bh(&br->lock);
			br_stp_enable_port(p);
			spin_unlock_bh(&br->lock);
		}
		break;

	case NETDEV_UNREGISTER:
		br_del_if(br, dev);
		break;

	case NETDEV_CHANGENAME:
		err = br_sysfs_renameif(p);
		if (err)
			return notifier_from_errno(err);
		break;

	case NETDEV_PRE_TYPE_CHANGE:
		/* Forbid underlaying device to change its type. */
		return NOTIFY_BAD;

	case NETDEV_RESEND_IGMP:
		/* Propagate to master device */
		call_netdevice_notifiers(event, br->dev);
		break;
	}

	/* Events that may cause spanning tree to refresh 
     * 这些事件源可能意味着需要刷新生成树
     * */
	if (event == NETDEV_CHANGEADDR || event == NETDEV_UP ||
	    event == NETDEV_CHANGE || event == NETDEV_DOWN)
		br_ifinfo_notify(RTM_NEWLINK, p);

	return NOTIFY_DONE;
}
