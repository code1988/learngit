/*
 * INET		802.1Q VLAN
 *		Ethernet-type device handling.
 *
 * Authors:	Ben Greear <greearb@candelatech.com>
 *              Please send support related email to: netdev@vger.kernel.org
 *              VLAN Home Page: http://www.candelatech.com/~greear/vlan.html
 *
 * Fixes:
 *              Fix for packet capture - Nick Eggleston <nick@dccinc.com>;
 *		Add HW acceleration hooks - David S. Miller <davem@redhat.com>;
 *		Correct all the locking - David S. Miller <davem@redhat.com>;
 *		Use hash table for VLAN groups - David S. Miller <davem@redhat.com>
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/capability.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/rculist.h>
#include <net/p8022.h>
#include <net/arp.h>
#include <linux/rtnetlink.h>
#include <linux/notifier.h>
#include <net/rtnetlink.h>
#include <net/net_namespace.h>
#include <net/netns/generic.h>
#include <asm/uaccess.h>

#include <linux/if_vlan.h>
#include "vlan.h"
#include "vlanproc.h"

#define DRV_VERSION "1.8"

/* Global VLAN variables */

int vlan_net_id __read_mostly;  // 用于记录vlan模块在所有网络命名空间中的ID号（注册到网络命名空间时分配）

const char vlan_fullname[] = "802.1Q VLAN Support";
const char vlan_version[] = DRV_VERSION;

/* End of global variables definitions. */

// 对四维结构的vlan组模型在第三维度上进行预分配
static int vlan_group_prealloc_vid(struct vlan_group *vg,
				   __be16 vlan_proto, u16 vlan_id)
{
	struct net_device **array;
	unsigned int pidx, vidx;
	unsigned int size;

	ASSERT_RTNL();

    // 根据vlan协议序号和vlan id序号，索引对应的二维数组元素
	pidx  = vlan_proto_idx(vlan_proto);
	vidx  = vlan_id / VLAN_GROUP_ARRAY_PART_LEN;
	array = vg->vlan_devices_arrays[pidx][vidx];
    // 每个二维数组元素又被看作是一个指针数组，数组的元素是指向每个vlan设备的指针
    // 如果对应的二维数组元素存在，也就是已经做过第三维度的预分配，所以直接返回
	if (array != NULL)
		return 0;

    // 程序运行到这里意味着对应的第三维度还未存在，则申请一个包含VLAN_GROUP_ARRAY_PART_LEN个元素的指针数组
	size = sizeof(struct net_device *) * VLAN_GROUP_ARRAY_PART_LEN;
	array = kzalloc(size, GFP_KERNEL);
	if (array == NULL)
		return -ENOBUFS;

	vg->vlan_devices_arrays[pidx][vidx] = array;
	return 0;
}

// 注销一个vlan设备
void unregister_vlan_dev(struct net_device *dev, struct list_head *head)
{
	struct vlan_dev_priv *vlan = vlan_dev_priv(dev);
	struct net_device *real_dev = vlan->real_dev;
	struct vlan_info *vlan_info;
	struct vlan_group *grp;
	u16 vlan_id = vlan->vlan_id;

	ASSERT_RTNL();

	vlan_info = rtnl_dereference(real_dev->vlan_info);
	BUG_ON(!vlan_info);

	grp = &vlan_info->grp;

	grp->nr_vlan_devs--;

	if (vlan->flags & VLAN_FLAG_MVRP)
		vlan_mvrp_request_leave(dev);
	if (vlan->flags & VLAN_FLAG_GVRP)
		vlan_gvrp_request_leave(dev);

	vlan_group_set_device(grp, vlan->vlan_proto, vlan_id, NULL);

	netdev_upper_dev_unlink(real_dev, dev);
	/* Because unregister_netdevice_queue() makes sure at least one rcu
	 * grace period is respected before device freeing,
	 * we dont need to call synchronize_net() here.
	 */
	unregister_netdevice_queue(dev, head);

	if (grp->nr_vlan_devs == 0) {
		vlan_mvrp_uninit_applicant(real_dev);
		vlan_gvrp_uninit_applicant(real_dev);
	}

	/* Take it out of our own structures, but be sure to interlock with
	 * HW accelerating devices or SW vlan input packet processing if
	 * VLAN is not 0 (leave it there for 802.1p).
	 */
	if (vlan_id)
		vlan_vid_del(real_dev, vlan->vlan_proto, vlan_id);

	/* Get rid of the vlan's reference to real_dev */
	dev_put(real_dev);
}

