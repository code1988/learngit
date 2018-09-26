/*
 * net/dsa/dsa.c - Hardware switch handling
 * 其实这部分dsa驱动不应该放在net目录下，而是应该合并到drivers/net/dsa中
 * Copyright (c) 2008-2009 Marvell Semiconductor
 * Copyright (c) 2013 Florian Fainelli <florian@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/ctype.h>
#include <linux/device.h>
#include <linux/hwmon.h>
#include <linux/list.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <net/dsa.h>
#include <linux/of.h>
#include <linux/of_mdio.h>
#include <linux/of_platform.h>
#include <linux/of_net.h>
#include <linux/sysfs.h>
#include <linux/phy_fixed.h>
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
dsa_switch_probe(struct device *host_dev, int sw_addr, char **_name)
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

		name = drv->probe(host_dev, sw_addr);
		if (name != NULL) {
			ret = drv;
			break;
		}
	}
	mutex_unlock(&dsa_switch_drivers_mutex);

	*_name = name;

	return ret;
}

/* hwmon support ************************************************************/

#ifdef CONFIG_NET_DSA_HWMON

static ssize_t temp1_input_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct dsa_switch *ds = dev_get_drvdata(dev);
	int temp, ret;

	ret = ds->drv->get_temp(ds, &temp);
	if (ret < 0)
		return ret;

	return sprintf(buf, "%d\n", temp * 1000);
}
static DEVICE_ATTR_RO(temp1_input);

static ssize_t temp1_max_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	struct dsa_switch *ds = dev_get_drvdata(dev);
	int temp, ret;

	ret = ds->drv->get_temp_limit(ds, &temp);
	if (ret < 0)
		return ret;

	return sprintf(buf, "%d\n", temp * 1000);
}

static ssize_t temp1_max_store(struct device *dev,
			       struct device_attribute *attr, const char *buf,
			       size_t count)
{
	struct dsa_switch *ds = dev_get_drvdata(dev);
	int temp, ret;

	ret = kstrtoint(buf, 0, &temp);
	if (ret < 0)
		return ret;

	ret = ds->drv->set_temp_limit(ds, DIV_ROUND_CLOSEST(temp, 1000));
	if (ret < 0)
		return ret;

	return count;
}
static DEVICE_ATTR_RW(temp1_max);

static ssize_t temp1_max_alarm_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct dsa_switch *ds = dev_get_drvdata(dev);
	bool alarm;
	int ret;

	ret = ds->drv->get_temp_alarm(ds, &alarm);
	if (ret < 0)
		return ret;

	return sprintf(buf, "%d\n", alarm);
}
static DEVICE_ATTR_RO(temp1_max_alarm);

static struct attribute *dsa_hwmon_attrs[] = {
	&dev_attr_temp1_input.attr,	/* 0 */
	&dev_attr_temp1_max.attr,	/* 1 */
	&dev_attr_temp1_max_alarm.attr,	/* 2 */
	NULL
};

static umode_t dsa_hwmon_attrs_visible(struct kobject *kobj,
				       struct attribute *attr, int index)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct dsa_switch *ds = dev_get_drvdata(dev);
	struct dsa_switch_driver *drv = ds->drv;
	umode_t mode = attr->mode;

	if (index == 1) {
		if (!drv->get_temp_limit)
			mode = 0;
		else if (!drv->set_temp_limit)
			mode &= ~S_IWUSR;
	} else if (index == 2 && !drv->get_temp_alarm) {
		mode = 0;
	}
	return mode;
}

static const struct attribute_group dsa_hwmon_group = {
	.attrs = dsa_hwmon_attrs,
	.is_visible = dsa_hwmon_attrs_visible,
};
__ATTRIBUTE_GROUPS(dsa_hwmon);

#endif /* CONFIG_NET_DSA_HWMON */

