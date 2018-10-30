/*
 *	watchdog_dev.c
 *
 *	(c) Copyright 2008-2011 Alan Cox <alan@lxorguk.ukuu.org.uk>,
 *						All Rights Reserved.
 *
 *	(c) Copyright 2008-2011 Wim Van Sebroeck <wim@iguana.be>.
 *
 *
 *	This source code is part of the generic code that can be used
 *	by all the watchdog timer drivers.
 *
 *	This part of the generic code takes care of the following
 *	misc device: /dev/watchdog.
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

#include <linux/module.h>	/* For module stuff/... */
#include <linux/types.h>	/* For standard types (like size_t) */
#include <linux/errno.h>	/* For the -ENODEV/... values */
#include <linux/kernel.h>	/* For printk/panic/... */
#include <linux/fs.h>		/* For file operations */
#include <linux/watchdog.h>	/* For watchdog specific items */
#include <linux/miscdevice.h>	/* For handling misc devices */
#include <linux/init.h>		/* For __init/__exit/... */
#include <linux/uaccess.h>	/* For copy_to_user/put_user/... */

#include "watchdog_core.h"

/* the dev_t structure to store the dynamically allocated watchdog devices 
 * 定义了全局的动态分配的一段看门狗起始设备号
 * */
static dev_t watchdog_devt;     
/* the watchdog device behind /dev/watchdog 
 * 旧的看门狗实现只会创建一个固定的杂项看门狗设备/dev/watchdog
 * */
static struct watchdog_device *old_wdd;

/*
 *	watchdog_ping: ping the watchdog.
 *	执行一次喂狗操作
 *	@wdd: the watchdog device to ping
 *
 *	If the watchdog has no own ping operation then it needs to be
 *	restarted via the start operation. This wrapper function does
 *	exactly that.
 *	We only ping when the watchdog device is running.
 */

static int watchdog_ping(struct watchdog_device *wdd)
{
	int err = 0;

	mutex_lock(&wdd->lock);

    // 确保该看门狗没有被注销
	if (test_bit(WDOG_UNREGISTERED, &wdd->status)) {
		err = -ENODEV;
		goto out_ping;
	}

    // 确保该看门狗处于运行状态
	if (!watchdog_active(wdd))
		goto out_ping;

    // 如果该看门狗设备注册了ping方法，则执行该方法完成喂狗；否则通过重启看门狗完成喂狗
	if (wdd->ops->ping)
		err = wdd->ops->ping(wdd);	/* ping the watchdog */
	else
		err = wdd->ops->start(wdd);	/* restart watchdog */

out_ping:
	mutex_unlock(&wdd->lock);
	return err;
}

/*
 *	watchdog_start: wrapper to start the watchdog.
 *	启动指定看门狗设备
 *	@wdd: the watchdog device to start
 *
 *	Start the watchdog if it is not active and mark it active.
 *	This function returns zero on success or a negative errno code for
 *	failure.
 */

static int watchdog_start(struct watchdog_device *wdd)
{
	int err = 0;

	mutex_lock(&wdd->lock);

    // 确保该看门狗没有被注销
	if (test_bit(WDOG_UNREGISTERED, &wdd->status)) {
		err = -ENODEV;
		goto out_start;
	}

    // 如果该看门狗已经处于运行状态，则直接返回
	if (watchdog_active(wdd))
		goto out_start;

    // 调用该看门狗设备的start方法来真正完成启动
	err = wdd->ops->start(wdd);
	if (err == 0)
		set_bit(WDOG_ACTIVE, &wdd->status);

out_start:
	mutex_unlock(&wdd->lock);
	return err;
}

/*
 *	watchdog_stop: wrapper to stop the watchdog.
 *	停止指定看门狗设备
 *	@wdd: the watchdog device to stop
 *
 *	Stop the watchdog if it is still active and unmark it active.
 *	This function returns zero on success or a negative errno code for
 *	failure.
 *	If the 'nowayout' feature was set, the watchdog cannot be stopped.
 */