// 检查宿主设备是否支持vlan协议以及要创建的vlan id在该设备上是否已经存在
int vlan_check_real_dev(struct net_device *real_dev,
			__be16 protocol, u16 vlan_id)
{
	const char *name = real_dev->name;

    // 检查该宿主设备的功能列表中是否支持vlan协议
	if (real_dev->features & NETIF_F_VLAN_CHALLENGED) {
		pr_info("VLANs not supported on %s\n", name);
		return -EOPNOTSUPP;
	}

    // 检查相同vlan id的vlan设备是否已经在该宿主设备上存在
	if (vlan_find_dev(real_dev, protocol, vlan_id) != NULL)
		return -EEXIST;

	return 0;
}

// 进一步注册一个新的vlan设备
int register_vlan_dev(struct net_device *dev)
{
	struct vlan_dev_priv *vlan = vlan_dev_priv(dev);    // 获取vlan设备附属的私有空间
	struct net_device *real_dev = vlan->real_dev;       // 获取宿主设备 
	u16 vlan_id = vlan->vlan_id;    // 获取vlan id
	struct vlan_info *vlan_info;
	struct vlan_group *grp;
	int err;

    // 将指定vlan协议ID的vlan id添加到宿主设备的vlan_info管理块中
	err = vlan_vid_add(real_dev, vlan->vlan_proto, vlan_id);
	if (err)
		return err;

    // 确保程序运行到这里时宿主设备的vlan_info成员不再为空
	vlan_info = rtnl_dereference(real_dev->vlan_info);
	/* vlan_info should be there now. vlan_vid_add took care of it */
	BUG_ON(!vlan_info);

	grp = &vlan_info->grp;
    // 该宿主设备第一次绑定vlan设备时，需要初始化gvrp和mvrp功能(如果使能了这两个功能)
	if (grp->nr_vlan_devs == 0) {
		err = vlan_gvrp_init_applicant(real_dev);
		if (err < 0)
			goto out_vid_del;
		err = vlan_mvrp_init_applicant(real_dev);
		if (err < 0)
			goto out_uninit_gvrp;
	}

    // 对四维结构的vlan组模型在第三维度上进行预分配
	err = vlan_group_prealloc_vid(grp, vlan->vlan_proto, vlan_id);
	if (err < 0)
		goto out_uninit_mvrp;

    // 设置该vlan设备的嵌套级别
	vlan->nest_level = dev_get_nest_level(real_dev, is_vlan_dev) + 1;
    // 注册该网络设备到内核中，注册结果会从通知链中反馈
	err = register_netdevice(dev);
	if (err < 0)
		goto out_uninit_mvrp;

    // 将vlan设备加入宿主设备的upper邻接链表中
	err = netdev_upper_dev_link(real_dev, dev);
	if (err)
		goto out_unregister_netdev;

	/* Account for reference in struct vlan_dev_priv 
     * 宿主设备的引用计数加1
     * */
	dev_hold(real_dev);

    // 从宿主设备继承一些状态(比如设备的链路状态)
	netif_stacked_transfer_operstate(real_dev, dev);
    // 将vlan设备加入lweventlist通知链
	linkwatch_fire_event(dev); /* _MUST_ call rfc2863_policy() */

	/* So, got the sucker initialized, now lets place
	 * it into our local structure.
     * 将指定的vlan设备记录到四维vlan组模型中
	 */
	vlan_group_set_device(grp, vlan->vlan_proto, vlan_id, dev);
    // vlan组中的vlan设备数量加1
	grp->nr_vlan_devs++;

	return 0;

out_unregister_netdev:
	unregister_netdevice(dev);
out_uninit_mvrp:
	if (grp->nr_vlan_devs == 0)
		vlan_mvrp_uninit_applicant(real_dev);
out_uninit_gvrp:
	if (grp->nr_vlan_devs == 0)
		vlan_gvrp_uninit_applicant(real_dev);
out_vid_del:
	vlan_vid_del(real_dev, vlan->vlan_proto, vlan_id);
	return err;
}

/*  Attach a VLAN device to a mac address (ie Ethernet Card).
 *  注册一个新的vlan设备
 *  @real_dev   - 宿主设备(一般就是真实的网卡，但对交换机来说，可能是DSA中创建的虚拟端口)
 *  @vlan_id    - 要创建的vlan设备的ID号
 *  Returns 0 if the device was created or a negative error code otherwise.
 */