/* basic switch operations **************************************************/
static int dsa_cpu_dsa_setup(struct dsa_switch *ds, struct net_device *master)
{
	struct dsa_chip_data *cd = ds->pd;
	struct device_node *port_dn;
	struct phy_device *phydev;
	int ret, port, mode;

	for (port = 0; port < DSA_MAX_PORTS; port++) {
		if (!(dsa_is_cpu_port(ds, port) || dsa_is_dsa_port(ds, port)))
			continue;

		port_dn = cd->port_dn[port];
		if (of_phy_is_fixed_link(port_dn)) {
			ret = of_phy_register_fixed_link(port_dn);
			if (ret) {
				netdev_err(master,
					   "failed to register fixed PHY\n");
				return ret;
			}
			phydev = of_phy_find_device(port_dn);

			mode = of_get_phy_mode(port_dn);
			if (mode < 0)
				mode = PHY_INTERFACE_MODE_NA;
			phydev->interface = mode;

			genphy_config_init(phydev);
			genphy_read_status(phydev);
			if (ds->drv->adjust_link)
				ds->drv->adjust_link(ds, port, phydev);
		}
	}
	return 0;
}

static int dsa_switch_setup_one(struct dsa_switch *ds, struct device *parent)
{
	struct dsa_switch_driver *drv = ds->drv;
	struct dsa_switch_tree *dst = ds->dst;
	struct dsa_chip_data *pd = ds->pd;
	bool valid_name_found = false;
	int index = ds->index;
	int i, ret;

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
				netdev_err(dst->master_netdev,
					   "multiple cpu ports?!\n");
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

	/* Make the built-in MII bus mask match the number of ports,
	 * switch drivers can override this later
	 */
	ds->phys_mii_mask = ds->phys_port_mask;

	/*
	 * If the CPU connects to this switch, set the switch tree
	 * tagging protocol to the preferred tagging format of this
	 * switch.
     * 该DSA实例的dsa-tag同步自cpu口所在的switch
	 */
	if (dst->cpu_switch == index) {
		switch (ds->tag_protocol) {
#ifdef CONFIG_NET_DSA_TAG_DSA
		case DSA_TAG_PROTO_DSA:
			dst->rcv = dsa_netdev_ops.rcv;
			break;
#endif
#ifdef CONFIG_NET_DSA_TAG_EDSA
		case DSA_TAG_PROTO_EDSA:
			dst->rcv = edsa_netdev_ops.rcv;
			break;
#endif
#ifdef CONFIG_NET_DSA_TAG_TRAILER
		case DSA_TAG_PROTO_TRAILER:
			dst->rcv = trailer_netdev_ops.rcv;
			break;
#endif
#ifdef CONFIG_NET_DSA_TAG_BRCM
		case DSA_TAG_PROTO_BRCM:
			dst->rcv = brcm_netdev_ops.rcv;
			break;
#endif
		case DSA_TAG_PROTO_NONE:
			break;
		default:
			ret = -ENOPROTOOPT;
			goto out;
		}

		dst->tag_protocol = ds->tag_protocol;
	}

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
	ds->slave_mii_bus = devm_mdiobus_alloc(parent);
	if (ds->slave_mii_bus == NULL) {
		ret = -ENOMEM;
		goto out;
	}
    // 然后初始化这个从mii-bus设备
	dsa_slave_mii_bus_init(ds);

    // 最后注册这个从mdio-bus设备
	ret = mdiobus_register(ds->slave_mii_bus);
	if (ret < 0)
		goto out;


	/*
	 * Create network devices for physical switch ports.
     * 为该switch的所有物理口创建对应的netdev
	 */
	for (i = 0; i < DSA_MAX_PORTS; i++) {
		if (!(ds->phys_port_mask & (1 << i)))
			continue;

        // 为每个物理端口创建对应的从netdev
		ret = dsa_slave_create(ds, parent, i, pd->port_names[i]);
		if (ret < 0) {
			netdev_err(dst->master_netdev, "[%d]: can't create dsa slave device for port %d(%s): %d\n",
				   index, i, pd->port_names[i], ret);
			ret = 0;
		}
	}

	/* Perform configuration of the CPU and DSA ports */
	ret = dsa_cpu_dsa_setup(ds, dst->master_netdev);
	if (ret < 0) {
		netdev_err(dst->master_netdev, "[%d] : can't configure CPU and DSA ports\n",
			   index);
		ret = 0;
	}

#ifdef CONFIG_NET_DSA_HWMON
	/* If the switch provides a temperature sensor,
	 * register with hardware monitoring subsystem.
	 * Treat registration error as non-fatal and ignore it.
	 */
	if (drv->get_temp) {
		const char *netname = netdev_name(dst->master_netdev);
		char hname[IFNAMSIZ + 1];
		int i, j;

		/* Create valid hwmon 'name' attribute */
		for (i = j = 0; i < IFNAMSIZ && netname[i]; i++) {
			if (isalnum(netname[i]))
				hname[j++] = netname[i];
		}
		hname[j] = '\0';
		scnprintf(ds->hwmon_name, sizeof(ds->hwmon_name), "%s_dsa%d",
			  hname, index);
		ds->hwmon_dev = hwmon_device_register_with_groups(NULL,
					ds->hwmon_name, ds, dsa_hwmon_groups);
		if (IS_ERR(ds->hwmon_dev))
			ds->hwmon_dev = NULL;
	}
#endif /* CONFIG_NET_DSA_HWMON */

	return ret;

out:
	return ret;
}

