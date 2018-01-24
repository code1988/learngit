/*
 * net/dsa/dsa.c - Hardware switch handling
 * Copyright (c) 2008-2009 Marvell Semiconductor
 * Copyright (c) 2013 Florian Fainelli <florian@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <net/dsa.h>
#include <linux/of.h>
#include <linux/of_mdio.h>
#include <linux/of_platform.h>
#include "dsa_priv.h"

char dsa_driver_version[] = "0.1";


/* switch driver registration ***********************************************/
static DEFINE_MUTEX(dsa_switch_drivers_mutex);  // 定义了一个用于维护switch驱动链表的互斥锁
static LIST_HEAD(dsa_switch_drivers);           // 定义了一张全局的switch驱动链表

// 注册指定的switch驱动到内核
void register_switch_driver(struct dsa_switch_driver *drv)
{
	mutex_lock(&dsa_switch_drivers_mutex);
	list_add_tail(&drv->list, &dsa_switch_drivers);
	mutex_unlock(&dsa_switch_drivers_mutex);
}
EXPORT_SYMBOL_GPL(register_switch_driver);

// 从内核中注销指定的switch驱动
void unregister_switch_driver(struct dsa_switch_driver *drv)
{
	mutex_lock(&dsa_switch_drivers_mutex);
	list_del_init(&drv->list);
	mutex_unlock(&dsa_switch_drivers_mutex);
}
EXPORT_SYMBOL_GPL(unregister_switch_driver);

/* 通过mii-bus设备探测指定的switch是否存在
 * @bus     mii-bus设备，探测动作将通过该设备发出
 * @sw_addr 要探测的switch的地址序号
 * @_name   用于存放探测到的switch名
 * @返回值  探测成功则返回对应的switch驱动，失败则返回NULL
 */
static struct dsa_switch_driver *
dsa_switch_probe(struct mii_bus *bus, int sw_addr, char **_name)
{
	struct dsa_switch_driver *ret;
	struct list_head *list;
	char *name;

	ret = NULL;
	name = NULL;

    // 依次遍历已经注册的所有switch驱动，执行其中的probe操作，如果探测成功则会将探测到的switch名存放在_name中
	mutex_lock(&dsa_switch_drivers_mutex);
	list_for_each(list, &dsa_switch_drivers) {
		struct dsa_switch_driver *drv;

		drv = list_entry(list, struct dsa_switch_driver, list);

		name = drv->probe(bus, sw_addr);
		if (name != NULL) {
			ret = drv;
			break;
		}
	}
	mutex_unlock(&dsa_switch_drivers_mutex);

	*_name = name;

	return ret;
}


/* basic switch operations **************************************************/
/* 根据传入的信息创建一个完整的switch实例
 * @dst     该switch实例所属的DSA实例
 * @index   该switch的序号，不级联情况下就是0
 * @parent  该switch实例所属的DSA设备
 * @bus     该switch实例使用的主mii-bus
 */
static struct dsa_switch *
dsa_switch_setup(struct dsa_switch_tree *dst, int index,
		 struct device *parent, struct mii_bus *bus)
{
	struct dsa_chip_data *pd = dst->pd->chip + index;
	struct dsa_switch_driver *drv;
	struct dsa_switch *ds;
	int ret;
	char *name;
	int i;
	bool valid_name_found = false;

	/*
	 * Probe for switch model.
     * 通过指定的mii-bus设备探测指定的switch是否存在
	 */
	drv = dsa_switch_probe(bus, pd->sw_addr, &name);
	if (drv == NULL) {
		printk(KERN_ERR "%s[%d]: could not detect attached switch\n",
		       dst->master_netdev->name, index);
		return ERR_PTR(-EINVAL);
	}
	printk(KERN_INFO "%s[%d]: detected a %s switch\n",
		dst->master_netdev->name, index, name);


	/*
	 * Allocate and initialise switch state.
     * 只有成功探测到的switch才会创建对应的实例，显然实际这里申请了switch实例 + switch私有空间
	 */
	ds = kzalloc(sizeof(*ds) + drv->priv_size, GFP_KERNEL);
	if (ds == NULL)
		return ERR_PTR(-ENOMEM);

	ds->dst = dst;
	ds->index = index;
	ds->pd = dst->pd->chip + index;
	ds->drv = drv;
	ds->master_mii_bus = bus;


