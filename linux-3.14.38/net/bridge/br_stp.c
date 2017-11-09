/*
 *	Spanning tree protocol; generic parts
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
#include <linux/rculist.h>

#include "br_private.h"
#include "br_private_stp.h"

/* since time values in bpdu are in jiffies and then scaled (1/256)
 * before sending, make sure that is at least one STP tick.
 */
#define MESSAGE_AGE_INCR	((HZ / 256) + 1)

static const char *const br_port_state_names[] = {
	[BR_STATE_DISABLED] = "disabled",
	[BR_STATE_LISTENING] = "listening",
	[BR_STATE_LEARNING] = "learning",
	[BR_STATE_FORWARDING] = "forwarding",
	[BR_STATE_BLOCKING] = "blocking",
};

void br_log_state(const struct net_bridge_port *p)
{
	br_info(p->br, "port %u(%s) entered %s state\n",
		(unsigned int) p->port_no, p->dev->name,
		br_port_state_names[p->state]);
}

/* called under bridge lock 
 * 根据桥端口号获取对应的net_bridge_port结构
 * */
struct net_bridge_port *br_get_port(struct net_bridge *br, u16 port_no)
{
	struct net_bridge_port *p;

	list_for_each_entry_rcu(p, &br->port_list, list) {
		if (p->port_no == port_no)
			return p;
	}

	return NULL;
}

/* called under bridge lock 
 * 判断该桥端口是否成为候选"根端口"
 * @p           需要判断的桥端口
 * @root_port   已经存在的候选"根端口"，如果还未存在，则传入0
 * @返回值      1-成为  0-不成为
 * */
static int br_should_become_root_port(const struct net_bridge_port *p,
				      u16 root_port)
{
	struct net_bridge *br;
	struct net_bridge_port *rp;
	int t;

	br = p->br;
    // 如果该桥端口处于disable状态，或者是"指定端口"，意味着不可能成为根端口，所以直接返回
	if (p->state == BR_STATE_DISABLED ||
	    br_is_designated_port(p))
		return 0;

    // 如果当前网桥ID小于等于该桥端口记录的根桥ID，意味着当前网桥可能是根桥，也就不可能存在根端口，所以直接返回
	if (memcmp(&br->bridge_id, &p->designated_root, 8) <= 0)
		return 0;

    // 程序运行到这里意味着该桥端口满足了成为"根端口"的条件
    // 如果该桥端口是第一个候选"根端口"，则直接返回；如果已经存在候选"根端口"，则继续进行比较，留下一个更好的候选"根端口"
	if (!root_port)
		return 1;

	rp = br_get_port(br, root_port);

    // 1. 比较两个候选"根端口"记录的根桥ID
	t = memcmp(&p->designated_root, &rp->designated_root, 8);
	if (t < 0)
		return 1;
	else if (t > 0)
		return 0;

    // 2. 比较当前网桥分别通过两个候选"根端口"产生的根路径开销
	if (p->designated_cost + p->path_cost <
	    rp->designated_cost + rp->path_cost)
		return 1;
	else if (p->designated_cost + p->path_cost >
		 rp->designated_cost + rp->path_cost)
		return 0;

    // 3. 比较两个候选"根端口"记录的"指定桥ID"
	t = memcmp(&p->designated_bridge, &rp->designated_bridge, 8);
	if (t < 0)
		return 1;
	else if (t > 0)
		return 0;

    // 4. 比较两个候选"根端口"记录的"指定端口ID"
	if (p->designated_port < rp->designated_port)
		return 1;
	else if (p->designated_port > rp->designated_port)
		return 0;

    // 5. 比较两个候选"根端口"的端口ID
	if (p->port_id < rp->port_id)
		return 1;

	return 0;
}

// 将处于blocking状态的候选"根端口"切换到listening状态，然后刷新该端口的forward_delay定时器(前提是配置了有效超时时间)
static void br_root_port_block(const struct net_bridge *br,
			       struct net_bridge_port *p)
{

	br_notice(br, "port %u(%s) tried to become root port (blocked)",
		  (unsigned int) p->port_no, p->dev->name);