/* 根据传入的信息创建一个完整的switch实例
 * @dst     该switch实例所属的DSA实例
 * @index   该switch的序号，不级联情况下就是0
 * @parent  该switch实例所属的DSA设备
 * @bus     该switch实例使用的主mii-bus
 */
static struct dsa_switch *
dsa_switch_setup(struct dsa_switch_tree *dst, int index,
		 struct device *parent, struct device *host_dev)
{
	struct dsa_chip_data *pd = dst->pd->chip + index;
	struct dsa_switch_driver *drv;
	struct dsa_switch *ds;
	int ret;
	char *name;

	/*
	 * Probe for switch model.
     * 通过指定的mii-bus设备探测指定的switch是否存在
	 */
	drv = dsa_switch_probe(host_dev, pd->sw_addr, &name);
	if (drv == NULL) {
		netdev_err(dst->master_netdev, "[%d]: could not detect attached switch\n",
			   index);
		return ERR_PTR(-EINVAL);
	}
	netdev_info(dst->master_netdev, "[%d]: detected a %s switch\n",
		    index, name);


	/*
	 * Allocate and initialise switch state.
     * 只有成功探测到的switch才会创建对应的实例，显然实际这里申请了switch实例 + switch私有空间
	 */
	ds = devm_kzalloc(parent, sizeof(*ds) + drv->priv_size, GFP_KERNEL);
	if (ds == NULL)
		return ERR_PTR(-ENOMEM);

	ds->dst = dst;
	ds->index = index;
	ds->pd = pd;
	ds->drv = drv;
	ds->tag_protocol = drv->tag_protocol;
	ds->master_dev = host_dev;

	ret = dsa_switch_setup_one(ds, parent);
	if (ret)
		return ERR_PTR(ret);

	return ds;
}

