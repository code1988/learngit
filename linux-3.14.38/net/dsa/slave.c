/*
 * net/dsa/slave.c - Slave device handling
 * Copyright (c) 2008-2009 Marvell Semiconductor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/phy.h>
#include "dsa_priv.h"

/* slave mii_bus handling ***************************************************/
/* 从mii-bus设备读操作的接口
 * @addr    要读的phy地址(就是端口号)
 * @reg     要读的phy地址上的寄存器序号
 *
 * 备注：调用本接口时需要持有对应的锁mdio_lock
 *
 * @返回值  成功返回实际值，失败返回0xffff
 */
static int dsa_slave_phy_read(struct mii_bus *bus, int addr, int reg)
{
	struct dsa_switch *ds = bus->priv;

    // 只有普通的物理口才能进行读操作
	if (ds->phys_port_mask & (1 << addr))
		return ds->drv->phy_read(ds, addr, reg);

	return 0xffff;
}

/* 从mii-bus设备写操作的接口
 * @addr    要写的phy地址(就是端口号)
 * @reg     要写的phy地址上的寄存器序号
 *
 * 备注：调用本接口时需要持有对应的锁mdio_lock
 *
 * @返回值  成功返回实际值，失败返回0
 */
static int dsa_slave_phy_write(struct mii_bus *bus, int addr, int reg, u16 val)
{
	struct dsa_switch *ds = bus->priv;

    // 只有普通的物理口才能进行写操作
	if (ds->phys_port_mask & (1 << addr))
		return ds->drv->phy_write(ds, addr, reg, val);

	return 0;
}

// 初始化指定switch的从mii-bus设备
void dsa_slave_mii_bus_init(struct dsa_switch *ds)
{
	ds->slave_mii_bus->priv = (void *)ds;       // 从mii-bus设备的私有空间指向所属的switch实例
	ds->slave_mii_bus->name = "dsa slave smi";
	ds->slave_mii_bus->read = dsa_slave_phy_read;
	ds->slave_mii_bus->write = dsa_slave_phy_write;
	snprintf(ds->slave_mii_bus->id, MII_BUS_ID_SIZE, "dsa-%d:%.2x",
			ds->index, ds->pd->sw_addr);
	ds->slave_mii_bus->parent = &ds->master_mii_bus->dev;   // 从mii-bus设备的宿主设备就是主mii-bus设备
}


/* slave device handling ****************************************************/
// 对指定的从netdev在注册到内核期间执行初始化
static int dsa_slave_init(struct net_device *dev)
{
    // 获取该从netdev对应的物理端口实例
	struct dsa_slave_priv *p = netdev_priv(dev);

    // 记录该从netdev的宿主netdev的接口序号
	dev->iflink = p->parent->dst->master_netdev->ifindex;

	return 0;
}

static int dsa_slave_open(struct net_device *dev)
{
	struct dsa_slave_priv *p = netdev_priv(dev);
	struct net_device *master = p->parent->dst->master_netdev;
	int err;

	if (!(master->flags & IFF_UP))
		return -ENETDOWN;

	if (!ether_addr_equal(dev->dev_addr, master->dev_addr)) {
		err = dev_uc_add(master, dev->dev_addr);
		if (err < 0)
			goto out;
	}

	if (dev->flags & IFF_ALLMULTI) {
		err = dev_set_allmulti(master, 1);
		if (err < 0)
			goto del_unicast;
	}
	if (dev->flags & IFF_PROMISC) {
		err = dev_set_promiscuity(master, 1);
		if (err < 0)
			goto clear_allmulti;
	}

	return 0;

clear_allmulti:
	if (dev->flags & IFF_ALLMULTI)
		dev_set_allmulti(master, -1);
del_unicast:
	if (!ether_addr_equal(dev->dev_addr, master->dev_addr))
		dev_uc_del(master, dev->dev_addr);
out:
	return err;
}

static int dsa_slave_close(struct net_device *dev)
{
	struct dsa_slave_priv *p = netdev_priv(dev);
	struct net_device *master = p->parent->dst->master_netdev;

	dev_mc_unsync(master, dev);
	dev_uc_unsync(master, dev);
	if (dev->flags & IFF_ALLMULTI)
		dev_set_allmulti(master, -1);
	if (dev->flags & IFF_PROMISC)
		dev_set_promiscuity(master, -1);

	if (!ether_addr_equal(dev->dev_addr, master->dev_addr))
		dev_uc_del(master, dev->dev_addr);

	return 0;
}

static void dsa_slave_change_rx_flags(struct net_device *dev, int change)
{
	struct dsa_slave_priv *p = netdev_priv(dev);
	struct net_device *master = p->parent->dst->master_netdev;

	if (change & IFF_ALLMULTI)
		dev_set_allmulti(master, dev->flags & IFF_ALLMULTI ? 1 : -1);
	if (change & IFF_PROMISC)
		dev_set_promiscuity(master, dev->flags & IFF_PROMISC ? 1 : -1);
}