static int register_vlan_device(struct net_device *real_dev, u16 vlan_id)
{
	struct net_device *new_dev;
	struct vlan_dev_priv *vlan;
	struct net *net = dev_net(real_dev);
    // 根据vlan_net_id在该网络命名空间下索引对应的私有数据块，也就是vlan_net
	struct vlan_net *vn = net_generic(net, vlan_net_id);
	char name[IFNAMSIZ];
	int err;

    // vlan id号不能超过4096
	if (vlan_id >= VLAN_VID_MASK)
		return -ERANGE;

    // 检查宿主设备是否支持vlan协议以及要创建的vlan id在该设备上是否已经存在
	err = vlan_check_real_dev(real_dev, htons(ETH_P_8021Q), vlan_id);
	if (err < 0)
		return err;

	/* Gotta set up the fields for the device. 
     * 根据vlan设备在用户层不同的显示风格生成相应的vlan设备名
     * */
	switch (vn->name_type) {
	case VLAN_NAME_TYPE_RAW_PLUS_VID:
		/* name will look like:	 eth1.0005 */
		snprintf(name, IFNAMSIZ, "%s.%.4i", real_dev->name, vlan_id);
		break;
	case VLAN_NAME_TYPE_PLUS_VID_NO_PAD:
		/* Put our vlan.VID in the name.
		 * Name will look like:	 vlan5
		 */
		snprintf(name, IFNAMSIZ, "vlan%i", vlan_id);
		break;
	case VLAN_NAME_TYPE_RAW_PLUS_VID_NO_PAD:
		/* Put our vlan.VID in the name.
		 * Name will look like:	 eth0.5
		 */
		snprintf(name, IFNAMSIZ, "%s.%i", real_dev->name, vlan_id);
		break;
	case VLAN_NAME_TYPE_PLUS_VID:
		/* Put our vlan.VID in the name.
		 * Name will look like:	 vlan0005
		 */
	default:
		snprintf(name, IFNAMSIZ, "vlan%.4i", vlan_id);
	}

    // 创建一个vlan设备，并执行vlan_setup完成基本的初始化
	new_dev = alloc_netdev(sizeof(struct vlan_dev_priv), name, vlan_setup);

	if (new_dev == NULL)
		return -ENOBUFS;

    // 将新创建的vlan设备关联到当前的网络命名空间
	dev_net_set(new_dev, net);
	/* need 4 bytes for extra VLAN header info,
	 * hope the underlying device can handle it.
	 */
	new_dev->mtu = real_dev->mtu;   // 将宿主设备的mtu继承到vlan设备
	new_dev->priv_flags |= (real_dev->priv_flags & IFF_UNICAST_FLT);    // 如果宿主设备支持单播过滤功能，则同样继承到vlan设备

    // 获取vlan设备附属的私有空间，初始化其中的成员
	vlan = vlan_dev_priv(new_dev);
	vlan->vlan_proto = htons(ETH_P_8021Q);  // 记录vlan类型ID号
	vlan->vlan_id = vlan_id;                // 记录vlan id
	vlan->real_dev = real_dev;              // 记录宿主设备
	vlan->dent = NULL;                      // 清除在proc文件系统中的入口
	vlan->flags = VLAN_FLAG_REORDER_HDR;

	new_dev->rtnl_link_ops = &vlan_link_ops;// 注册rtnetlink接口管理该vlan设备的回调函数集
    
    // 进一步注册一个新的vlan设备
	err = register_vlan_dev(new_dev);
	if (err < 0)
		goto out_free_newdev;

	return 0;

out_free_newdev:
	free_netdev(new_dev);
	return err;
}

static void vlan_sync_address(struct net_device *dev,
			      struct net_device *vlandev)
{
	struct vlan_dev_priv *vlan = vlan_dev_priv(vlandev);

	/* May be called without an actual change */
	if (ether_addr_equal(vlan->real_dev_addr, dev->dev_addr))
		return;

	/* vlan address was different from the old address and is equal to
	 * the new address */
	if (!ether_addr_equal(vlandev->dev_addr, vlan->real_dev_addr) &&
	    ether_addr_equal(vlandev->dev_addr, dev->dev_addr))
		dev_uc_del(dev, vlandev->dev_addr);