	/*
	 * Validate supplied switch configuration.
     * 检查提供给该switch的配置信息是否有效
	 */
	for (i = 0; i < DSA_MAX_PORTS; i++) {
		char *name;

		name = pd->port_names[i];
		if (name == NULL)
			continue;

		if (!strcmp(name, "cpu")) {
            // 如果端口名为"cpu"，意味着是switch上是连接cpu的端口，一个DSA实例中只允许存在一个cpu口
			if (dst->cpu_switch != -1) {
				printk(KERN_ERR "multiple cpu ports?!\n");
				ret = -EINVAL;
				goto out;
			}
			dst->cpu_switch = index;
			dst->cpu_port = i;
		} else if (!strcmp(name, "dsa")) {
            // 如果端口名为"dsa"，意味着该端口开启了dsa功能
			ds->dsa_port_mask |= 1 << i;
		} else {
            // 除了"cpu"和"dsa"端口，其他端口都是普通的物理口
			ds->phys_port_mask |= 1 << i;
		}
		valid_name_found = true;
	}

	if (!valid_name_found && i == DSA_MAX_PORTS) {
		ret = -EINVAL;
		goto out;
	}

	/*
	 * If the CPU connects to this switch, set the switch tree
	 * tagging protocol to the preferred tagging format of this
	 * switch.
     *
     * 该DSA实例的dsa-tag同步自cpu口所在的switch
	 */
	if (ds->dst->cpu_switch == index)
		ds->dst->tag_protocol = drv->tag_protocol;


	/*
	 * Do basic register setup.
     * 初始化switch
	 */
	ret = drv->setup(ds);
	if (ret < 0)
		goto out;

    // 将该DSA实例的宿主netdev的硬件地址设置到switch中
	ret = drv->set_addr(ds, dst->master_netdev->dev_addr);
	if (ret < 0)
		goto out;

    // 为该switch申请一个从mii-bus设备
	ds->slave_mii_bus = mdiobus_alloc();
	if (ds->slave_mii_bus == NULL) {
		ret = -ENOMEM;
		goto out;
	}
    // 然后初始化这个从mii-bus设备
	dsa_slave_mii_bus_init(ds);

    // 最后注册这个从mdio-bus设备
	ret = mdiobus_register(ds->slave_mii_bus);
	if (ret < 0)
		goto out_free;


	/*
	 * Create network devices for physical switch ports.
     * 为该switch的所有物理口创建对应的netdev
	 */
	for (i = 0; i < DSA_MAX_PORTS; i++) {
		struct net_device *slave_dev;

		if (!(ds->phys_port_mask & (1 << i)))
			continue;

        // 为每个物理端口创建对应的从netdev
		slave_dev = dsa_slave_create(ds, parent, i, pd->port_names[i]);
		if (slave_dev == NULL) {
			printk(KERN_ERR "%s[%d]: can't create dsa "
			       "slave device for port %d(%s)\n",
			       dst->master_netdev->name,
			       index, i, pd->port_names[i]);
			continue;
		}

		ds->ports[i] = slave_dev;
	}

	return ds;

out_free:
	mdiobus_free(ds->slave_mii_bus);
out:
	kfree(ds);
	return ERR_PTR(ret);
}

static void dsa_switch_destroy(struct dsa_switch *ds)
{
}


/* link polling *************************************************************/
static void dsa_link_poll_work(struct work_struct *ugly)
{
	struct dsa_switch_tree *dst;
	int i;

	dst = container_of(ugly, struct dsa_switch_tree, link_poll_work);

	for (i = 0; i < dst->pd->nr_chips; i++) {
		struct dsa_switch *ds = dst->ds[i];

		if (ds != NULL && ds->drv->poll_link != NULL)
			ds->drv->poll_link(ds);
	}

	mod_timer(&dst->link_poll_timer, round_jiffies(jiffies + HZ));
}

static void dsa_link_poll_timer(unsigned long _dst)
{
	struct dsa_switch_tree *dst = (void *)_dst;

	schedule_work(&dst->link_poll_work);
}


/* platform driver init and cleanup *****************************************/
// 判断指定device所属的class是否为指定的分类
static int dev_is_class(struct device *dev, void *class)
{
	if (dev->class != NULL && !strcmp(dev->class->name, class))
		return 1;

	return 0;
}

/* 从指定device开始递归查找指定类的device
 *
 * 备注：其中包括了对找到的device引用计数加1
 */
static struct device *dev_find_class(struct device *parent, char *class)
{
	if (dev_is_class(parent, class)) {
		get_device(parent);
		return parent;
	}

	return device_find_child(parent, class, dev_is_class);
}