static void dsa_switch_destroy(struct dsa_switch *ds)
{
	struct device_node *port_dn;
	struct phy_device *phydev;
	struct dsa_chip_data *cd = ds->pd;
	int port;

#ifdef CONFIG_NET_DSA_HWMON
	if (ds->hwmon_dev)
		hwmon_device_unregister(ds->hwmon_dev);
#endif

	/* Disable configuration of the CPU and DSA ports */
	for (port = 0; port < DSA_MAX_PORTS; port++) {
		if (!(dsa_is_cpu_port(ds, port) || dsa_is_dsa_port(ds, port)))
			continue;

		port_dn = cd->port_dn[port];
		if (of_phy_is_fixed_link(port_dn)) {
			phydev = of_phy_find_device(port_dn);
			if (phydev) {
				int addr = phydev->addr;

				phy_device_free(phydev);
				of_node_put(port_dn);
				fixed_phy_del(addr);
			}
		}
	}

	/* Destroy network devices for physical switch ports. */
	for (port = 0; port < DSA_MAX_PORTS; port++) {
		if (!(ds->phys_port_mask & (1 << port)))
			continue;

		if (!ds->ports[port])
			continue;

		unregister_netdev(ds->ports[port]);
		free_netdev(ds->ports[port]);
	}

	mdiobus_unregister(ds->slave_mii_bus);
}

#ifdef CONFIG_PM_SLEEP
static int dsa_switch_suspend(struct dsa_switch *ds)
{
	int i, ret = 0;

	/* Suspend slave network devices */
	for (i = 0; i < DSA_MAX_PORTS; i++) {
		if (!dsa_is_port_initialized(ds, i))
			continue;

		ret = dsa_slave_suspend(ds->ports[i]);
		if (ret)
			return ret;
	}

	if (ds->drv->suspend)
		ret = ds->drv->suspend(ds);

	return ret;
}

static int dsa_switch_resume(struct dsa_switch *ds)
{
	int i, ret = 0;

	if (ds->drv->resume)
		ret = ds->drv->resume(ds);

	if (ret)
		return ret;

	/* Resume slave network devices */
	for (i = 0; i < DSA_MAX_PORTS; i++) {
		if (!dsa_is_port_initialized(ds, i))
			continue;

		ret = dsa_slave_resume(ds->ports[i]);
		if (ret)
			return ret;
	}

	return 0;
}
#endif


/* link polling *************************************************************/
/* 用于查询link状态的工作处理函数
 * @ugly    执行该工作的工作队列
 */
static void dsa_link_poll_work(struct work_struct *ugly)
{
	struct dsa_switch_tree *dst;
	int i;

    // 获取所属的DSA实例
	dst = container_of(ugly, struct dsa_switch_tree, link_poll_work);

    // 遍历该DSA实例包含的switch(不级联就是1个)
	for (i = 0; i < dst->pd->nr_chips; i++) {
		struct dsa_switch *ds = dst->ds[i];

        // 执行每个switch的link轮寻动作
		if (ds != NULL && ds->drv->poll_link != NULL)
			ds->drv->poll_link(ds);
	}

    // 最后刷新定时器
	mod_timer(&dst->link_poll_timer, round_jiffies(jiffies + HZ));
}

/* 用于查询link状态的定时器处理函数
 * @_dst    执行该任务的DSA实例
 */
static void dsa_link_poll_timer(unsigned long _dst)
{
	struct dsa_switch_tree *dst = (void *)_dst;

    // 调度运行对应的工作队列
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
struct mii_bus *dsa_host_dev_to_mii_bus(struct device *dev)
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
EXPORT_SYMBOL_GPL(dsa_host_dev_to_mii_bus);

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
					int chip_index, int port_index,
					struct device_node *link)
{
	const __be32 *reg;
	int link_sw_addr;
	struct device_node *parent_sw;
	int len;

	parent_sw = of_get_parent(link);
	if (!parent_sw)
		return -EINVAL;

	reg = of_get_property(parent_sw, "reg", &len);
	if (!reg || (len != sizeof(*reg) * 2))
		return -EINVAL;

	/*
	 * Get the destination switch number from the second field of its 'reg'
	 * property, i.e. for "reg = <0x19 1>" sw_addr is '1'.
	 */
	link_sw_addr = be32_to_cpup(reg + 1);

	if (link_sw_addr >= pd->nr_chips)
		return -EINVAL;