    // 从blocking切换到listening状态
	p->state = BR_STATE_LISTENING;
    // 将STP端口状态切换的事件通知用户空间相关的监听者(log && netlink标记第1次)
	br_log_state(p);
	br_ifinfo_notify(RTM_NEWLINK, p);

    // 如果设置了forward_delay，则刷新forward_delay定时器
	if (br->forward_delay > 0)
		mod_timer(&p->forward_delay_timer, jiffies + br->forward_delay);
}

/* called under bridge lock 
 * 刷新该网桥的"根桥"、"根端口"、"根路径开销"
 * */
static void br_root_selection(struct net_bridge *br)
{
	struct net_bridge_port *p;
	u16 root_port = 0;

    // 遍历网桥的所有桥端口，尝试选举出真正的"根端口"
	list_for_each_entry(p, &br->port_list, list) {
        // 略过不会成为候选"根端口"的桥端口
		if (!br_should_become_root_port(p, root_port))
			continue;

        // 如果该端口被block，则先切换到listening状态;其他情况下该端口正式成为候选"根端口"
		if (p->flags & BR_ROOT_BLOCK)
			br_root_port_block(br, p);
		else
			root_port = p->port_no;
	}

    // 程序运行到这里意味着选举完成，root_port为0意味着"根端口"不存在
	br->root_port = root_port;

    // 根据"根端口"是否存在，进而设置该网桥的"根桥"和"根路径开销"
	if (!root_port) {
		br->designated_root = br->bridge_id;
		br->root_path_cost = 0;
	} else {
		p = br_get_port(br, root_port);
		br->designated_root = p->designated_root;
		br->root_path_cost = p->designated_cost + p->path_cost;
	}
}

/* called under bridge lock 
 * 指定的网桥变成根桥后的操作
 * */
void br_become_root_bridge(struct net_bridge *br)
{
	br->max_age = br->bridge_max_age;
	br->hello_time = br->bridge_hello_time;
	br->forward_delay = br->bridge_forward_delay;
    // 执行拓扑变化处理
	br_topology_change_detection(br);
    // 关闭tcn定时器
	del_timer(&br->tcn_timer);

    // 如果该网桥设备处于UP状态，则定时从所有"指定端口"生成并发送配置BPDU
	if (br->dev->flags & IFF_UP) {
		br_config_bpdu_generation(br);
		mod_timer(&br->hello_timer, jiffies + br->hello_time);
	}
}

/* called under bridge lock 
 * 从当前端口发送配置BPDU
 *
 * 备注：显然该端口必然是"指定端口"
 * */
void br_transmit_config(struct net_bridge_port *p)
{
	struct br_config_bpdu bpdu;
	struct net_bridge *br;

    // 确保hold定时器还没有启用
	if (timer_pending(&p->hold_timer)) {
		p->config_pending = 1;
		return;
	}

	br = p->br;

	bpdu.topology_change = br->topology_change;
	bpdu.topology_change_ack = p->topology_change_ack;
	bpdu.root = br->designated_root;
	bpdu.root_path_cost = br->root_path_cost;
	bpdu.bridge_id = br->bridge_id;
	bpdu.port_id = p->port_id;
	if (br_is_root_bridge(br))
		bpdu.message_age = 0;
	else {
		struct net_bridge_port *root
			= br_get_port(br, br->root_port);
		bpdu.message_age = (jiffies - root->designated_age)
			+ MESSAGE_AGE_INCR;
	}
	bpdu.max_age = br->max_age;
	bpdu.hello_time = br->hello_time;
	bpdu.forward_delay = br->forward_delay;

    // 发送前会再次确认消息年龄是否仍旧有效
	if (bpdu.message_age < br->max_age) {
		br_send_config_bpdu(p, &bpdu);
        // 发送之后清除topology_change_ack、config_pending标志，同时开启hold定时器
		p->topology_change_ack = 0;
		p->config_pending = 0;
		mod_timer(&p->hold_timer,
			  round_jiffies(jiffies + BR_HOLD_TIME));
	}
}

/* called under bridge lock 
 * 使用收到的配置BPDU来更新桥端口的配置信息，然后刷新消息年龄定时器
 * */
