/*
 *	Spanning tree protocol; interface code
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
#include <linux/kmod.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
#include <net/switchdev.h>

#include "br_private.h"
#include "br_private_stp.h"


/* Port id is composed of priority and port number.
 * 根据优先级和桥端口号计算用于stp的端口ID号
 *          高6位(优先级) | 低10位(桥端口号)
 * NB: some bits of priority are dropped to
 *     make room for more ports.
 */
static inline port_id br_make_port_id(__u8 priority, __u16 port_no)
{
	return ((u16)priority << BR_PORT_BITS)
		| (port_no & ((1<<BR_PORT_BITS)-1));
}

#define BR_MAX_PORT_PRIORITY ((u16)~0 >> BR_PORT_BITS)

/* called under bridge lock 
 * 桥端口的一部分STP功能初始化，包括计算端口ID、设置该桥端口为"指定端口"、端口设置BLOCKING等
 *
 * 备注：STP网桥初始都默认自己的是"根桥"，STP端口初始都默认自己是"指定端口"
 * */
void br_init_port(struct net_bridge_port *p)
{
	struct switchdev_attr attr = {
		.id = SWITCHDEV_ATTR_ID_BRIDGE_AGEING_TIME,
		.flags = SWITCHDEV_F_SKIP_EOPNOTSUPP | SWITCHDEV_F_DEFER,
		.u.ageing_time = jiffies_to_clock_t(p->br->ageing_time),
	};
	int err;

    // 根据优先级和桥端口号计算用于stp的端口ID号
	p->port_id = br_make_port_id(p->priority, p->port_no);
	br_become_designated_port(p);
	br_set_state(p, BR_STATE_BLOCKING);
	p->topology_change_ack = 0;
	p->config_pending = 0;

	err = switchdev_port_attr_set(p->dev, &attr);
	if (err && err != -EOPNOTSUPP)
		netdev_err(p->dev, "failed to set HW ageing time\n");
}

/* called under bridge lock */
void br_stp_enable_bridge(struct net_bridge *br)
{
	struct net_bridge_port *p;

	spin_lock_bh(&br->lock);
	if (br->stp_enabled == BR_KERNEL_STP)
		mod_timer(&br->hello_timer, jiffies + br->hello_time);
	mod_timer(&br->gc_timer, jiffies + HZ/10);

	br_config_bpdu_generation(br);

	list_for_each_entry(p, &br->port_list, list) {
		if (netif_running(p->dev) && netif_oper_up(p->dev))
			br_stp_enable_port(p);

	}
	spin_unlock_bh(&br->lock);
}

/* NO locks held */
void br_stp_disable_bridge(struct net_bridge *br)
{
	struct net_bridge_port *p;

	spin_lock_bh(&br->lock);
	list_for_each_entry(p, &br->port_list, list) {
		if (p->state != BR_STATE_DISABLED)
			br_stp_disable_port(p);

	}

	br->topology_change = 0;
	br->topology_change_detected = 0;
	spin_unlock_bh(&br->lock);

	del_timer_sync(&br->hello_timer);
	del_timer_sync(&br->topology_change_timer);
	del_timer_sync(&br->tcn_timer);
	del_timer_sync(&br->gc_timer);
}

/* called under bridge lock 
 * 使能端口
 *
 * 备注：调用本函数的不一定是内核stp的端口，也可以是没开启stp功能的普通桥端口或开启了用户态stp的端口
 * */
void br_stp_enable_port(struct net_bridge_port *p)
{
    // 桥端口的一部分STP功能初始化
	br_init_port(p);
    // 桥端口状态更新
	br_port_state_selection(p->br);
    // 将STP端口状态切换的事件通知用户空间相关的监听者(log && netlink标记第4次)
	br_log_state(p);
	br_ifinfo_notify(RTM_NEWLINK, p);
}

/* called under bridge lock 
 * 禁用端口
 * */