	/* vlan address was equal to the old address and is different from
	 * the new address */
	if (ether_addr_equal(vlandev->dev_addr, vlan->real_dev_addr) &&
	    !ether_addr_equal(vlandev->dev_addr, dev->dev_addr))
		dev_uc_add(dev, vlandev->dev_addr);

	ether_addr_copy(vlan->real_dev_addr, dev->dev_addr);
}

static void vlan_transfer_features(struct net_device *dev,
				   struct net_device *vlandev)
{
	struct vlan_dev_priv *vlan = vlan_dev_priv(vlandev);

	vlandev->gso_max_size = dev->gso_max_size;

	if (vlan_hw_offload_capable(dev->features, vlan->vlan_proto))
		vlandev->hard_header_len = dev->hard_header_len;
	else
		vlandev->hard_header_len = dev->hard_header_len + VLAN_HLEN;

#if IS_ENABLED(CONFIG_FCOE)
	vlandev->fcoe_ddp_xid = dev->fcoe_ddp_xid;
#endif

	netdev_update_features(vlandev);
}

static void __vlan_device_event(struct net_device *dev, unsigned long event)
{
	switch (event) {
	case NETDEV_CHANGENAME:
		vlan_proc_rem_dev(dev);
		if (vlan_proc_add_dev(dev) < 0)
			pr_warn("failed to change proc name for %s\n",
				dev->name);
		break;
	case NETDEV_REGISTER:
		if (vlan_proc_add_dev(dev) < 0)
			pr_warn("failed to add proc entry for %s\n", dev->name);
		break;
	case NETDEV_UNREGISTER:
		vlan_proc_rem_dev(dev);
		break;
	}
}

static int vlan_device_event(struct notifier_block *unused, unsigned long event,
			     void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	struct vlan_group *grp;
	struct vlan_info *vlan_info;
	int i, flgs;
	struct net_device *vlandev;
	struct vlan_dev_priv *vlan;
	bool last = false;
	LIST_HEAD(list);

	if (is_vlan_dev(dev))
		__vlan_device_event(dev, event);

	if ((event == NETDEV_UP) &&
	    (dev->features & NETIF_F_HW_VLAN_CTAG_FILTER)) {
		pr_info("adding VLAN 0 to HW filter on device %s\n",
			dev->name);
		vlan_vid_add(dev, htons(ETH_P_8021Q), 0);
	}

	vlan_info = rtnl_dereference(dev->vlan_info);
	if (!vlan_info)
		goto out;
	grp = &vlan_info->grp;

	/* It is OK that we do not hold the group lock right now,
	 * as we run under the RTNL lock.
	 */

	switch (event) {
	case NETDEV_CHANGE:
		/* Propagate real device state to vlan devices */
		vlan_group_for_each_dev(grp, i, vlandev)
			netif_stacked_transfer_operstate(dev, vlandev);
		break;

	case NETDEV_CHANGEADDR:
		/* Adjust unicast filters on underlying device */
		vlan_group_for_each_dev(grp, i, vlandev) {
			flgs = vlandev->flags;
			if (!(flgs & IFF_UP))
				continue;

			vlan_sync_address(dev, vlandev);
		}
		break;

	case NETDEV_CHANGEMTU:
		vlan_group_for_each_dev(grp, i, vlandev) {
			if (vlandev->mtu <= dev->mtu)
				continue;

			dev_set_mtu(vlandev, dev->mtu);
		}
		break;

	case NETDEV_FEAT_CHANGE:
		/* Propagate device features to underlying device */
		vlan_group_for_each_dev(grp, i, vlandev)
			vlan_transfer_features(dev, vlandev);
		break;

	case NETDEV_DOWN:
		if (dev->features & NETIF_F_HW_VLAN_CTAG_FILTER)
			vlan_vid_del(dev, htons(ETH_P_8021Q), 0);

		/* Put all VLANs for this dev in the down state too.  */
		vlan_group_for_each_dev(grp, i, vlandev) {
			flgs = vlandev->flags;
			if (!(flgs & IFF_UP))
				continue;

			vlan = vlan_dev_priv(vlandev);
			if (!(vlan->flags & VLAN_FLAG_LOOSE_BINDING))
				dev_change_flags(vlandev, flgs & ~IFF_UP);
			netif_stacked_transfer_operstate(dev, vlandev);
		}
		break;

	case NETDEV_UP:
		/* Put all VLANs for this dev in the up state too.  */
		vlan_group_for_each_dev(grp, i, vlandev) {
			flgs = vlandev->flags;
			if (flgs & IFF_UP)
				continue;

			vlan = vlan_dev_priv(vlandev);
			if (!(vlan->flags & VLAN_FLAG_LOOSE_BINDING))
				dev_change_flags(vlandev, flgs | IFF_UP);
			netif_stacked_transfer_operstate(dev, vlandev);
		}
		break;

	case NETDEV_UNREGISTER:
		/* twiddle thumbs on netns device moves */
		if (dev->reg_state != NETREG_UNREGISTERING)
			break;

		vlan_group_for_each_dev(grp, i, vlandev) {
			/* removal of last vid destroys vlan_info, abort
			 * afterwards */
			if (vlan_info->nr_vids == 1)
				last = true;

			unregister_vlan_dev(vlandev, &list);
			if (last)
				break;
		}
		unregister_netdevice_many(&list);
		break;

	case NETDEV_PRE_TYPE_CHANGE:
		/* Forbid underlaying device to change its type. */
		if (vlan_uses_dev(dev))
			return NOTIFY_BAD;
		break;

	case NETDEV_NOTIFY_PEERS:
	case NETDEV_BONDING_FAILOVER:
	case NETDEV_RESEND_IGMP:
		/* Propagate to vlan devices */
		vlan_group_for_each_dev(grp, i, vlandev)
			call_netdevice_notifiers(event, vlandev);
		break;
	}

out:
	return NOTIFY_DONE;
}