static int watchdog_stop(struct watchdog_device *wdd)
{
	int err = 0;

	mutex_lock(&wdd->lock);

    // 确保该看门狗没有被注销
	if (test_bit(WDOG_UNREGISTERED, &wdd->status)) {
		err = -ENODEV;
		goto out_stop;
	}

    // 确保该看门狗处于运行状态
	if (!watchdog_active(wdd))
		goto out_stop;

    // 确保该看门狗没有处于无路可走的状态
	if (test_bit(WDOG_NO_WAY_OUT, &wdd->status)) {
		dev_info(wdd->dev, "nowayout prevents watchdog being stopped!\n");
		err = -EBUSY;
		goto out_stop;
	}

    // 调用该看门狗设备的stop方法来真正完成停止
	err = wdd->ops->stop(wdd);
	if (err == 0)
		clear_bit(WDOG_ACTIVE, &wdd->status);

out_stop:
	mutex_unlock(&wdd->lock);
	return err;
}

/*
 *	watchdog_get_status: wrapper to get the watchdog status
 *	获取指定看门狗当前状态集合
 *	@wdd: the watchdog device to get the status from
 *	@status: the status of the watchdog device      用于存放该看门狗的当前状态
 *
 *	Get the watchdog's status flags.
 */

static int watchdog_get_status(struct watchdog_device *wdd,
							unsigned int *status)
{
	int err = 0;

	*status = 0;
	if (!wdd->ops->status)
		return -EOPNOTSUPP;

	mutex_lock(&wdd->lock);

    // 确保该看门狗没有被注销
	if (test_bit(WDOG_UNREGISTERED, &wdd->status)) {
		err = -ENODEV;
		goto out_status;
	}

    // 调用该看门狗设备的status方法来真正获取状态
	*status = wdd->ops->status(wdd);

out_status:
	mutex_unlock(&wdd->lock);
	return err;
}

/*
 *	watchdog_set_timeout: set the watchdog timer timeout
 *	设置指定看门狗的定时器超时值
 *	@wdd: the watchdog device to set the timeout for
 *	@timeout: timeout to set in seconds
 */

static int watchdog_set_timeout(struct watchdog_device *wdd,
							unsigned int timeout)
{
	int err;

	if (!wdd->ops->set_timeout || !(wdd->info->options & WDIOF_SETTIMEOUT))
		return -EOPNOTSUPP;

    // 确保要设置的超时值合法
	if (watchdog_timeout_invalid(wdd, timeout))
		return -EINVAL;

	mutex_lock(&wdd->lock);

    // 确保该看门狗没有被注销
	if (test_bit(WDOG_UNREGISTERED, &wdd->status)) {
		err = -ENODEV;
		goto out_timeout;
	}

    // 调用该看门狗设备的set_timeout方法来真正完成超时值设置
	err = wdd->ops->set_timeout(wdd, timeout);

out_timeout:
	mutex_unlock(&wdd->lock);
	return err;
}

/*
 *	watchdog_get_timeleft: wrapper to get the time left before a reboot
 *	获取指定看门狗距离超时的剩余时间
 *	@wdd: the watchdog device to get the remaining time from
 *	@timeleft: the time that's left
 *
 *	Get the time before a watchdog will reboot (if not pinged).
 */

static int watchdog_get_timeleft(struct watchdog_device *wdd,
							unsigned int *timeleft)
{
	int err = 0;

	*timeleft = 0;
	if (!wdd->ops->get_timeleft)
		return -EOPNOTSUPP;

	mutex_lock(&wdd->lock);

    // 确保该看门狗没有被注销
	if (test_bit(WDOG_UNREGISTERED, &wdd->status)) {
		err = -ENODEV;
		goto out_timeleft;
	}

    // 调用该看门狗设备的get_timeout方法来真正完成剩余时间获取
	*timeleft = wdd->ops->get_timeleft(wdd);

out_timeleft:
	mutex_unlock(&wdd->lock);
	return err;
}

/*
 *	watchdog_ioctl_op: call the watchdog drivers ioctl op if defined
 *	@wdd: the watchdog device to do the ioctl on
 *	@cmd: watchdog command
 *	@arg: argument pointer
 */

