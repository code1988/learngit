/*
 *	Spanning tree protocol; timer-related code
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
#include <linux/times.h>

#include "br_private.h"
#include "br_private_stp.h"

/* called under bridge lock 
 * 检查该网桥是否存在"指定端口"
 * @返回值： 1-存在， 0-不存在
 * */
static int br_is_designated_for_some_port(const struct net_bridge *br)
{
	struct net_bridge_port *p;

	list_for_each_entry(p, &br->port_list, list) {
		if (p->state != BR_STATE_DISABLED &&
		    !memcmp(&p->designated_bridge, &br->bridge_id, 8))
			return 1;
	}

	return 0;
}

/* "根桥"的hello定时器超时函数，主要是定时发送配置BPDU信息
 * @arg - 指向struct net_bridge结构
 */
static void br_hello_timer_expired(unsigned long arg)
{
	struct net_bridge *br = (struct net_bridge *)arg;

	br_debug(br, "hello timer expired\n");
	spin_lock(&br->lock);
    // 确保该桥端口设备处于up状态，才能发送BPDU
	if (br->dev->flags & IFF_UP) {
		br_config_bpdu_generation(br);

		mod_timer(&br->hello_timer, round_jiffies(jiffies + br->hello_time));
	}
	spin_unlock(&br->lock);
}

/* "根端口"的message_age定时器超时处理函数，触发意味着丢失了邻居
 * 
 */
static void br_message_age_timer_expired(unsigned long arg)
{
	struct net_bridge_port *p = (struct net_bridge_port *) arg;
	struct net_bridge *br = p->br;
	const bridge_id *id = &p->designated_bridge;
	int was_root;

	if (p->state == BR_STATE_DISABLED)
		return;

	br_info(br, "port %u(%s) neighbor %.2x%.2x.%pM lost\n",
		(unsigned int) p->port_no, p->dev->name,
		id->prio[0], id->prio[1], &id->addr);

	/*
	 * According to the spec, the message age timer cannot be
	 * running when we are the root bridge. So..  this was_root
	 * check is redundant. I'm leaving it in for now, though.
     * 注释也说了，本定时器不可能运行在"根桥"中，所以下面判断"根桥"是多余的
	 */
	spin_lock(&br->lock);
	if (p->state == BR_STATE_DISABLED)
		goto unlock;
	was_root = br_is_root_bridge(br);

    // 丢失邻居后的"根端口"将变成"指定端口"
	br_become_designated_port(p);
    // 更新该网桥的STP配置信息
	br_configuration_update(br);
    // 执行端口状态更新流程
	br_port_state_selection(br);
    // 如果该网桥从"非根桥"变成"根桥"，需要执行相关流程
	if (br_is_root_bridge(br) && !was_root)
		br_become_root_bridge(br);
 unlock:
	spin_unlock(&br->lock);
}

/* STP端口的状态转换延时定时器
 * STP端口从listening->learning以及从learning->forwarding状态转换需要等待该定时器超时
 */
static void br_forward_delay_timer_expired(unsigned long arg)
{
	struct net_bridge_port *p = (struct net_bridge_port *) arg;
	struct net_bridge *br = p->br;

	br_debug(br, "port %u(%s) forward delay timer\n",
		 (unsigned int) p->port_no, p->dev->name);
	spin_lock(&br->lock);
	if (p->state == BR_STATE_LISTENING) {
        // listening->learning，并刷新该定时器
		p->state = BR_STATE_LEARNING;
		mod_timer(&p->forward_delay_timer,
			  jiffies + br->forward_delay);
	} else if (p->state == BR_STATE_LEARNING) {
        // learning->forwarding的流程
		p->state = BR_STATE_FORWARDING;
        // 如果所在网桥存在"指定端口"，就进入拓扑变化流程
		if (br_is_designated_for_some_port(br))
			br_topology_change_detection(br);
		netif_carrier_on(br->dev);
	}
	br_log_state(p);
	br_ifinfo_notify(RTM_NEWLINK, p);
	spin_unlock(&br->lock);
}