	/* First time routing table allocation */
	if (!cd->rtable) {
		cd->rtable = kmalloc_array(pd->nr_chips, sizeof(s8),
					   GFP_KERNEL);
		if (!cd->rtable)
			return -ENOMEM;

		/* default to no valid uplink/downlink */
		memset(cd->rtable, -1, pd->nr_chips * sizeof(s8));
	}

	cd->rtable[link_sw_addr] = port_index;

	return 0;
}

static int dsa_of_probe_links(struct dsa_platform_data *pd,
			      struct dsa_chip_data *cd,
			      int chip_index, int port_index,
			      struct device_node *port,
			      const char *port_name)
{
	struct device_node *link;
	int link_index;
	int ret;

	for (link_index = 0;; link_index++) {
        // 获取记录了链路信息的dts节点(用于级联情况)
		link = of_parse_phandle(port, "link", link_index);
		if (!link)
			break;

        // 如果配置了switch级联情况，并且该switch端口是dsa口且配置了"link"属性，则在这里设置switch的路由表
		if (!strcmp(port_name, "dsa") && pd->nr_chips > 1) {
			ret = dsa_of_setup_routing_table(pd, cd, chip_index,
							 port_index, link);
			if (ret)
				return ret;
		}
	}
	return 0;
}

static void dsa_of_free_platform_data(struct dsa_platform_data *pd)
{
	int i;
	int port_index;

	for (i = 0; i < pd->nr_chips; i++) {
		port_index = 0;
		while (port_index < DSA_MAX_PORTS) {
			kfree(pd->chip[i].port_names[port_index]);
			port_index++;
		}
		kfree(pd->chip[i].rtable);

		/* Drop our reference to the MDIO bus device */
		if (pd->chip[i].host_dev)
			put_device(pd->chip[i].host_dev);
	}
	kfree(pd->chip);
}

