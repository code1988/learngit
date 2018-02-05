/*
 * Linux network device link state notification
 * netdevlink状态通知操作接口
 *
 * Author:
 *     Stefan Rompf <sux@loplof.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/if.h>
#include <net/sock.h>
#include <net/pkt_sched.h>
#include <linux/rtnetlink.h>
#include <linux/jiffies.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/bitops.h>
#include <asm/types.h>


enum lw_bits {
	LW_URGENT = 0,  // 标识当前正在调度的事件为紧急事件
};

static unsigned long linkwatch_flags;       // 用于记录链路事件通知链当前正在调度的事件的一些特性，3.14.38版本就只有LW_URGENT
static unsigned long linkwatch_nextevent;   // 记录了正常情况下下一次执行linkwatch_event的时间点

static void linkwatch_event(struct work_struct *dummy);
static DECLARE_DELAYED_WORK(linkwatch_work, linkwatch_event);   // 创建并初始化一个延迟执行的工作单元linkwatch_work

static LIST_HEAD(lweventlist);              // 创建并初始化一个链路事件通知链表头
static DEFINE_SPINLOCK(lweventlist_lock);   // 定义了一个自旋锁，用于维护lweventlist的操作

static unsigned char default_operstate(const struct net_device *dev)
{
	if (!netif_carrier_ok(dev))
		return (dev->ifindex != dev->iflink ?
			IF_OPER_LOWERLAYERDOWN : IF_OPER_DOWN);

	if (netif_dormant(dev))
		return IF_OPER_DORMANT;

	return IF_OPER_UP;
}


static void rfc2863_policy(struct net_device *dev)
{
	unsigned char operstate = default_operstate(dev);

	if (operstate == dev->operstate)
		return;

	write_lock_bh(&dev_base_lock);

	switch(dev->link_mode) {
	case IF_LINK_MODE_DORMANT:
		if (operstate == IF_OPER_UP)
			operstate = IF_OPER_DORMANT;
		break;

	case IF_LINK_MODE_DEFAULT:
	default:
		break;
	}

	dev->operstate = operstate;

	write_unlock_bh(&dev_base_lock);
}


void linkwatch_init_dev(struct net_device *dev)
{
	/* Handle pre-registration link state changes */
	if (!netif_carrier_ok(dev) || netif_dormant(dev))
		rfc2863_policy(dev);
}

// 判断该netdev是否有紧急链路事件
static bool linkwatch_urgent_event(struct net_device *dev)
{
    // 如果该netdev处于未启用状态，则肯定没有紧急事件
	if (!netif_running(dev))
		return false;

    // 如果该netdev的ifindex和iflink字段不等，意味着该netdev是虚拟设备，这里认为肯定有紧急事件
	if (dev->ifindex != dev->iflink)
		return true;

    // 如果该设备设置了IFF_TEAM_PORT标志，这里认为肯定有紧急事件
	if (dev->priv_flags & IFF_TEAM_PORT)
		return true;

    // 如果该netdev有载波，并且其tx qdisc发生了变化，则认为有紧急事件
	return netif_carrier_ok(dev) &&	qdisc_tx_changing(dev);
}

// 将新的netdev添加到链路事件通知链尾部
static void linkwatch_add_event(struct net_device *dev)
{
	unsigned long flags;

    // 对lweventlist操作需要上锁
	spin_lock_irqsave(&lweventlist_lock, flags);
    // 如果该netdev尚未被插入链表则将其插入lweventlist
	if (list_empty(&dev->link_watch_list)) {
		list_add_tail(&dev->link_watch_list, &lweventlist);
		dev_hold(dev);
	}
	spin_unlock_irqrestore(&lweventlist_lock, flags);
}

/* 链路通知链执行调度操作
 * @urgent  标识新的调度是否加入了紧急事件
 */
static void linkwatch_schedule_work(int urgent)
{
	unsigned long delay = linkwatch_nextevent - jiffies;

    // 如果当前正在调度原有的紧急事件，则直接返回(意味着新的调度操作即便存在紧急事件也会因此退化为普通事件)
	if (test_bit(LW_URGENT, &linkwatch_flags))
		return;

	/* Minimise down-time: drop delay for up event. 
     * 如果新的调度存在紧急事件，则设置LW_URGENT标志，以防被后面的紧急事件抢占
     * */
	if (urgent) {
        // 这里调用原子操作test_and_set_bit，从而避免了潜在的竞争
        // 同时还要注意到，如果已经存在了调度中的紧急事件，则直接返回(意味着新的调度操作即便存在紧急事件也会因此退化为普通事件)
		if (test_and_set_bit(LW_URGENT, &linkwatch_flags))
			return;

        // 延时清0
		delay = 0;
	}

	/* If we wrap around we'll delay it by at most HZ. 
     * 这里确保调度延时不会超过1s
     * */
	if (delay > HZ)
		delay = 0;

	/*
	 * If urgent, schedule immediate execution; otherwise, don't
	 * override the existing timer.
     * 如果该调度是紧急事件，则立即排入缺省工作队列system_wq;否则延时delay后再排入system_wq
	 */
	if (test_bit(LW_URGENT, &linkwatch_flags))
		mod_delayed_work(system_wq, &linkwatch_work, 0);
	else
		schedule_delayed_work(&linkwatch_work, delay);
}