static int watchdog_ioctl_op(struct watchdog_device *wdd, unsigned int cmd,
							unsigned long arg)
{
	int err;

	if (!wdd->ops->ioctl)
		return -ENOIOCTLCMD;

	mutex_lock(&wdd->lock);

	if (test_bit(WDOG_UNREGISTERED, &wdd->status)) {
		err = -ENODEV;
		goto out_ioctl;
	}

	err = wdd->ops->ioctl(wdd, cmd, arg);

out_ioctl:
	mutex_unlock(&wdd->lock);
	return err;
}

/*
 *	watchdog_write: writes to the watchdog.
 *	看门狗设备文件的write方法，实质就是执行一次喂狗操作
 *	@file: file from VFS
 *	@data: user address of data
 *	@len: length of data
 *	@ppos: pointer to the file offset
 *
 *	A write to a watchdog device is defined as a keepalive ping.
 *	Writing the magic 'V' sequence allows the next close to turn
 *	off the watchdog (if 'nowayout' is not set).
 *	对看门狗设备文件的写入操作被认为是一次保活操作
 */

static ssize_t watchdog_write(struct file *file, const char __user *data,
						size_t len, loff_t *ppos)
{
	struct watchdog_device *wdd = file->private_data;
	size_t i;
	char c;
	int err;

	if (len == 0)
		return 0;

	/*
	 * Note: just in case someone wrote the magic character
	 * five months ago...
	 */
	clear_bit(WDOG_ALLOW_RELEASE, &wdd->status);

	/* scan to see whether or not we got the magic character 
     * 如果写入的数据中存在'V'字符，则该看门狗设备会被设置WDOG_ALLOW_RELEASE标志
     * */
	for (i = 0; i != len; i++) {
		if (get_user(c, data + i))
			return -EFAULT;
		if (c == 'V')
			set_bit(WDOG_ALLOW_RELEASE, &wdd->status);
	}

	/* someone wrote to us, so we send the watchdog a keepalive ping 
     * 执行一次保活操作
     * */
	err = watchdog_ping(wdd);
	if (err < 0)
		return err;

	return len;
}

/*
 *	watchdog_ioctl: handle the different ioctl's for the watchdog device.
 *	看门狗设备的ioctl接口
 *	@file: file handle to the device
 *	@cmd: watchdog command
 *	@arg: argument pointer
 *
 *	The watchdog API defines a common set of functions for all watchdogs
 *	according to their available features.
 */

static long watchdog_ioctl(struct file *file, unsigned int cmd,
							unsigned long arg)
{
	struct watchdog_device *wdd = file->private_data;
	void __user *argp = (void __user *)arg;
	int __user *p = argp;
	unsigned int val;
	int err;

	err = watchdog_ioctl_op(wdd, cmd, arg);
	if (err != -ENOIOCTLCMD)
		return err;

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		return copy_to_user(argp, wdd->info,
			sizeof(struct watchdog_info)) ? -EFAULT : 0;
	case WDIOC_GETSTATUS:
		err = watchdog_get_status(wdd, &val);
		if (err == -ENODEV)
			return err;
		return put_user(val, p);
	case WDIOC_GETBOOTSTATUS:
		return put_user(wdd->bootstatus, p);
	case WDIOC_SETOPTIONS:
		if (get_user(val, p))
			return -EFAULT;
		if (val & WDIOS_DISABLECARD) {
			err = watchdog_stop(wdd);
			if (err < 0)
				return err;
		}
		if (val & WDIOS_ENABLECARD) {
			err = watchdog_start(wdd);
			if (err < 0)
				return err;
		}
		return 0;
	case WDIOC_KEEPALIVE:
		if (!(wdd->info->options & WDIOF_KEEPALIVEPING))
			return -EOPNOTSUPP;
		return watchdog_ping(wdd);
	case WDIOC_SETTIMEOUT:
		if (get_user(val, p))
			return -EFAULT;
		err = watchdog_set_timeout(wdd, val);
		if (err < 0)
			return err;
		/* If the watchdog is active then we send a keepalive ping
		 * to make sure that the watchdog keep's running (and if
		 * possible that it takes the new timeout) */
		err = watchdog_ping(wdd);
		if (err < 0)
			return err;
		/* Fall */
	case WDIOC_GETTIMEOUT:
		/* timeout == 0 means that we don't know the timeout */
		if (wdd->timeout == 0)
			return -EOPNOTSUPP;
		return put_user(wdd->timeout, p);
	case WDIOC_GETTIMELEFT:
		err = watchdog_get_timeleft(wdd, &val);
		if (err)
			return err;
		return put_user(val, p);
	default:
		return -ENOTTY;
	}
}