// 解析DSA设备的dts节点，这过程中主要会创建DSA配置控制块和下属的switch配置控制块
static int dsa_of_probe(struct device *dev)
{
	struct device_node *np = dev->of_node;
	struct device_node *child, *mdio, *ethernet, *port;
	struct mii_bus *mdio_bus, *mdio_bus_switch;
	struct net_device *ethernet_dev;
	struct dsa_platform_data *pd;
	struct dsa_chip_data *cd;
	const char *port_name;
	int chip_index, port_index;
	const unsigned int *sw_addr, *port_reg;
	u32 eeprom_len;
	int ret;

    // 获取跟该dsa dts节点关联的mdio dts节点
	mdio = of_parse_phandle(np, "dsa,mii-bus", 0);
	if (!mdio)
		return -EINVAL;

    // 根据得到的mdio dts节点进一步获取对应的mdio总线设备
	mdio_bus = of_mdio_find_bus(mdio);
	if (!mdio_bus)
		return -EPROBE_DEFER;

    // 获取跟该dsa dts节点关联的以太网控制器dts节点
	ethernet = of_parse_phandle(np, "dsa,ethernet", 0);
	if (!ethernet) {
		ret = -EINVAL;
		goto out_put_mdio;
	}

    // 根据得到的以太网控制器dts节点进一步获取对应的以太网控制器设备
	ethernet_dev = of_find_net_device_by_node(ethernet);
	if (!ethernet_dev) {
		ret = -EPROBE_DEFER;
		goto out_put_mdio;
	}

    // 创建并初始化DSA配置控制块
	pd = kzalloc(sizeof(*pd), GFP_KERNEL);
	if (!pd) {
		ret = -ENOMEM;
		goto out_put_ethernet;
	}

	dev->platform_data = pd;                            // 将DSA配置块跟DSA设备关联
	pd->of_netdev = ethernet_dev;                       // 将DSA的宿主device设置为上面获取到的以太网控制器设备
	pd->nr_chips = of_get_available_child_count(np);    // 获取该DSA的dts节点中配置的switch数量

	if (pd->nr_chips > DSA_MAX_SWITCHES)
		pd->nr_chips = DSA_MAX_SWITCHES;

    // 创建并初始化switch配置控制块
	pd->chip = kcalloc(pd->nr_chips, sizeof(struct dsa_chip_data),
			   GFP_KERNEL);
	if (!pd->chip) {
		ret = -ENOMEM;
		goto out_free;
	}

	chip_index = -1;
	for_each_available_child_of_node(np, child) {
		chip_index++;
		cd = &pd->chip[chip_index];

		cd->of_node = child;

		/* When assigning the host device, increment its refcount 
         * 所有switch共用同一个mdio设备
         * */
		cd->host_dev = get_device(&mdio_bus->dev);

        // 获取该switch的地址序号
		sw_addr = of_get_property(child, "reg", NULL);
		if (!sw_addr)
			continue;

		cd->sw_addr = be32_to_cpup(sw_addr);
		if (cd->sw_addr >= PHY_MAX_ADDR)
			continue;

		if (!of_property_read_u32(child, "eeprom-length", &eeprom_len))
			cd->eeprom_len = eeprom_len;

		mdio = of_parse_phandle(child, "mii-bus", 0);
		if (mdio) {
			mdio_bus_switch = of_mdio_find_bus(mdio);
			if (!mdio_bus_switch) {
				ret = -EPROBE_DEFER;
				goto out_free_chip;
			}

			/* Drop the mdio_bus device ref, replacing the host
			 * device with the mdio_bus_switch device, keeping
			 * the refcount from of_mdio_find_bus() above.
			 */
			put_device(cd->host_dev);
			cd->host_dev = &mdio_bus_switch->dev;
		}

		for_each_available_child_of_node(child, port) {
            // 获取该switch每个端口的物理地址序号
			port_reg = of_get_property(port, "reg", NULL);
			if (!port_reg)
				continue;

			port_index = be32_to_cpup(port_reg);
			if (port_index >= DSA_MAX_PORTS)
				break;

            // 获取该switch每个端口名
			port_name = of_get_property(port, "label", NULL);
			if (!port_name)
				continue;

			cd->port_dn[port_index] = port;

			cd->port_names[port_index] = kstrdup(port_name,
					GFP_KERNEL);
			if (!cd->port_names[port_index]) {
				ret = -ENOMEM;
				goto out_free_chip;
			}

			ret = dsa_of_probe_links(pd, cd, chip_index,
						 port_index, port, port_name);
			if (ret)
				goto out_free_chip;

		}
	}

	/* The individual chips hold their own refcount on the mdio bus,
	 * so drop ours */
	put_device(&mdio_bus->dev);

	return 0;

out_free_chip:
	dsa_of_free_platform_data(pd);
out_free:
	kfree(pd);
	dev->platform_data = NULL;
out_put_ethernet:
	put_device(&ethernet_dev->dev);
out_put_mdio:
	put_device(&mdio_bus->dev);
	return ret;
}

static void dsa_of_remove(struct device *dev)
{
	struct dsa_platform_data *pd = dev->platform_data;

	if (!dev->of_node)
		return;

	dsa_of_free_platform_data(pd);
	put_device(&pd->of_netdev->dev);
	kfree(pd);
}
#else
static inline int dsa_of_probe(struct device *dev)
{
	return 0;
}

static inline void dsa_of_remove(struct device *dev)
{
}
#endif

static int dsa_setup_dst(struct dsa_switch_tree *dst, struct net_device *dev,
			 struct device *parent, struct dsa_platform_data *pd)
{
	int i;
	unsigned configured = 0;

	dst->pd = pd;
	dst->master_netdev = dev;
	dst->cpu_switch = -1;
	dst->cpu_port = -1;