static void br_record_config_information(struct net_bridge_port *p,
					 const struct br_config_bpdu *bpdu)
{
	p->designated_root = bpdu->root;
	p->designated_cost = bpdu->root_path_cost;
	p->designated_bridge = bpdu->bridge_id;
	p->designated_port = bpdu->port_id;
	p->designated_age = jiffies - bpdu->message_age;

    // 刷新消息年龄定时器，超时时间为剩余的消息年龄
	mod_timer(&p->message_age_timer, jiffies
		  + (bpdu->max_age - bpdu->message_age));
}

/* called under bridge lock 
 * 使用来自根桥的配置BPDU更新该网桥的超时参数
 * @bpdu    必然是来自根桥的
 * */
static void br_record_config_timeout_values(struct net_bridge *br,
					    const struct br_config_bpdu *bpdu)
{
	br->max_age = bpdu->max_age;
	br->hello_time = bpdu->hello_time;
	br->forward_delay = bpdu->forward_delay;
	br->topology_change = bpdu->topology_change;
}

/* called under bridge lock 
 * 发送TCN-BPDU
 * */
void br_transmit_tcn(struct net_bridge *br)
{
	struct net_bridge_port *p;

	p = br_get_port(br, br->root_port);
	if (p)
		br_send_tcn_bpdu(p);
	else
		br_notice(br, "root port %u not found for topology notice\n",
			  br->root_port);
}

/* called under bridge lock 
 * 判断该桥端口是否可以成为所在链路的"指定端口"
 * @返回值  1-可以  0-不可以
 * */
static int br_should_become_designated_port(const struct net_bridge_port *p)
{
	struct net_bridge *br;
	int t;

	br = p->br;
    // 判断顺序依次是：
    // 1. 如果该端口已经是"指定端口"，则直接返回可以
	if (br_is_designated_port(p))
		return 1;

    // 2. 如果该端口的"指定根桥"跟所在网桥的"根桥"不同，则直接返回可以(因为所在网桥已经完成了"根桥"选举，其"根桥"肯定正确)
	if (memcmp(&p->designated_root, &br->designated_root, 8))
		return 1;

    // 3. 比较该网桥和该端口的根路径开销
	if (br->root_path_cost < p->designated_cost)
		return 1;
	else if (br->root_path_cost > p->designated_cost)
		return 0;

    // 4. 比较该网桥ID和该端口的指定桥ID
	t = memcmp(&br->bridge_id, &p->designated_bridge, 8);
	if (t < 0)
		return 1;
	else if (t > 0)
		return 0;

    // 5. 比较该端口ID和指定端口ID
	if (p->port_id < p->designated_port)
		return 1;

	return 0;
}

/* called under bridge lock 
 * 更新该网桥的"指定端口"，也就是选举得到"指定端口"
 *
 * 备注：显然调用本函数时已经完成了"根桥"和"根端口"的选举
 * */
static void br_designated_port_selection(struct net_bridge *br)
{
	struct net_bridge_port *p;

    // 遍历该网桥，将可以成为"指定端口"的桥端口设置成"指定端口"
	list_for_each_entry(p, &br->port_list, list) {
		if (p->state != BR_STATE_DISABLED &&
		    br_should_become_designated_port(p))
			br_become_designated_port(p);

	}
}

/* called under bridge lock 
 * 通过比较普通配置BPDU中携带的STP信息和当前桥端口记录的STP信息，来决定是否需要更新配置
 * 整个比较过程按照
 *      根桥ID -> 根路径开销 -> 指定桥ID -> 指定端口ID
 * 的优先级顺序进行
 *
 * @返回值： 1-需要更新 0-不需要更新
 * */
