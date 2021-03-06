/*
 *	watchdog_core.c
 *
 *	(c) Copyright 2008-2011 Alan Cox <alan@lxorguk.ukuu.org.uk>,
 *						All Rights Reserved.
 *
 *	(c) Copyright 2008-2011 Wim Van Sebroeck <wim@iguana.be>.
 *
 *	This source code is part of the generic code that can be used
 *	by all the watchdog timer drivers.
 *
 *	Based on source code of the following authors:
 *	  Matt Domsch <Matt_Domsch@dell.com>,
 *	  Rob Radez <rob@osinvestor.com>,
 *	  Rusty Lynch <rusty@linux.co.intel.com>
 *	  Satyam Sharma <satyam@infradead.org>
 *	  Randy Dunlap <randy.dunlap@oracle.com>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *	Neither Alan Cox, CymruNet Ltd., Wim Van Sebroeck nor Iguana vzw.
 *	admit liability nor provide warranty for any of this software.
 *	This material is provided "AS-IS" and at no charge.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>	/* For EXPORT_SYMBOL/module stuff/... */
#include <linux/types.h>	/* For standard types */
#include <linux/errno.h>	/* For the -ENODEV/... values */
#include <linux/kernel.h>	/* For printk/panic/... */
#include <linux/watchdog.h>	/* For watchdog specific items */
#include <linux/init.h>		/* For __init/__exit/... */
#include <linux/idr.h>		/* For ida_* macros */
#include <linux/err.h>		/* For IS_ERR macros */
#include <linux/of.h>		/* For of_get_timeout_sec */

#include "watchdog_core.h"	/* For watchdog_dev_register/... */

static DEFINE_IDA(watchdog_ida);
static struct class *watchdog_class;    // 指向一个全局的看门狗设备类对象

/*
 * Deferred Registration infrastructure.
 *
 * Sometimes watchdog drivers needs to be loaded as soon as possible,
 * for example when it's impossible to disable it. To do so,
 * raising the initcall level of the watchdog driver is a solution.
 * But in such case, the miscdev is maybe not ready (subsys_initcall), and
 * watchdog_core need miscdev to register the watchdog as a char device.
 *
 * The deferred registration infrastructure offer a way for the watchdog
 * subsystem to register a watchdog properly, even before miscdev is ready.
 */

static DEFINE_MUTEX(wtd_deferred_reg_mutex);
static LIST_HEAD(wtd_deferred_reg_list);    /* 初始化过程中需要延迟注册的看门狗设备链，
                                               记录了那些在看门狗系统初始化前探测到的看门狗设备 */
static bool wtd_deferred_reg_done;          // 标识是否已经完成延迟注册

// 将指定的看门狗设备加入延迟设备链
static int watchdog_deferred_registration_add(struct watchdog_device *wdd)
{
	list_add_tail(&wdd->deferred,
		      &wtd_deferred_reg_list);
	return 0;
}

static void watchdog_deferred_registration_del(struct watchdog_device *wdd)
{
	struct list_head *p, *n;
	struct watchdog_device *wdd_tmp;

	list_for_each_safe(p, n, &wtd_deferred_reg_list) {
		wdd_tmp = list_entry(p, struct watchdog_device,
				     deferred);
		if (wdd_tmp == wdd) {
			list_del(&wdd_tmp->deferred);
			break;
		}
	}
}

// 检查看门狗最小和最大超时值是否合法
static void watchdog_check_min_max_timeout(struct watchdog_device *wdd)
{
	/*
	 * Check that we have valid min and max timeout values, if
	 * not reset them both to 0 (=not used or unknown)
	 */
	if (wdd->min_timeout > wdd->max_timeout) {
		pr_info("Invalid min and max timeout values, resetting to 0!\n");
		wdd->min_timeout = 0;
		wdd->max_timeout = 0;
	}
}

/**
 * watchdog_init_timeout() - initialize the timeout field
 * @timeout_parm: timeout module parameter
 * @dev: Device that stores the timeout-sec property
 *
 * Initialize the timeout field of the watchdog_device struct with either the
 * timeout module parameter (if it is valid value) or the timeout-sec property
 * (only if it is a valid value and the timeout_parm is out of bounds).
 * If none of them are valid then we keep the old value (which should normally
 * be the default timeout value.
 *
 * A zero is returned on success and -EINVAL for failure.
 */
int watchdog_init_timeout(struct watchdog_device *wdd,
				unsigned int timeout_parm, struct device *dev)
{
	unsigned int t = 0;
	int ret = 0;

	watchdog_check_min_max_timeout(wdd);

	/* try to get the timeout module parameter first */
	if (!watchdog_timeout_invalid(wdd, timeout_parm) && timeout_parm) {
		wdd->timeout = timeout_parm;
		return ret;
	}
	if (timeout_parm)
		ret = -EINVAL;

	/* try to get the timeout_sec property */
	if (dev == NULL || dev->of_node == NULL)
		return ret;
	of_property_read_u32(dev->of_node, "timeout-sec", &t);
	if (!watchdog_timeout_invalid(wdd, t) && t)
		wdd->timeout = t;
	else
		ret = -EINVAL;

	return ret;
}
EXPORT_SYMBOL_GPL(watchdog_init_timeout);