/*
 *	watchdog_open: open the /dev/watchdog* devices.
 *	看门狗设备文件的open方法，实质就是启动对应的看门狗
 *	@inode: inode of device
 *	@file: file handle to device
 *
 *	When the /dev/watchdog* device gets opened, we start the watchdog.
 *	Watch out: the /dev/watchdog device is single open, so we make sure
 *	it can only be opened once.
 */

static int watchdog_open(struct inode *inode, struct file *file)
{
	int err = -EBUSY;
	struct watchdog_device *wdd;

	/* Get the corresponding watchdog device 
     * 根据设备号判断打开的是旧式看门狗还是新式看门狗
     * */
	if (imajor(inode) == MISC_MAJOR)
		wdd = old_wdd;
	else
		wdd = container_of(inode->i_cdev, struct watchdog_device, cdev);

	/* the watchdog is single open! 
     * 确保该看门狗设备文件只会被同时打开一次
     * */
	if (test_and_set_bit(WDOG_DEV_OPEN, &wdd->status))
		return -EBUSY;

	/*
	 * If the /dev/watchdog device is open, we don't want the module
	 * to be unloaded.
	 */
	if (!try_module_get(wdd->ops->owner))
		goto out;

    // 启动该看门狗设备
	err = watchdog_start(wdd);
	if (err < 0)
		goto out_mod;

    // 将该打开的看门狗文件对象和看门狗设备对象绑定
	file->private_data = wdd;

	if (wdd->ops->ref)
		wdd->ops->ref(wdd);

	/* dev/watchdog is a virtual (and thus non-seekable) filesystem 
     * 将该看门狗文件设置为不允许seek操作
     * */
	return nonseekable_open(inode, file);

out_mod:
	module_put(wdd->ops->owner);
out:
	clear_bit(WDOG_DEV_OPEN, &wdd->status);
	return err;
}

/*
 *	watchdog_release: release the watchdog device.
 *	看门狗设备文件的release(即close)方法
 *	@inode: inode of device
 *	@file: file handle to device
 *
 *	This is the code for when /dev/watchdog gets closed. We will only
 *	stop the watchdog when we have received the magic char (and nowayout
 *	was not set), else the watchdog will keep running.
 */

static int watchdog_release(struct inode *inode, struct file *file)
{
	struct watchdog_device *wdd = file->private_data;
	int err = -EBUSY;

	/*
	 * We only stop the watchdog if we received the magic character
	 * or if WDIOF_MAGICCLOSE is not set. If nowayout was set then
	 * watchdog_stop will fail.
     * 已经启动的看门狗只有在两种情况下才会停止：
     *      [1]. 看门狗设置WDOG_ALLOW_RELEASE标志   -> 看门狗文件close
     *      [2]. 看门狗清除WDIOF_MAGICCLOSE标志     -> 看门狗文件close
	 */
	if (!test_bit(WDOG_ACTIVE, &wdd->status))
		err = 0;
	else if (test_and_clear_bit(WDOG_ALLOW_RELEASE, &wdd->status) ||
		 !(wdd->info->options & WDIOF_MAGICCLOSE))
		err = watchdog_stop(wdd);

	/* If the watchdog was not stopped, send a keepalive ping 
     * 如果看门狗停止失败，则需要进行一次喂狗
     * */
	if (err < 0) {
		mutex_lock(&wdd->lock);
		if (!test_bit(WDOG_UNREGISTERED, &wdd->status))
			dev_crit(wdd->dev, "watchdog did not stop!\n");
		mutex_unlock(&wdd->lock);
		watchdog_ping(wdd);
	}

	/* Allow the owner module to be unloaded again */
	module_put(wdd->ops->owner);

	/* make sure that /dev/watchdog can be re-opened 
     * 清除WDOG_DEV_OPEN标志，使该看门狗文件可以被重新打开
     * */
	clear_bit(WDOG_DEV_OPEN, &wdd->status);

	/* Note wdd may be gone after this, do not use after this! */
	if (wdd->ops->unref)
		wdd->ops->unref(wdd);

	return 0;
}