// 从指定device开始递归查找"mdio_bus"类的mii总线设备
static struct mii_bus *dev_to_mii_bus(struct device *dev)
{
	struct device *d;

	d = dev_find_class(dev, "mdio_bus");
	if (d != NULL) {
		struct mii_bus *bus;

        // 获取device对应的mii总线设备
		bus = to_mii_bus(d);
		put_device(d);

		return bus;
	}

	return NULL;
}

// 从指定device开始递归查找"net"类的netdev
static struct net_device *dev_to_net_device(struct device *dev)
{
	struct device *d;

    // 从该device开始递归查找"net"类的device
	d = dev_find_class(dev, "net");
	if (d != NULL) {
		struct net_device *nd;

        // 获取device对应的netdev
		nd = to_net_dev(d);
		dev_hold(nd);
		put_device(d);

		return nd;
	}

	return NULL;
}

#ifdef CONFIG_OF
// 设置级联switch情况下的switch端口路由表
static int dsa_of_setup_routing_table(struct dsa_platform_data *pd,
					struct dsa_chip_data *cd,
					int chip_index,
					struct device_node *link)
{
	int ret;
	const __be32 *reg;
	int link_port_addr;
	int link_sw_addr;
	struct device_node *parent_sw;
	int len;

	parent_sw = of_get_parent(link);
	if (!parent_sw)
		return -EINVAL;

	reg = of_get_property(parent_sw, "reg", &len);
	if (!reg || (len != sizeof(*reg) * 2))
		return -EINVAL;

	link_sw_addr = be32_to_cpup(reg + 1);

	if (link_sw_addr >= pd->nr_chips)
		return -EINVAL;

	/* First time routing table allocation */
	if (!cd->rtable) {
		cd->rtable = kmalloc(pd->nr_chips * sizeof(s8), GFP_KERNEL);
		if (!cd->rtable)
			return -ENOMEM;

		/* default to no valid uplink/downlink */
		memset(cd->rtable, -1, pd->nr_chips * sizeof(s8));
	}

	reg = of_get_property(link, "reg", NULL);
	if (!reg) {
		ret = -EINVAL;
		goto out;
	}

	link_port_addr = be32_to_cpup(reg);

	cd->rtable[link_sw_addr] = link_port_addr;

	return 0;
out:
	kfree(cd->rtable);
	return ret;
}

static void dsa_of_free_platform_data(struct dsa_platform_data *pd)
{
	int i;
	int port_index;

	for (i = 0; i < pd->nr_chips; i++) {
		port_index = 0;
		while (port_index < DSA_MAX_PORTS) {
			if (pd->chip[i].port_names[port_index])
				kfree(pd->chip[i].port_names[port_index]);
			port_index++;
		}
		kfree(pd->chip[i].rtable);
	}
	kfree(pd->chip);
}