// 正常流程注册一个看门狗设备
static int __watchdog_register_device(struct watchdog_device *wdd)
{
	int ret, id = -1, devno;

	if (wdd == NULL || wdd->info == NULL || wdd->ops == NULL)
		return -EINVAL;

	/* Mandatory operations need to be supported */
	if (wdd->ops->start == NULL || wdd->ops->stop == NULL)
		return -EINVAL;

    // 检查看门狗最小和最大超时值是否合法
	watchdog_check_min_max_timeout(wdd);

	/*
	 * Note: now that all watchdog_device data has been verified, we
	 * will not check this anymore in other functions. If data gets
	 * corrupted in a later stage then we expect a kernel panic!
	 */

	mutex_init(&wdd->lock);

	/* Use alias for watchdog id if possible */
	if (wdd->parent) {
		ret = of_alias_get_id(wdd->parent->of_node, "watchdog");
		if (ret >= 0)
			id = ida_simple_get(&watchdog_ida, ret,
					    ret + 1, GFP_KERNEL);
	}

	if (id < 0)
		id = ida_simple_get(&watchdog_ida, 0, MAX_DOGS, GFP_KERNEL);

	if (id < 0)
		return id;
	wdd->id = id;

    //	注册该看门狗设备到内核
	ret = watchdog_dev_register(wdd);
	if (ret) {
		ida_simple_remove(&watchdog_ida, id);
		if (!(id == 0 && ret == -EBUSY))
			return ret;

		/* Retry in case a legacy watchdog module exists */
		id = ida_simple_get(&watchdog_ida, 1, MAX_DOGS, GFP_KERNEL);
		if (id < 0)
			return id;
		wdd->id = id;

		ret = watchdog_dev_register(wdd);
		if (ret) {
			ida_simple_remove(&watchdog_ida, id);
			return ret;
		}
	}

    // 为该看门狗设备创建一个对应的device对象
	devno = wdd->cdev.dev;
	wdd->dev = device_create(watchdog_class, wdd->parent, devno,
					NULL, "watchdog%d", wdd->id);
	if (IS_ERR(wdd->dev)) {
		watchdog_dev_unregister(wdd);
		ida_simple_remove(&watchdog_ida, id);
		ret = PTR_ERR(wdd->dev);
		return ret;
	}

	return 0;
}

/**
 * watchdog_register_device() - register a watchdog device
 * 注册一个指定的看门狗设备
 * @wdd: watchdog device
 *
 * Register a watchdog device with the kernel so that the
 * watchdog timer can be accessed from userspace.
 *
 * A zero is returned on success and a negative errno code for
 * failure.
 */

int watchdog_register_device(struct watchdog_device *wdd)
{
	int ret;

	mutex_lock(&wtd_deferred_reg_mutex);
    // 如果已经完成延迟注册，则按正常流程注册该看门狗设备；否则将该看门狗设备加入延迟设备链
	if (wtd_deferred_reg_done)
		ret = __watchdog_register_device(wdd);
	else
		ret = watchdog_deferred_registration_add(wdd);
	mutex_unlock(&wtd_deferred_reg_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(watchdog_register_device);

static void __watchdog_unregister_device(struct watchdog_device *wdd)
{
	int ret;
	int devno;

	if (wdd == NULL)
		return;

	devno = wdd->cdev.dev;
	ret = watchdog_dev_unregister(wdd);
	if (ret)
		pr_err("error unregistering /dev/watchdog (err=%d)\n", ret);
	device_destroy(watchdog_class, devno);
	ida_simple_remove(&watchdog_ida, wdd->id);
	wdd->dev = NULL;
}

/**
 * watchdog_unregister_device() - unregister a watchdog device
 * @wdd: watchdog device to unregister
 *
 * Unregister a watchdog device that was previously successfully
 * registered with watchdog_register_device().
 */

void watchdog_unregister_device(struct watchdog_device *wdd)
{
	mutex_lock(&wtd_deferred_reg_mutex);
	if (wtd_deferred_reg_done)
		__watchdog_unregister_device(wdd);
	else
		watchdog_deferred_registration_del(wdd);
	mutex_unlock(&wtd_deferred_reg_mutex);
}

EXPORT_SYMBOL_GPL(watchdog_unregister_device);

// 注册挂在wtd_deferred_reg_list延迟链表上的看门狗设备
static int __init watchdog_deferred_registration(void)
{
	mutex_lock(&wtd_deferred_reg_mutex);
	wtd_deferred_reg_done = true;
	while (!list_empty(&wtd_deferred_reg_list)) {
		struct watchdog_device *wdd;

		wdd = list_first_entry(&wtd_deferred_reg_list,
				       struct watchdog_device, deferred);
		list_del(&wdd->deferred);
		__watchdog_register_device(wdd);
	}
	mutex_unlock(&wtd_deferred_reg_mutex);
	return 0;
}

// 看门狗子系统初始化
static int __init watchdog_init(void)
{
	int err;

    // 创建一个全局的看门狗设备类对象
	watchdog_class = class_create(THIS_MODULE, "watchdog");
	if (IS_ERR(watchdog_class)) {
		pr_err("couldn't create class\n");
		return PTR_ERR(watchdog_class);
	}

    // 为看门狗类设备申请一段设备号
	err = watchdog_dev_init();
	if (err < 0) {
		class_destroy(watchdog_class);
		return err;
	}

    // 程序运行到这里意味着看门狗子系统已经完成初始化
    // 注册挂在wtd_deferred_reg_list延迟链表上的看门狗设备
	watchdog_deferred_registration();
	return 0;
}

static void __exit watchdog_exit(void)
{
	watchdog_dev_exit();
	class_destroy(watchdog_class);
	ida_destroy(&watchdog_ida);
}

subsys_initcall_sync(watchdog_init);
module_exit(watchdog_exit);

MODULE_AUTHOR("Alan Cox <alan@lxorguk.ukuu.org.uk>");
MODULE_AUTHOR("Wim Van Sebroeck <wim@iguana.be>");
MODULE_DESCRIPTION("WatchDog Timer Driver Core");
MODULE_LICENSE("GPL");