    // 创建配置的每个switch实例(不级联就是1个)
	for (i = 0; i < pd->nr_chips; i++) {
		struct dsa_switch *ds;

        // 创建完整的switch实例
		ds = dsa_switch_setup(dst, i, parent, pd->chip[i].host_dev);
		if (IS_ERR(ds)) {
			netdev_err(dev, "[%d]: couldn't create dsa switch instance (error %ld)\n",
				   i, PTR_ERR(ds));
			continue;
		}

		dst->ds[i] = ds;
        // 如果驱动支持就开启该DSA实例的定时轮寻link功能
		if (ds->drv->poll_link != NULL)
			dst->link_poll_needed = 1;

		++configured;
	}

	/*
	 * If no switch was found, exit cleanly
	 */
	if (!configured)
		return -EPROBE_DEFER;

	/*
	 * If we use a tagging format that doesn't have an ethertype
	 * field, make sure that all packets from this point on get
	 * sent to the tag format's receive function.
	 */
	wmb();
    // 将创建的DSA实例跟宿主netdev绑定
	dev->dsa_ptr = (void *)dst;

    // 如果该DSA实例使能了定时轮寻link功能，则在最后注册对应的定时器
	if (dst->link_poll_needed) {
        // 初始化用于轮寻link状态的工作队列
		INIT_WORK(&dst->link_poll_work, dsa_link_poll_work);
        // 初始化并开启用于轮寻link状态的定时器
		init_timer(&dst->link_poll_timer);
		dst->link_poll_timer.data = (unsigned long)dst;
		dst->link_poll_timer.function = dsa_link_poll_timer;
		dst->link_poll_timer.expires = round_jiffies(jiffies + HZ); // 这里设为1s超时
		add_timer(&dst->link_poll_timer);
	}

	return 0;
}

/* dsa驱动探测到匹配的设备后的回调函数，实际就是初始化探测到的dsa设备
 * @pdev    指向匹配到的DSA设备
 */
static int dsa_probe(struct platform_device *pdev)
{
	struct dsa_platform_data *pd = pdev->dev.platform_data;
	struct net_device *dev;
	struct dsa_switch_tree *dst;
	int ret;

	pr_notice_once("Distributed Switch Architecture driver version %s\n",
		       dsa_driver_version);

    // 如果DSA设备存在对应的dts节点，则对其进行解析，这过程中主要会创建DSA配置控制块和下属的switch配置控制块
	if (pdev->dev.of_node) {
		ret = dsa_of_probe(&pdev->dev);
		if (ret)
			return ret;

		pd = pdev->dev.platform_data;
	}

	if (pd == NULL || (pd->netdev == NULL && pd->of_netdev == NULL))
		return -EINVAL;

    // 获取宿主device的父结构netdev
	if (pd->of_netdev) {
		dev = pd->of_netdev;
		dev_hold(dev);
	} else {
		dev = dev_to_net_device(pd->netdev);
	}
	if (dev == NULL) {
		ret = -EPROBE_DEFER;
		goto out;
	}

	if (dev->dsa_ptr != NULL) {
		dev_put(dev);
		ret = -EEXIST;
		goto out;
	}

    // 创建并初始化DSA实例
	dst = devm_kzalloc(&pdev->dev, sizeof(*dst), GFP_KERNEL);
	if (dst == NULL) {
		dev_put(dev);
		ret = -ENOMEM;
		goto out;
	}

    // 将DSA实例作为私有数据记录到该DSA设备底层device的私有数据块中 
	platform_set_drvdata(pdev, dst);

	ret = dsa_setup_dst(dst, dev, &pdev->dev, pd);
	if (ret)
		goto out;

	return 0;

out:
	dsa_of_remove(&pdev->dev);

	return ret;
}

static void dsa_remove_dst(struct dsa_switch_tree *dst)
{
	int i;

	if (dst->link_poll_needed)
		del_timer_sync(&dst->link_poll_timer);

	flush_work(&dst->link_poll_work);

	for (i = 0; i < dst->pd->nr_chips; i++) {
		struct dsa_switch *ds = dst->ds[i];

		if (ds)
			dsa_switch_destroy(ds);
	}
}