void br_stp_disable_port(struct net_bridge_port *p)
{
	struct net_bridge *br = p->br;
	int wasroot;

	wasroot = br_is_root_bridge(br);
	br_become_designated_port(p);
	br_set_state(p, BR_STATE_DISABLED);
	p->topology_change_ack = 0;
	p->config_pending = 0;

    // 将STP端口状态切换的事件通知用户空间相关的监听者(log && netlink标记第5次)
	br_log_state(p);
	br_ifinfo_notify(RTM_NEWLINK, p);

	del_timer(&p->message_age_timer);
	del_timer(&p->forward_delay_timer);
	del_timer(&p->hold_timer);

	br_fdb_delete_by_port(br, p, 0, 0);
	br_multicast_disable_port(p);

	br_configuration_update(br);

	br_port_state_selection(br);

	if (br_is_root_bridge(br) && !wasroot)
		br_become_root_bridge(br);
}

static void br_stp_start(struct net_bridge *br)
{
	int r;
	char *argv[] = { BR_STP_PROG, br->dev->name, "start", NULL };
	char *envp[] = { NULL };
	struct net_bridge_port *p;

	if (net_eq(dev_net(br->dev), &init_net))
		r = call_usermodehelper(BR_STP_PROG, argv, envp, UMH_WAIT_PROC);
	else
		r = -ENOENT;

	spin_lock_bh(&br->lock);

	if (br->bridge_forward_delay < BR_MIN_FORWARD_DELAY)
		__br_set_forward_delay(br, BR_MIN_FORWARD_DELAY);
	else if (br->bridge_forward_delay > BR_MAX_FORWARD_DELAY)
		__br_set_forward_delay(br, BR_MAX_FORWARD_DELAY);

	if (r == 0) {
		br->stp_enabled = BR_USER_STP;
		br_debug(br, "userspace STP started\n");
		/* Stop hello and hold timers */
		del_timer(&br->hello_timer);
		list_for_each_entry(p, &br->port_list, list)
			del_timer(&p->hold_timer);
	} else {
		br->stp_enabled = BR_KERNEL_STP;
		br_debug(br, "using kernel STP\n");

		/* To start timers on any ports left in blocking */
		br_port_state_selection(br);
	}

	spin_unlock_bh(&br->lock);
}

static void br_stp_stop(struct net_bridge *br)
{
	int r;
	char *argv[] = { BR_STP_PROG, br->dev->name, "stop", NULL };
	char *envp[] = { NULL };
	struct net_bridge_port *p;

	if (br->stp_enabled == BR_USER_STP) {
		r = call_usermodehelper(BR_STP_PROG, argv, envp, UMH_WAIT_PROC);
		br_info(br, "userspace STP stopped, return code %d\n", r);

		/* To start timers on any ports left in blocking */
		mod_timer(&br->hello_timer, jiffies + br->hello_time);
		list_for_each_entry(p, &br->port_list, list)
			mod_timer(&p->hold_timer,
				  round_jiffies(jiffies + BR_HOLD_TIME));
		spin_lock_bh(&br->lock);
		br_port_state_selection(br);
		spin_unlock_bh(&br->lock);
	}

	br->stp_enabled = BR_NO_STP;
}

void br_stp_set_enabled(struct net_bridge *br, unsigned long val)
{
	ASSERT_RTNL();

	if (val) {
		if (br->stp_enabled == BR_NO_STP)
			br_stp_start(br);
	} else {
		if (br->stp_enabled != BR_NO_STP)
			br_stp_stop(br);
	}
}

/* called under bridge lock 
 * 修改网桥ID，这里实际就是修改网桥MAC
 * @br  - 指向一个网桥
 * @addr    - 指向一个MAC
 * */
void br_stp_change_bridge_id(struct net_bridge *br, const unsigned char *addr)
{
	/* should be aligned on 2 bytes for ether_addr_equal() */
	unsigned short oldaddr_aligned[ETH_ALEN >> 1];
	unsigned char *oldaddr = (unsigned char *)oldaddr_aligned;
	struct net_bridge_port *p;
	int wasroot;

    // 判断该桥是否为根桥
	wasroot = br_is_root_bridge(br);

	br_fdb_change_mac_address(br, addr);

	memcpy(oldaddr, br->bridge_id.addr, ETH_ALEN);
	memcpy(br->bridge_id.addr, addr, ETH_ALEN);
	memcpy(br->dev->dev_addr, addr, ETH_ALEN);

	list_for_each_entry(p, &br->port_list, list) {
		if (ether_addr_equal(p->designated_bridge.addr, oldaddr))
			memcpy(p->designated_bridge.addr, addr, ETH_ALEN);

		if (ether_addr_equal(p->designated_root.addr, oldaddr))
			memcpy(p->designated_root.addr, addr, ETH_ALEN);
	}

	br_configuration_update(br);
	br_port_state_selection(br);
	if (br_is_root_bridge(br) && !wasroot)
		br_become_root_bridge(br);
}