static struct notifier_block vlan_notifier_block __read_mostly = {
	.notifier_call = vlan_device_event,
};

/*
 *	VLAN IOCTL handler.
 *	用于ioctl的vlan操作钩子函数 
 *	o execute requested action or pass command to the device driver
 *   arg is really a struct vlan_ioctl_args __user *.
 */
static int vlan_ioctl_handler(struct net *net, void __user *arg)
{
	int err;
	struct vlan_ioctl_args args;
	struct net_device *dev = NULL;

    // 将ioctl的vlan传入参数从用户空间拷贝到内核
	if (copy_from_user(&args, arg, sizeof(struct vlan_ioctl_args)))
		return -EFAULT;

	/* Null terminate this sucker, just in case. */
	args.device1[23] = 0;
	args.u.device2[23] = 0;

    // rtnl上锁
	rtnl_lock();

	switch (args.cmd) {
	case SET_VLAN_INGRESS_PRIORITY_CMD:
	case SET_VLAN_EGRESS_PRIORITY_CMD:
	case SET_VLAN_FLAG_CMD:
	case ADD_VLAN_CMD:
	case DEL_VLAN_CMD:
	case GET_VLAN_REALDEV_NAME_CMD:
	case GET_VLAN_VID_CMD:
		err = -ENODEV;
        // 根据用户传入的设备名查找实际对应的设备管理块
		dev = __dev_get_by_name(net, args.device1);
		if (!dev)
			goto out;

		err = -EINVAL;
        // 如果不是添加vlan设备命令时，需要检测设备是否为vlan设备
		if (args.cmd != ADD_VLAN_CMD && !is_vlan_dev(dev))
			goto out;
	}

	switch (args.cmd) {
	case SET_VLAN_INGRESS_PRIORITY_CMD:
		err = -EPERM;
        // 设置vlan入口优先级命令的用户需要进行管理员级别权限测试
		if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
			break;
		vlan_dev_set_ingress_priority(dev,
					      args.u.skb_priority,
					      args.vlan_qos);
		err = 0;
		break;

	case SET_VLAN_EGRESS_PRIORITY_CMD:
		err = -EPERM;
        // 设置vlan出口优先级命令的用户需要进行管理员级别权限测试
		if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
			break;
		err = vlan_dev_set_egress_priority(dev,
						   args.u.skb_priority,
						   args.vlan_qos);
		break;

	case SET_VLAN_FLAG_CMD:
		err = -EPERM;
        // 设置vlan标志位命令的用户需要进行管理员级别权限测试
		if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
			break;
		err = vlan_dev_change_flags(dev,
					    args.vlan_qos ? args.u.flag : 0,
					    args.u.flag);
		break;

	case SET_VLAN_NAME_TYPE_CMD:
		err = -EPERM;
        // 设置vlan设备名风格的用户需要进行管理员级别权限测试
		if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
			break;
		if ((args.u.name_type >= 0) &&
		    (args.u.name_type < VLAN_NAME_TYPE_HIGHEST)) {
			struct vlan_net *vn;

			vn = net_generic(net, vlan_net_id);
			vn->name_type = args.u.name_type;
			err = 0;
		} else {
			err = -EINVAL;
		}
		break;

	case ADD_VLAN_CMD:
		err = -EPERM;
        // 添加vlan设备的用户需要进行管理员级别权限测试
		if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
			break;
        // 在宿主设备上注册一个新的vlan设备
		err = register_vlan_device(dev, args.u.VID);
		break;

	case DEL_VLAN_CMD:
		err = -EPERM;
        // 删除vlan设备的用户需要进行管理员级别权限测试
		if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
			break;
		unregister_vlan_dev(dev, NULL);
		err = 0;
		break;

	case GET_VLAN_REALDEV_NAME_CMD:
		err = 0;
		vlan_dev_get_realdev_name(dev, args.u.device2);
		if (copy_to_user(arg, &args,
				 sizeof(struct vlan_ioctl_args)))
			err = -EFAULT;
		break;

	case GET_VLAN_VID_CMD:
		err = 0;
		args.u.VID = vlan_dev_vlan_id(dev);
		if (copy_to_user(arg, &args,
				 sizeof(struct vlan_ioctl_args)))
		      err = -EFAULT;
		break;

	default:
		err = -EOPNOTSUPP;
		break;
	}
out:
	rtnl_unlock();
	return err;
}