static void dsa_slave_set_rx_mode(struct net_device *dev)
{
	struct dsa_slave_priv *p = netdev_priv(dev);
	struct net_device *master = p->parent->dst->master_netdev;

	dev_mc_sync(master, dev);
	dev_uc_sync(master, dev);
}

static int dsa_slave_set_mac_address(struct net_device *dev, void *a)
{
	struct dsa_slave_priv *p = netdev_priv(dev);
	struct net_device *master = p->parent->dst->master_netdev;
	struct sockaddr *addr = a;
	int err;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	if (!(dev->flags & IFF_UP))
		goto out;

	if (!ether_addr_equal(addr->sa_data, master->dev_addr)) {
		err = dev_uc_add(master, addr->sa_data);
		if (err < 0)
			return err;
	}

	if (!ether_addr_equal(dev->dev_addr, master->dev_addr))
		dev_uc_del(master, dev->dev_addr);

out:
	ether_addr_copy(dev->dev_addr, addr->sa_data);

	return 0;
}

// 从netdev的ioctl接口
static int dsa_slave_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct dsa_slave_priv *p = netdev_priv(dev);

	if (p->phy != NULL)
		return phy_mii_ioctl(p->phy, ifr, cmd);

	return -EOPNOTSUPP;
}


/* ethtool operations *******************************************************/
static int
dsa_slave_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct dsa_slave_priv *p = netdev_priv(dev);
	int err;

	err = -EOPNOTSUPP;
	if (p->phy != NULL) {
		err = phy_read_status(p->phy);
		if (err == 0)
			err = phy_ethtool_gset(p->phy, cmd);
	}

	return err;
}

static int
dsa_slave_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct dsa_slave_priv *p = netdev_priv(dev);

	if (p->phy != NULL)
		return phy_ethtool_sset(p->phy, cmd);

	return -EOPNOTSUPP;
}

static void dsa_slave_get_drvinfo(struct net_device *dev,
				  struct ethtool_drvinfo *drvinfo)
{
	strlcpy(drvinfo->driver, "dsa", sizeof(drvinfo->driver));
	strlcpy(drvinfo->version, dsa_driver_version, sizeof(drvinfo->version));
	strlcpy(drvinfo->fw_version, "N/A", sizeof(drvinfo->fw_version));
	strlcpy(drvinfo->bus_info, "platform", sizeof(drvinfo->bus_info));
}

static int dsa_slave_nway_reset(struct net_device *dev)
{
	struct dsa_slave_priv *p = netdev_priv(dev);

	if (p->phy != NULL)
		return genphy_restart_aneg(p->phy);

	return -EOPNOTSUPP;
}

static u32 dsa_slave_get_link(struct net_device *dev)
{
	struct dsa_slave_priv *p = netdev_priv(dev);

	if (p->phy != NULL) {
		genphy_update_link(p->phy);
		return p->phy->link;
	}

	return -EOPNOTSUPP;
}

static void dsa_slave_get_strings(struct net_device *dev,
				  uint32_t stringset, uint8_t *data)
{
	struct dsa_slave_priv *p = netdev_priv(dev);
	struct dsa_switch *ds = p->parent;

	if (stringset == ETH_SS_STATS) {
		int len = ETH_GSTRING_LEN;

		strncpy(data, "tx_packets", len);
		strncpy(data + len, "tx_bytes", len);
		strncpy(data + 2 * len, "rx_packets", len);
		strncpy(data + 3 * len, "rx_bytes", len);
		if (ds->drv->get_strings != NULL)
			ds->drv->get_strings(ds, p->port, data + 4 * len);
	}
}

static void dsa_slave_get_ethtool_stats(struct net_device *dev,
					struct ethtool_stats *stats,
					uint64_t *data)
{
	struct dsa_slave_priv *p = netdev_priv(dev);
	struct dsa_switch *ds = p->parent;

	data[0] = p->dev->stats.tx_packets;
	data[1] = p->dev->stats.tx_bytes;
	data[2] = p->dev->stats.rx_packets;
	data[3] = p->dev->stats.rx_bytes;
	if (ds->drv->get_ethtool_stats != NULL)
		ds->drv->get_ethtool_stats(ds, p->port, data + 4);
}

static int dsa_slave_get_sset_count(struct net_device *dev, int sset)
{
	struct dsa_slave_priv *p = netdev_priv(dev);
	struct dsa_switch *ds = p->parent;

	if (sset == ETH_SS_STATS) {
		int count;

		count = 4;
		if (ds->drv->get_sset_count != NULL)
			count += ds->drv->get_sset_count(ds);

		return count;
	}

	return -EOPNOTSUPP;
}

// 定义了从DSA设备用于ethtool命令的操作集合
static const struct ethtool_ops dsa_slave_ethtool_ops = {
	.get_settings		= dsa_slave_get_settings,
	.set_settings		= dsa_slave_set_settings,
	.get_drvinfo		= dsa_slave_get_drvinfo,
	.nway_reset		= dsa_slave_nway_reset,
	.get_link		= dsa_slave_get_link,
	.get_strings		= dsa_slave_get_strings,
	.get_ethtool_stats	= dsa_slave_get_ethtool_stats,
	.get_sset_count		= dsa_slave_get_sset_count,
};