/* should be aligned on 2 bytes for ether_addr_equal() 
 * 这个常量数组定义了一个全0的MAC地址
 * */
static const unsigned short br_mac_zero_aligned[ETH_ALEN >> 1];

/* called under bridge lock 
 * 重新计算网桥ID，如果有变化则返回true，否则返回false
 * */
bool br_stp_recalculate_bridge_id(struct net_bridge *br)
{
	const unsigned char *br_mac_zero =
			(const unsigned char *)br_mac_zero_aligned;
	const unsigned char *addr = br_mac_zero;
	struct net_bridge_port *p;

	/* user has chosen a value so keep it 
     * 如果该桥设备的MAC地址是手动分配而来的，则无需重新计算，直接返回
     * */
	if (br->dev->addr_assign_type == NET_ADDR_SET)
		return false;

    // 遍历桥的每个桥端口，查找端口设备的最小MAC
	list_for_each_entry(p, &br->port_list, list) {
		if (addr == br_mac_zero ||
		    memcmp(p->dev->dev_addr, addr, ETH_ALEN) < 0)
			addr = p->dev->dev_addr;

	}

    // 比较桥MAC和最小的桥端口MAC是否相同，相同则无需重新计算，直接返回
	if (ether_addr_equal(br->bridge_id.addr, addr))
		return false;	/* no change */

    // 程序运行到这里意味着当前最小的桥端口MAC已经小于桥MAC
    // 所以需要修改桥MAC
	br_stp_change_bridge_id(br, addr);
	return true;
}

/* Acquires and releases bridge lock */
void br_stp_set_bridge_priority(struct net_bridge *br, u16 newprio)
{
	struct net_bridge_port *p;
	int wasroot;

	spin_lock_bh(&br->lock);
	wasroot = br_is_root_bridge(br);

	list_for_each_entry(p, &br->port_list, list) {
		if (p->state != BR_STATE_DISABLED &&
		    br_is_designated_port(p)) {
			p->designated_bridge.prio[0] = (newprio >> 8) & 0xFF;
			p->designated_bridge.prio[1] = newprio & 0xFF;
		}

	}

	br->bridge_id.prio[0] = (newprio >> 8) & 0xFF;
	br->bridge_id.prio[1] = newprio & 0xFF;
	br_configuration_update(br);
	br_port_state_selection(br);
	if (br_is_root_bridge(br) && !wasroot)
		br_become_root_bridge(br);
	spin_unlock_bh(&br->lock);
}

/* called under bridge lock */
int br_stp_set_port_priority(struct net_bridge_port *p, unsigned long newprio)
{
	port_id new_port_id;

	if (newprio > BR_MAX_PORT_PRIORITY)
		return -ERANGE;

	new_port_id = br_make_port_id(newprio, p->port_no);
	if (br_is_designated_port(p))
		p->designated_port = new_port_id;

	p->port_id = new_port_id;
	p->priority = newprio;
	if (!memcmp(&p->br->bridge_id, &p->designated_bridge, 8) &&
	    p->port_id < p->designated_port) {
		br_become_designated_port(p);
		br_port_state_selection(p->br);
	}

	return 0;
}

/* called under bridge lock */
int br_stp_set_path_cost(struct net_bridge_port *p, unsigned long path_cost)
{
	if (path_cost < BR_MIN_PATH_COST ||
	    path_cost > BR_MAX_PATH_COST)
		return -ERANGE;

	p->flags |= BR_ADMIN_COST;
	p->path_cost = path_cost;
	br_configuration_update(p->br);
	br_port_state_selection(p->br);
	return 0;
}

ssize_t br_show_bridge_id(char *buf, const struct bridge_id *id)
{
	return sprintf(buf, "%.2x%.2x.%.2x%.2x%.2x%.2x%.2x%.2x\n",
	       id->prio[0], id->prio[1],
	       id->addr[0], id->addr[1], id->addr[2],
	       id->addr[3], id->addr[4], id->addr[5]);
}