// 处理指定设备上的链路事件
static void linkwatch_do_dev(struct net_device *dev)
{
	/*
	 * Make sure the above read is complete since it can be
	 * rewritten as soon as we clear the bit below.
	 */
	smp_mb__before_clear_bit();

	/* We are about to handle this device,
	 * so new events can be accepted
     * 该netdev的链路事件已经在处理中，所以可以清除__LINK_STATE_LINKWATCH_PENDING标志了
	 */
	clear_bit(__LINK_STATE_LINKWATCH_PENDING, &dev->state);

	rfc2863_policy(dev);
    // 只有处于启用状态的netdev才需要处理链路事件
	if (dev->flags & IFF_UP) {
        // 根据链路是否有载波，从而决定是进行激活还是非激活处理
		if (netif_carrier_ok(dev))
			dev_activate(dev);
		else
			dev_deactivate(dev);

        // 通知内核其他模块以及用户层，指定netdev的链路状态已经发生改变
		netdev_state_change(dev);
	}
	dev_put(dev);
}

/* 执行工作单元linkwatch_work关联的任务
 * @urgent_only     标识本次处理的是否是紧急事件，1表示有紧急事件，0表示正常调度
 */
static void __linkwatch_run_queue(int urgent_only)
{
	struct net_device *dev;
	LIST_HEAD(wrk);         // 创建并初始化一个临时链表头

	/*
	 * Limit the number of linkwatch events to one
	 * per second so that a runaway driver does not
	 * cause a storm of messages on the netlink
	 * socket.  This limit does not apply to up events
	 * while the device qdisc is down.
     * 这里将正常调度的时间间隔设为1s，以避免失控驱动可能引发的rtnetlink风暴
	 */
	if (!urgent_only)
		linkwatch_nextevent = jiffies + HZ;
	/* Limit wrap-around effect on delay. */
	else if (time_after(linkwatch_nextevent, jiffies + HZ))
		linkwatch_nextevent = jiffies;

    // 清除LW_URGENT标识
	clear_bit(LW_URGENT, &linkwatch_flags);

	spin_lock_irq(&lweventlist_lock);
    // 将链路事件通知链转储到临时链表wrk中，然后复位通知链
	list_splice_init(&lweventlist, &wrk);

    // 遍历转储出来的通知链
	while (!list_empty(&wrk)) {
        // 获取wrk链表中首节点所属的netdev
		dev = list_first_entry(&wrk, struct net_device, link_watch_list);
        // 将该首节点从wrk链表中移除
		list_del_init(&dev->link_watch_list);

        // 如果是因紧急事件触发的调度行为，这里就在寻找该紧急事件源，对于不符的节点还是会被回插到lweventlist尾部
		if (urgent_only && !linkwatch_urgent_event(dev)) {
			list_add_tail(&dev->link_watch_list, &lweventlist);
			continue;
		}

        // 程序运行到这里意味着要么是紧急事件源，要么是正常调度情况
		spin_unlock_irq(&lweventlist_lock);
        // 处理该netdev上的链路事件
		linkwatch_do_dev(dev);
		spin_lock_irq(&lweventlist_lock);
	}

    // 如果遍历完转储出来的通知链后，lweventlist还是非空，意味着通知链上存在非紧急事件，所以再次调度通知链
	if (!list_empty(&lweventlist))
		linkwatch_schedule_work(0);
	spin_unlock_irq(&lweventlist_lock);
}

void linkwatch_forget_dev(struct net_device *dev)
{
	unsigned long flags;
	int clean = 0;

	spin_lock_irqsave(&lweventlist_lock, flags);
	if (!list_empty(&dev->link_watch_list)) {
		list_del_init(&dev->link_watch_list);
		clean = 1;
	}
	spin_unlock_irqrestore(&lweventlist_lock, flags);
	if (clean)
		linkwatch_do_dev(dev);
}


/* Must be called with the rtnl semaphore held */
void linkwatch_run_queue(void)
{
	__linkwatch_run_queue(0);
}

// 工作单元linkwatch_work关联的工作处理函数
static void linkwatch_event(struct work_struct *dummy)
{
    // 由于需要调用到内核rtnetlink模块，所以首先进行上锁
	rtnl_lock();
    // 通过比较linkwatch_nextevent和当前时间来确定本地调度是否是因紧急事件引起
	__linkwatch_run_queue(time_after(linkwatch_nextevent, jiffies));
	rtnl_unlock();
}

// 将指定netdev加入链路事件通知链
void linkwatch_fire_event(struct net_device *dev)
{
    // 首先判断该netdev是否有紧急链路事件
	bool urgent = linkwatch_urgent_event(dev);

    // 置位该netdev的__LINK_STATE_LINKWATCH_PENDING标志，并判断原先的状态
	if (!test_and_set_bit(__LINK_STATE_LINKWATCH_PENDING, &dev->state)) {
        // 对于原先该标志没有置位的情况，则将该设备添加进lweventlist通知链(尾部)
		linkwatch_add_event(dev);
	} else if (!urgent)
        // 对于原先已经置位的情况，意味着通知链中已经添加了该netdev，如果该netdev没有紧急链路事件，则直接返回，等待正常的调度即可
		return;

    // 执行通知链调度
	linkwatch_schedule_work(urgent);
}
EXPORT_SYMBOL(linkwatch_fire_event);