// 解析DSA设备的dts节点，这过程中主要会创建DSA配置控制块和下属的switch配置控制块
static int dsa_of_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device_node *child, *mdio, *ethernet, *port, *link;
	struct mii_bus *mdio_bus;
	struct platform_device *ethernet_dev;
	struct dsa_platform_data *pd;
	struct dsa_chip_data *cd;
	const char *port_name;
	int chip_index, port_index;
	const unsigned int *sw_addr, *port_reg;
	int ret;

    // 获取记录了mdio设备信息的dts节点
	mdio = of_parse_phandle(np, "dsa,mii-bus", 0);
	if (!mdio)
		return -EINVAL;

    // 根据得到的mdio dts节点进一步获取对应的mdio设备
	mdio_bus = of_mdio_find_bus(mdio);
	if (!mdio_bus)
		return -EINVAL;

    // 获取记录了以太网控制器信息的dts节点
	ethernet = of_parse_phandle(np, "dsa,ethernet", 0);
	if (!ethernet)
		return -EINVAL;

    // 根据得到的以太网控制器dts节点进一步获取对应的以太网控制器设备
	ethernet_dev = of_find_device_by_node(ethernet);
	if (!ethernet_dev)
		return -ENODEV;

    // 创建并初始化DSA配置控制块
	pd = kzalloc(sizeof(*pd), GFP_KERNEL);
	if (!pd)
		return -ENOMEM;

	pdev->dev.platform_data = pd;       // 将DSA配置块跟DSA设备关联
	pd->netdev = &ethernet_dev->dev;    // 将DSA的宿主device设置为上面获取到的以太网控制器设备
	pd->nr_chips = of_get_child_count(np);  // 获取该DSA的dts节点中配置的switch数量
	if (pd->nr_chips > DSA_MAX_SWITCHES)
		pd->nr_chips = DSA_MAX_SWITCHES;

    // 创建并初始化switch配置控制块
	pd->chip = kzalloc(pd->nr_chips * sizeof(struct dsa_chip_data),
			GFP_KERNEL);
	if (!pd->chip) {
		ret = -ENOMEM;
		goto out_free;
	}

	chip_index = 0;
	for_each_available_child_of_node(np, child) {
		cd = &pd->chip[chip_index];

		cd->mii_bus = &mdio_bus->dev;       // 所有switch共用同一个mdio设备

        // 获取该switch的地址序号
		sw_addr = of_get_property(child, "reg", NULL);
		if (!sw_addr)
			continue;

		cd->sw_addr = be32_to_cpup(sw_addr);
		if (cd->sw_addr > PHY_MAX_ADDR)
			continue;

		for_each_available_child_of_node(child, port) {
            // 获取该switch每个端口的物理地址序号
			port_reg = of_get_property(port, "reg", NULL);
			if (!port_reg)
				continue;

			port_index = be32_to_cpup(port_reg);

            // 获取该switch每个端口名
			port_name = of_get_property(port, "label", NULL);
			if (!port_name)
				continue;

			cd->port_names[port_index] = kstrdup(port_name,
					GFP_KERNEL);
			if (!cd->port_names[port_index]) {
				ret = -ENOMEM;
				goto out_free_chip;
			}

            // 获取记录了链路信息的dts节点(用于级联情况)
			link = of_parse_phandle(port, "link", 0);

            // 如果配置了switch级联情况，并且该switch端口是dsa口且配置了"link"属性，则在这里设置switch的路由表
			if (!strcmp(port_name, "dsa") && link &&
					pd->nr_chips > 1) {
				ret = dsa_of_setup_routing_table(pd, cd,
						chip_index, link);
				if (ret)
					goto out_free_chip;
			}

			if (port_index == DSA_MAX_PORTS)
				break;
		}
	}

	return 0;

out_free_chip:
	dsa_of_free_platform_data(pd);
out_free:
	kfree(pd);
	pdev->dev.platform_data = NULL;
	return ret;
}

static void dsa_of_remove(struct platform_device *pdev)
{
	struct dsa_platform_data *pd = pdev->dev.platform_data;

	if (!pdev->dev.of_node)
		return;

	dsa_of_free_platform_data(pd);
	kfree(pd);
}
#else
static inline int dsa_of_probe(struct platform_device *pdev)
{
	return 0;
}

static inline void dsa_of_remove(struct platform_device *pdev)
{
}
#endif

/* DSA驱动API: probe回调函数
 * @pdev    指向匹配到的DSA设备
 */
static int dsa_probe(struct platform_device *pdev)
{
	static int dsa_version_printed;
	struct dsa_platform_data *pd = pdev->dev.platform_data;
	struct net_device *dev;
	struct dsa_switch_tree *dst;
	int i, ret;

	if (!dsa_version_printed++)
		printk(KERN_NOTICE "Distributed Switch Architecture "
			"driver version %s\n", dsa_driver_version);

    // 如果DSA设备存在对应的dts节点，则对其进行解析，这过程中主要会创建DSA配置控制块和下属的switch配置控制块
	if (pdev->dev.of_node) {
		ret = dsa_of_probe(pdev);
		if (ret)
			return ret;

		pd = pdev->dev.platform_data;
	}

	if (pd == NULL || pd->netdev == NULL)
		return -EINVAL;

    // 获取宿主device的父结构netdev
	dev = dev_to_net_device(pd->netdev);
	if (dev == NULL) {
		ret = -EINVAL;
		goto out;
	}

	if (dev->dsa_ptr != NULL) {
		dev_put(dev);
		ret = -EEXIST;
		goto out;
	}

    // 创建并初始化DSA实例
	dst = kzalloc(sizeof(*dst), GFP_KERNEL);
	if (dst == NULL) {
		dev_put(dev);
		ret = -ENOMEM;
		goto out;
	}

	platform_set_drvdata(pdev, dst);    // 将DSA实例作为私有数据记录到该DSA设备的私有数据块中

	dst->pd = pd;
	dst->master_netdev = dev;
	dst->cpu_switch = -1;
	dst->cpu_port = -1;

    // 创建配置的每个switch实例(不级联就是1个)
	for (i = 0; i < pd->nr_chips; i++) {
		struct mii_bus *bus;
		struct dsa_switch *ds;

        // 获取对应的mii-bus设备，这里也就是mdio设备
		bus = dev_to_mii_bus(pd->chip[i].mii_bus);
		if (bus == NULL) {
			printk(KERN_ERR "%s[%d]: no mii bus found for "
				"dsa switch\n", dev->name, i);
			continue;
		}

        // 创建完整的switch实例
		ds = dsa_switch_setup(dst, i, &pdev->dev, bus);
		if (IS_ERR(ds)) {
			printk(KERN_ERR "%s[%d]: couldn't create dsa switch "
				"instance (error %ld)\n", dev->name, i,
				PTR_ERR(ds));
			continue;
		}

		dst->ds[i] = ds;
		if (ds->drv->poll_link != NULL)
			dst->link_poll_needed = 1;
	}

	/*
	 * If we use a tagging format that doesn't have an ethertype
	 * field, make sure that all packets from this point on get
	 * sent to the tag format's receive function.
	 */
	wmb();
	dev->dsa_ptr = (void *)dst;

	if (dst->link_poll_needed) {
		INIT_WORK(&dst->link_poll_work, dsa_link_poll_work);
		init_timer(&dst->link_poll_timer);
		dst->link_poll_timer.data = (unsigned long)dst;
		dst->link_poll_timer.function = dsa_link_poll_timer;
		dst->link_poll_timer.expires = round_jiffies(jiffies + HZ);
		add_timer(&dst->link_poll_timer);
	}

	return 0;

out:
	dsa_of_remove(pdev);

	return ret;
}