// DSA驱动的remove回调函数
static int dsa_remove(struct platform_device *pdev)
{
	struct dsa_switch_tree *dst = platform_get_drvdata(pdev);

	dsa_remove_dst(dst);
	dsa_of_remove(&pdev->dev);

	return 0;
}

static void dsa_shutdown(struct platform_device *pdev)
{
}

// 设备收到dsa报文的处理入口
static int dsa_switch_rcv(struct sk_buff *skb, struct net_device *dev,
			  struct packet_type *pt, struct net_device *orig_dev)
{
    /* 这个收到dsa报文的设备大概率寄生了一个dsa实例的
     * 这里实际就是找到该dsa实例，进而调用rcv方法处理收到的dsa报文
     */
	struct dsa_switch_tree *dst = dev->dsa_ptr;

	if (unlikely(dst == NULL)) {
		kfree_skb(skb);
		return 0;
	}

	return dst->rcv(skb, dev, pt, orig_dev);
}

static struct packet_type dsa_pack_type __read_mostly = {
	.type	= cpu_to_be16(ETH_P_XDSA),
	.func	= dsa_switch_rcv,
};

// 定义了一个为DSA驱动服务的通知单元，专门用于监听网络设备事件
static struct notifier_block dsa_netdevice_nb __read_mostly = {
	.notifier_call	= dsa_slave_netdevice_event,
};

#ifdef CONFIG_PM_SLEEP
static int dsa_suspend(struct device *d)
{
	struct platform_device *pdev = to_platform_device(d);
	struct dsa_switch_tree *dst = platform_get_drvdata(pdev);
	int i, ret = 0;

	for (i = 0; i < dst->pd->nr_chips; i++) {
		struct dsa_switch *ds = dst->ds[i];

		if (ds != NULL)
			ret = dsa_switch_suspend(ds);
	}

	return ret;
}

static int dsa_resume(struct device *d)
{
	struct platform_device *pdev = to_platform_device(d);
	struct dsa_switch_tree *dst = platform_get_drvdata(pdev);
	int i, ret = 0;

	for (i = 0; i < dst->pd->nr_chips; i++) {
		struct dsa_switch *ds = dst->ds[i];

		if (ds != NULL)
			ret = dsa_switch_resume(ds);
	}

	return ret;
}
#endif

static SIMPLE_DEV_PM_OPS(dsa_pm_ops, dsa_suspend, dsa_resume);

/* 定义了一张通用DSA驱动支持的device描述列表，只有符合该表中描述的device才是匹配的device
 * 
 * 备注： 3.14.38版本中DSA驱动只支持marvell
 *        4.4.52版本中增加了对broadcom芯片的支持
 */
static const struct of_device_id dsa_of_match_table[] = {
	{ .compatible = "brcm,bcm7445-switch-v4.0" },
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
		.of_match_table = dsa_of_match_table,
		.pm	= &dsa_pm_ops,
	},
};

// DSA驱动初始化入口
static int __init dsa_init_module(void)
{
	int rc;

    // 将为DSA驱动服务的通知单元注册到网络设备通知链中
	register_netdevice_notifier(&dsa_netdevice_nb);

    // 将DSA驱动注册到platform总线中
	rc = platform_driver_register(&dsa_driver);
	if (rc)
		return rc;

    // 注册DSA报文接收处理方法到内核
	dev_add_pack(&dsa_pack_type);

	return 0;
}
module_init(dsa_init_module);

// DSA驱动卸载入口
static void __exit dsa_cleanup_module(void)
{
	unregister_netdevice_notifier(&dsa_netdevice_nb);
	dev_remove_pack(&dsa_pack_type);
	platform_driver_unregister(&dsa_driver);
}
module_exit(dsa_cleanup_module);

MODULE_AUTHOR("Lennert Buytenhek <buytenh@wantstofly.org>");
MODULE_DESCRIPTION("Driver for Distributed Switch Architecture switch chips");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:dsa");