// 具体的vlan功能初始化(实际就是创建proc文件系统下的vlan接口)
static int __net_init vlan_init_net(struct net *net)
{
    // 根据vlan_net_id在该网络命名空间下索引对应的私有数据块，也就是vlan_net
	struct vlan_net *vn = net_generic(net, vlan_net_id);
	int err;

    // 设置用户层vlan的默认显示风格，类似 eth0.10
	vn->name_type = VLAN_NAME_TYPE_RAW_PLUS_VID_NO_PAD;

    // 在proc文件系统中创建vlan接口(/proc/net/vlan)
	err = vlan_proc_init(net);

	return err;
}

// vlan功能注销
static void __net_exit vlan_exit_net(struct net *net)
{
	vlan_proc_cleanup(net);
}

// 定义了vlan模块在网络命名空间层面的操作集合
static struct pernet_operations vlan_net_ops = {
	.init = vlan_init_net,
	.exit = vlan_exit_net,
	.id   = &vlan_net_id,
	.size = sizeof(struct vlan_net),
};

// vlan模块初始化
static int __init vlan_proto_init(void)
{
	int err;

	pr_info("%s v%s\n", vlan_fullname, vlan_version);

    // 将vlan模块添加到每一个网络命名空间，并且执行了vlan_init_net
	err = register_pernet_subsys(&vlan_net_ops);
	if (err < 0)
		goto err0;

    // 向通知链中注册一个vlan事件通知块
	err = register_netdevice_notifier(&vlan_notifier_block);
	if (err < 0)
		goto err2;

    // 没有配置CONFIG_VLAN_8021Q_GVRP功能时，这里不做任何事
	err = vlan_gvrp_init();
	if (err < 0)
		goto err3;

    // 没有配置CONFIG_VLAN_8021Q_MVRP功能时，这里不做任何事
	err = vlan_mvrp_init();
	if (err < 0)
		goto err4;

    // 初始化操作vlan用的netlink接口
	err = vlan_netlink_init();
	if (err < 0)
		goto err5;

    // 设置用于vlan操作的ioctl钩子函数
	vlan_ioctl_set(vlan_ioctl_handler);
	return 0;

err5:
	vlan_mvrp_uninit();
err4:
	vlan_gvrp_uninit();
err3:
	unregister_netdevice_notifier(&vlan_notifier_block);
err2:
	unregister_pernet_subsys(&vlan_net_ops);
err0:
	return err;
}

static void __exit vlan_cleanup_module(void)
{
	vlan_ioctl_set(NULL);
	vlan_netlink_fini();

	unregister_netdevice_notifier(&vlan_notifier_block);

	unregister_pernet_subsys(&vlan_net_ops);
	rcu_barrier(); /* Wait for completion of call_rcu()'s */

	vlan_mvrp_uninit();
	vlan_gvrp_uninit();
}

module_init(vlan_proto_init);
module_exit(vlan_cleanup_module);

MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