// DSA驱动的remove回调函数
static int dsa_remove(struct platform_device *pdev)
{
	struct dsa_switch_tree *dst = platform_get_drvdata(pdev);
	int i;

	if (dst->link_poll_needed)
		del_timer_sync(&dst->link_poll_timer);

	flush_work(&dst->link_poll_work);

	for (i = 0; i < dst->pd->nr_chips; i++) {
		struct dsa_switch *ds = dst->ds[i];

		if (ds != NULL)
			dsa_switch_destroy(ds);
	}

	dsa_of_remove(pdev);

	return 0;
}

static void dsa_shutdown(struct platform_device *pdev)
{
}

/* 定义了一张通用DSA驱动支持的device描述列表，只有符合该表中描述的device才是匹配的device
 * 
 * 备注： 3.14.38版本中DSA驱动只支持marvell
 */
static const struct of_device_id dsa_of_match_table[] = {
	{ .compatible = "marvell,dsa", },
	{}
};
MODULE_DEVICE_TABLE(of, dsa_of_match_table);

// 定义了一个platform类型的DSA驱动
static struct platform_driver dsa_driver = {
	.probe		= dsa_probe,
	.remove		= dsa_remove,
	.shutdown	= dsa_shutdown,
	.driver = {
		.name	= "dsa",
		.owner	= THIS_MODULE,
		.of_match_table = dsa_of_match_table,
	},
};

// 通用DSA驱动初始化入口
static int __init dsa_init_module(void)
{
	int rc;

    // 将DSA驱动注册到platform总线中
	rc = platform_driver_register(&dsa_driver);
	if (rc)
		return rc;

    // 注册具体的DSA协议，目前就是dsa_packet_type
#ifdef CONFIG_NET_DSA_TAG_DSA
	dev_add_pack(&dsa_packet_type);
#endif
#ifdef CONFIG_NET_DSA_TAG_EDSA
	dev_add_pack(&edsa_packet_type);
#endif
#ifdef CONFIG_NET_DSA_TAG_TRAILER
	dev_add_pack(&trailer_packet_type);
#endif
	return 0;
}
module_init(dsa_init_module);

// 通用DSA驱动卸载入口
static void __exit dsa_cleanup_module(void)
{
#ifdef CONFIG_NET_DSA_TAG_TRAILER
	dev_remove_pack(&trailer_packet_type);
#endif
#ifdef CONFIG_NET_DSA_TAG_EDSA
	dev_remove_pack(&edsa_packet_type);
#endif
#ifdef CONFIG_NET_DSA_TAG_DSA
	dev_remove_pack(&dsa_packet_type);
#endif
	platform_driver_unregister(&dsa_driver);
}
module_exit(dsa_cleanup_module);

MODULE_AUTHOR("Lennert Buytenhek <buytenh@wantstofly.org>");
MODULE_DESCRIPTION("Driver for Distributed Switch Architecture switch chips");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:dsa");