#ifdef CONFIG_NET_DSA_TAG_DSA
// 定义了dsa类型的从端口netdev操作集合
static const struct net_device_ops dsa_netdev_ops = {
	.ndo_init		= dsa_slave_init,
	.ndo_open	 	= dsa_slave_open,
	.ndo_stop		= dsa_slave_close,
	.ndo_start_xmit		= dsa_xmit,
	.ndo_change_rx_flags	= dsa_slave_change_rx_flags,
	.ndo_set_rx_mode	= dsa_slave_set_rx_mode,
	.ndo_set_mac_address	= dsa_slave_set_mac_address,
	.ndo_do_ioctl		= dsa_slave_ioctl,
};
#endif
#ifdef CONFIG_NET_DSA_TAG_EDSA
static const struct net_device_ops edsa_netdev_ops = {
	.ndo_init		= dsa_slave_init,
	.ndo_open	 	= dsa_slave_open,
	.ndo_stop		= dsa_slave_close,
	.ndo_start_xmit		= edsa_xmit,
	.ndo_change_rx_flags	= dsa_slave_change_rx_flags,
	.ndo_set_rx_mode	= dsa_slave_set_rx_mode,
	.ndo_set_mac_address	= dsa_slave_set_mac_address,
	.ndo_do_ioctl		= dsa_slave_ioctl,
};
#endif
#ifdef CONFIG_NET_DSA_TAG_TRAILER
static const struct net_device_ops trailer_netdev_ops = {
	.ndo_init		= dsa_slave_init,
	.ndo_open	 	= dsa_slave_open,
	.ndo_stop		= dsa_slave_close,
	.ndo_start_xmit		= trailer_xmit,
	.ndo_change_rx_flags	= dsa_slave_change_rx_flags,
	.ndo_set_rx_mode	= dsa_slave_set_rx_mode,
	.ndo_set_mac_address	= dsa_slave_set_mac_address,
	.ndo_do_ioctl		= dsa_slave_ioctl,
};
#endif

/* slave device setup *******************************************************/
/* 为指定端口创建对应的从netdev
 * @ds      switch实例
 * @parent  该switch实例所属的DSA设备
 * @port    物理端口号
 * @name    端口名
 */
struct net_device *
dsa_slave_create(struct dsa_switch *ds, struct device *parent,
		 int port, char *name)
{
	struct net_device *master = ds->dst->master_netdev;
	struct net_device *slave_dev;
	struct dsa_slave_priv *p;
	int ret;

	slave_dev = alloc_netdev(sizeof(struct dsa_slave_priv),
				 name, ether_setup);
	if (slave_dev == NULL)
		return slave_dev;

	slave_dev->features = master->vlan_features;
	SET_ETHTOOL_OPS(slave_dev, &dsa_slave_ethtool_ops);
	eth_hw_addr_inherit(slave_dev, master); // 该端口netdev硬件地址从宿主设备继承
	slave_dev->tx_queue_len = 0;

    // 根据所属DSA实例使用的dsa-tag类型注册对应的netdev驱动
	switch (ds->dst->tag_protocol) {
#ifdef CONFIG_NET_DSA_TAG_DSA
	case htons(ETH_P_DSA):
		slave_dev->netdev_ops = &dsa_netdev_ops;
		break;
#endif
#ifdef CONFIG_NET_DSA_TAG_EDSA
	case htons(ETH_P_EDSA):
		slave_dev->netdev_ops = &edsa_netdev_ops;
		break;
#endif
#ifdef CONFIG_NET_DSA_TAG_TRAILER
	case htons(ETH_P_TRAILER):
		slave_dev->netdev_ops = &trailer_netdev_ops;
		break;
#endif
	default:
		BUG();
	}

    // 设置该端口netdev的父device为DSA设备
	SET_NETDEV_DEV(slave_dev, parent);
	slave_dev->vlan_features = master->vlan_features;   // 该端口netdev->vlan_features从宿主设备继承

	p = netdev_priv(slave_dev);
	p->dev = slave_dev;
	p->parent = ds;
	p->port = port;
	p->phy = ds->slave_mii_bus->phy_map[port];

    // 注册该端口netdev到内核，期间会执行dsa_slave_init
	ret = register_netdev(slave_dev);
	if (ret) {
		printk(KERN_ERR "%s: error %d registering interface %s\n",
				master->name, ret, slave_dev->name);
		free_netdev(slave_dev);
		return NULL;
	}

    // 刚创建的netdev无载波
	netif_carrier_off(slave_dev);

    // 将该端口netdev和对应的phy绑定
	if (p->phy != NULL) {
		phy_attach(slave_dev, dev_name(&p->phy->dev),
			   PHY_INTERFACE_MODE_GMII);

		p->phy->autoneg = AUTONEG_ENABLE;
		p->phy->speed = 0;
		p->phy->duplex = 0;
		p->phy->advertising = p->phy->supported | ADVERTISED_Autoneg;
		phy_start_aneg(p->phy);
	}

	return slave_dev;
}