static int br_supersedes_port_info(const struct net_bridge_port *p,
				   const struct br_config_bpdu *bpdu)
{
	int t;

    // 1.比较BPDU中携带的"根桥"和当前桥端口记录的"根桥"
	t = memcmp(&bpdu->root, &p->designated_root, 8);
	if (t < 0)
		return 1;
	else if (t > 0)
		return 0;

    // 2.比较BPDU中携带的"根路径开销"和当前桥端口记录的"根路径开销"
	if (bpdu->root_path_cost < p->designated_cost)
		return 1;
	else if (bpdu->root_path_cost > p->designated_cost)
		return 0;

    // 3.比较BPDU中携带的"指定桥"和当前桥端口记录的"指定桥"
	t = memcmp(&bpdu->bridge_id, &p->designated_bridge, 8);
	if (t < 0)
		return 1;
	else if (t > 0)
		return 0;

    // 4.理论上BPDU中携带的"指定桥"不可能是当前桥ID，如果真出现，意味着收到了自己发出的BPDU
	if (memcmp(&bpdu->bridge_id, &p->br->bridge_id, 8))
		return 1;

    // 5.比较BPDU中携带的"指定端口"和当前桥端口记录的"指定端口"
	if (bpdu->port_id <= p->designated_port)
		return 1;

	return 0;
}

/* called under bridge lock 
 * 当"根端口"收到设置了TCA的BPDU时执行本函数，具体就是清除topology_change_detected标志，同时关闭tcn定时器，不再向对端发送TCN-BPDU
 * */
static void br_topology_change_acknowledged(struct net_bridge *br)
{
	br->topology_change_detected = 0;
	del_timer(&br->tcn_timer);
}

/* called under bridge lock 
 * 探测到拓扑变化后网桥的处理流程
 * */
void br_topology_change_detection(struct net_bridge *br)
{
	int isroot = br_is_root_bridge(br);

    // 确保开启了内核STP
	if (br->stp_enabled != BR_KERNEL_STP)
		return;

	br_info(br, "topology change detected, %s\n",
		isroot ? "propagating" : "sending tcn bpdu");

	if (isroot) {
        // 对于"根桥"，就会开启topology_change定时器，定时往"指定端口"发送TC置1的配置BPDU信息?
		br->topology_change = 1;
		mod_timer(&br->topology_change_timer, jiffies
			  + br->bridge_forward_delay + br->bridge_max_age);
	} else if (!br->topology_change_detected) {
        // 对于"非根桥"，就会开启tcn定时器，定时往"根端口"发送TCN-BPDU信息
		br_transmit_tcn(br);
		mod_timer(&br->tcn_timer, jiffies + br->bridge_hello_time);
	}

	br->topology_change_detected = 1;
}

/* called under bridge lock 
 * 从所有"指定端口"生成并发送配置BPDU
 *
 * 备注：br不一定是"根桥"
 * */
void br_config_bpdu_generation(struct net_bridge *br)
{
	struct net_bridge_port *p;

	list_for_each_entry(p, &br->port_list, list) {
        // 只有不处于BR_STATE_DISABLED状态的"指定端口"才能发送配置BPDU
		if (p->state != BR_STATE_DISABLED &&
		    br_is_designated_port(p))
			br_transmit_config(p);
	}
}

/* called under bridge lock 
 * 当从"指定端口"收到差的配置BPDU时会调用本函数，向对端回复一个更好的配置BPDU
 * */
static void br_reply(struct net_bridge_port *p)
{
	br_transmit_config(p);
}

/* called under bridge lock 
 * 更新该网桥的STP配置信息
 *
 * 备注：会在以下2中场景下被调用
 *          在收到一个更好的配置BPDU时;
 *          "根端口"丢失了邻居时
 * */
void br_configuration_update(struct net_bridge *br)
{
    // 更新该网桥的"根桥"、"根端口"、"根路径开销" (这里实际就是进行"根桥"和"根端口"的选举)
	br_root_selection(br);
    // 更新该网桥的"指定端口"(这里实际就是进行"指定端口"的选举)
	br_designated_port_selection(br);
}

/* called under bridge lock 
 * 将该桥端口设置为"指定端口"
 * */
void br_become_designated_port(struct net_bridge_port *p)
{
	struct net_bridge *br;

	br = p->br;
	p->designated_root = br->designated_root;
	p->designated_cost = br->root_path_cost;
	p->designated_bridge = br->bridge_id;
	p->designated_port = p->port_id;
}


/* called under bridge lock 
 * 尝试将该端口切换到blocking状态(前提是当前不处于disable或blocking)
 * */