/* "非根桥"的tcn定时器超时函数，
 * 当"非根桥"自身拓扑发生变化或者从"指定端口"收到TCN-BPDU时，就会定时往"根端口"发送TCN-BPDU信息，
 * 直到收到TCA回复
 */
static void br_tcn_timer_expired(unsigned long arg)
{
	struct net_bridge *br = (struct net_bridge *) arg;

	br_debug(br, "tcn timer expired\n");
	spin_lock(&br->lock);
    // 只有处于up状态的"非根桥"设备才会发TCN-BPDU信息
	if (!br_is_root_bridge(br) && (br->dev->flags & IFF_UP)) {
		br_transmit_tcn(br);

        // 发完刷新该定时器
		mod_timer(&br->tcn_timer, jiffies + br->bridge_hello_time);
	}
	spin_unlock(&br->lock);
}

/* "根桥"的topology_change定时器超时处理函数，主要就是清除拓扑变化标志(即意味着结束发送TC置1的配置BPDU信息)
 * 当"根桥"自身拓扑发生变化或者从"指定端口"收到TCN-BPDU时，就会往"指定端口"发送TC置1的配置BPDU信息，
 * 直到topology_change定时器超时才结束发送 
 */
static void br_topology_change_timer_expired(unsigned long arg)
{
	struct net_bridge *br = (struct net_bridge *) arg;

	br_debug(br, "topo change timer expired\n");
	spin_lock(&br->lock);
	br->topology_change_detected = 0;
	br->topology_change = 0;
	spin_unlock(&br->lock);
}

/* STP桥端口的hold定时器超时处理函数，主要就是处理可能存在的下一个配置发送请求
 * 如果存在下一个配置BPDU发送请求，就在这里发送
 */
static void br_hold_timer_expired(unsigned long arg)
{
	struct net_bridge_port *p = (struct net_bridge_port *) arg;

	br_debug(p->br, "port %u(%s) hold timer expired\n",
		 (unsigned int) p->port_no, p->dev->name);

	spin_lock(&p->br->lock);
	if (p->config_pending)
		br_transmit_config(p);
	spin_unlock(&p->br->lock);
}

/* 网桥stp相关定时器初始化，主要包括:
 *          "根桥"定时发送配置BPDU信息的hello定时器;
 *          "非根桥"发送TCN-BPDU信息的tcn定时器;
 *          "根桥"发送TC置1的配置BPDU信息的topology_change定时器;
 *          gc定时器
 */
void br_stp_timer_init(struct net_bridge *br)
{
	setup_timer(&br->hello_timer, br_hello_timer_expired,
		      (unsigned long) br);

	setup_timer(&br->tcn_timer, br_tcn_timer_expired,
		      (unsigned long) br);

	setup_timer(&br->topology_change_timer,
		      br_topology_change_timer_expired,
		      (unsigned long) br);

	setup_timer(&br->gc_timer, br_fdb_cleanup, (unsigned long) br);
}

/* 桥端口stp相关定时器初始化，主要包括：
 *          message_age定时器；
 *          用于控制STP端口listening->learning或learning->forwarding转换时延的forward_delay定时器；
 *          hold定时器
 */
void br_stp_port_timer_init(struct net_bridge_port *p)
{
	setup_timer(&p->message_age_timer, br_message_age_timer_expired,
		      (unsigned long) p);

	setup_timer(&p->forward_delay_timer, br_forward_delay_timer_expired,
		      (unsigned long) p);

	setup_timer(&p->hold_timer, br_hold_timer_expired,
		      (unsigned long) p);
}

/* Report ticks left (in USER_HZ) used for API */
unsigned long br_timer_value(const struct timer_list *timer)
{
	return timer_pending(timer)
		? jiffies_delta_to_clock_t(timer->expires - jiffies) : 0;
}