// 定义了看门狗设备提供给文件系统的接口集合(看门狗杂项设备和字符设备共用这套接口)
static const struct file_operations watchdog_fops = {
	.owner		= THIS_MODULE,
	.write		= watchdog_write,
	.unlocked_ioctl	= watchdog_ioctl,
	.open		= watchdog_open,
	.release	= watchdog_release,
};

// 定义了一个看门狗杂项设备
static struct miscdevice watchdog_miscdev = {
	.minor		= WATCHDOG_MINOR,
	.name		= "watchdog",
	.fops		= &watchdog_fops,
};

/*
 *	watchdog_dev_register: register a watchdog device
 *	注册一个看门狗设备到内核
 *	@wdd: watchdog device
 *
 *	Register a watchdog device including handling the legacy
 *	/dev/watchdog node. /dev/watchdog is actually a miscdevice and
 *	thus we set it up like that.
 */

int watchdog_dev_register(struct watchdog_device *wdd)
{
	int err, devno;

    /* 如果看门狗设备的id为0,则额外注册一个看门狗杂项设备到内核
     * 备注：注册这个看门狗杂项设备(/dev/watchdog)实际是为了向前兼容
     */
	if (wdd->id == 0) {
		old_wdd = wdd;
		watchdog_miscdev.parent = wdd->parent;
		err = misc_register(&watchdog_miscdev);
		if (err != 0) {
			pr_err("%s: cannot register miscdev on minor=%d (err=%d).\n",
				wdd->info->identity, WATCHDOG_MINOR, err);
			if (err == -EBUSY)
				pr_err("%s: a legacy watchdog module is probably present.\n",
					wdd->info->identity);
			old_wdd = NULL;
			return err;
		}
	}

	/* Fill in the data structures 
     * 初始化看门狗字符设备
     * */
	devno = MKDEV(MAJOR(watchdog_devt), wdd->id);
	cdev_init(&wdd->cdev, &watchdog_fops);
	wdd->cdev.owner = wdd->ops->owner;

	/* Add the device 
     * 将看门狗字符设备设备注册到内核
     * */
	err  = cdev_add(&wdd->cdev, devno, 1);
	if (err) {
		pr_err("watchdog%d unable to add device %d:%d\n",
			wdd->id,  MAJOR(watchdog_devt), wdd->id);
		if (wdd->id == 0) {
			misc_deregister(&watchdog_miscdev);
			old_wdd = NULL;
		}
	}
	return err;
}

/*
 *	watchdog_dev_unregister: unregister a watchdog device
 *	@watchdog: watchdog device
 *
 *	Unregister the watchdog and if needed the legacy /dev/watchdog device.
 */

int watchdog_dev_unregister(struct watchdog_device *wdd)
{
	mutex_lock(&wdd->lock);
	set_bit(WDOG_UNREGISTERED, &wdd->status);
	mutex_unlock(&wdd->lock);

	cdev_del(&wdd->cdev);
	if (wdd->id == 0) {
		misc_deregister(&watchdog_miscdev);
		old_wdd = NULL;
	}
	return 0;
}

/*
 *	watchdog_dev_init: init dev part of watchdog core
 *	为看门狗类设备注册一段设备号
 *
 *	Allocate a range of chardev nodes to use for watchdog devices
 */

int __init watchdog_dev_init(void)
{
	int err = alloc_chrdev_region(&watchdog_devt, 0, MAX_DOGS, "watchdog");
	if (err < 0)
		pr_err("watchdog: unable to allocate char dev region\n");
	return err;
}

/*
 *	watchdog_dev_exit: exit dev part of watchdog core
 *
 *	Release the range of chardev nodes used for watchdog devices
 */

void __exit watchdog_dev_exit(void)
{
	unregister_chrdev_region(watchdog_devt, MAX_DOGS);
}