static void br_make_blocking(struct net_bridge_port *p)
{
	if (p->state != BR_STATE_DISABLED &&
	    p->state != BR_STATE_BLOCKING) {
        // 如果当前处于forwarding或learning，需要首先进行拓扑变化处理
		if (p->state == BR_STATE_FORWARDING ||
		    p->state == BR_STATE_LEARNING)
			br_topology_change_detection(p->br);

		p->state = BR_STATE_BLOCKING;
        // 将STP端口状态切换的事件通知用户空间相关的监听者(log && netlink标记第2次)
		br_log_state(p);
		br_ifinfo_notify(RTM_NEWLINK, p);

        // 关闭该端口的forward_delay定时器
		del_timer(&p->forward_delay_timer);
	}
}

/* called under bridge lock 
 * 尝试将该端口从blocking切换到forwarding状态
 * */
static void br_make_forwarding(struct net_bridge_port *p)
{
	struct net_bridge *br = p->br;

    // 忽略对非blocking状态的端口的操作
	if (p->state != BR_STATE_BLOCKING)
		return;

	if (br->stp_enabled == BR_NO_STP || br->forward_delay == 0) {
        /* 如果所在网桥没有开启STP功能，或者开了但是forward_delay为0，则
         *      该端口直接从blocking一步到位进入forwarding状态;
         *      进行拓扑变化处理(前提是网桥开了STP);
         *      关闭该端口的forward_delay定时器
         */
		p->state = BR_STATE_FORWARDING;
		br_topology_change_detection(br);
		del_timer(&p->forward_delay_timer);
	} else if (br->stp_enabled == BR_KERNEL_STP)
        // 如果所在网桥开启了内核STP功能，则该端口进入listening状态
		p->state = BR_STATE_LISTENING;
	else
        // 如果所在网桥开启了用户态STP功能，则该端口进入learning状态
		p->state = BR_STATE_LEARNING;

    // 使能端口的IGMP功能
	br_multicast_enable_port(p);

    // 将STP端口状态切换的事件通知用户空间相关的监听者(log && netlink标记第3次)
	br_log_state(p);
	br_ifinfo_notify(RTM_NEWLINK, p);

    // 只有网桥开启了STP功能并且forward_delay非0的情况下，由于端口无法从blocking一步到位进入forwarding，所以需要开启forward_delay定时器
	if (br->forward_delay != 0)
		mod_timer(&p->forward_delay_timer, jiffies + br->forward_delay);
}

/* called under bridge lock 
 * 执行端口状态更新流程
 * */
void br_port_state_selection(struct net_bridge *br)
{
	struct net_bridge_port *p;
	unsigned int liveports = 0;

	list_for_each_entry(p, &br->port_list, list) {
        // 忽略disable状态的端口
		if (p->state == BR_STATE_DISABLED)
			continue;

		/* Don't change port states if userspace is handling STP 
         * 使能了用户态STP的网桥忽略以下这部分操作
         * */
		if (br->stp_enabled != BR_USER_STP) {
			if (p->port_no == br->root_port) {
                // "根端口"清零config_pending、topology_change_ack标志，然后尝试从blocking切换到forwarding状态
				p->config_pending = 0;
				p->topology_change_ack = 0;
				br_make_forwarding(p);
			} else if (br_is_designated_port(p)) {
                // "指定端口"关闭message_age定时器，然后尝试从blocking切换到forwarding状态
				del_timer(&p->message_age_timer);
				br_make_forwarding(p);
			} else {
                // 其他类型端口清零config_pending、topology_change_ack标志，然后尝试将该端口切换到blocking状态
				p->config_pending = 0;
				p->topology_change_ack = 0;
				br_make_blocking(p);
			}
		}

        // 统计forwarding状态的端口
		if (p->state == BR_STATE_FORWARDING)
			++liveports;
	}

    // 如果该网桥不存在forwarding端口，则通知内核该网桥设备链路断开，否则通知内核该网桥设备链路正常
	if (liveports == 0)
		netif_carrier_off(br->dev);
	else
		netif_carrier_on(br->dev);
}

/* called under bridge lock */
static void br_topology_change_acknowledge(struct net_bridge_port *p)
{
	p->topology_change_ack = 1;
	br_transmit_config(p);
}

/* called under bridge lock 
 * 处理配置BPDU的入口函数
 * */
void br_received_config_bpdu(struct net_bridge_port *p,
			     const struct br_config_bpdu *bpdu)
{
	struct net_bridge *br;
	int was_root;

	br = p->br;
    // 首先判断当前网桥是否是"根桥"
	was_root = br_is_root_bridge(br);

    // 通过比较普通配置BPDU中携带的STP信息和当前桥端口记录的STP信息，来决定是否需要更新配置
	if (br_supersedes_port_info(p, bpdu)) {
        // 如果需要更新配置，则首先更新当前桥端口的STP信息
		br_record_config_information(p, bpdu);
        // 接着更新当前网桥的STP信息
		br_configuration_update(br);
        // 配置更新完毕后，执行端口状态更新流程
		br_port_state_selection(br);

        // 如果当前网桥从"根桥"变成"非根桥"
		if (!br_is_root_bridge(br) && was_root) {
            // 则需要关闭hello定时器
			del_timer(&br->hello_timer);
            // 如果此时网络拓扑正在发生变化，则需要关闭topology_change定时器，同时开启tcn定时器，定时发送TCN-BPDU
			if (br->topology_change_detected) {
				del_timer(&br->topology_change_timer);
				br_transmit_tcn(br);

				mod_timer(&br->tcn_timer,
					  jiffies + br->bridge_hello_time);
			}
		}

        // 如果收到该BPDU的端口就是"根端口"
		if (p->port_no == br->root_port) {
            // 使用来自根桥的配置BPDU更新该网桥的超时参数
			br_record_config_timeout_values(br, bpdu);
            // 生成一个新的配置BPDU并从所有"指定端口"发送
			br_config_bpdu_generation(br);
            // 最后，如果来自根桥的配置BPDU设置了TCA，意味着对端收到了本端口之前发送的TCN-BPDU，可以结束发送TCN-BPDU
			if (bpdu->topology_change_ack)
				br_topology_change_acknowledged(br);
		}
	} else if (br_is_designated_port(p)) {
        // 当从"指定端口"收到差的配置BPDU时，这里会向对端回复一个更好的配置BPDU
		br_reply(p);
	}
}

/* called under bridge lock */
void br_received_tcn_bpdu(struct net_bridge_port *p)
{
	if (br_is_designated_port(p)) {
		br_info(p->br, "port %u(%s) received tcn bpdu\n",
			(unsigned int) p->port_no, p->dev->name);

		br_topology_change_detection(p->br);
		br_topology_change_acknowledge(p);
	}
}

/* Change bridge STP parameter */
int br_set_hello_time(struct net_bridge *br, unsigned long val)
{
	unsigned long t = clock_t_to_jiffies(val);

	if (t < BR_MIN_HELLO_TIME || t > BR_MAX_HELLO_TIME)
		return -ERANGE;

	spin_lock_bh(&br->lock);
	br->bridge_hello_time = t;
	if (br_is_root_bridge(br))
		br->hello_time = br->bridge_hello_time;
	spin_unlock_bh(&br->lock);
	return 0;
}

int br_set_max_age(struct net_bridge *br, unsigned long val)
{
	unsigned long t = clock_t_to_jiffies(val);

	if (t < BR_MIN_MAX_AGE || t > BR_MAX_MAX_AGE)
		return -ERANGE;

	spin_lock_bh(&br->lock);
	br->bridge_max_age = t;
	if (br_is_root_bridge(br))
		br->max_age = br->bridge_max_age;
	spin_unlock_bh(&br->lock);
	return 0;

}

void __br_set_forward_delay(struct net_bridge *br, unsigned long t)
{
	br->bridge_forward_delay = t;
	if (br_is_root_bridge(br))
		br->forward_delay = br->bridge_forward_delay;
}

int br_set_forward_delay(struct net_bridge *br, unsigned long val)
{
	unsigned long t = clock_t_to_jiffies(val);
	int err = -ERANGE;

	spin_lock_bh(&br->lock);
	if (br->stp_enabled != BR_NO_STP &&
	    (t < BR_MIN_FORWARD_DELAY || t > BR_MAX_FORWARD_DELAY))
		goto unlock;

	__br_set_forward_delay(br, t);
	err = 0;

unlock:
	spin_unlock_bh(&br->lock);
	return err;
}
