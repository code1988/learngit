/*
 * include/net/dsa.h - Driver for Distributed Switch Architecture switch chips
 * Copyright (c) 2008-2009 Marvell Semiconductor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __LINUX_NET_DSA_H
#define __LINUX_NET_DSA_H

#include <linux/if_ether.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/of.h>
#include <linux/phy.h>
#include <linux/phy_fixed.h>
#include <linux/ethtool.h>

enum dsa_tag_protocol {
	DSA_TAG_PROTO_NONE = 0,
	DSA_TAG_PROTO_DSA,
	DSA_TAG_PROTO_TRAILER,
	DSA_TAG_PROTO_EDSA,
	DSA_TAG_PROTO_BRCM,
};

#define DSA_MAX_SWITCHES	4   // 每个DSA实例支持的最大switch数量
#define DSA_MAX_PORTS		12  // 每个DSA实例支持的最大端口数

// 用于描述switch的配置信息
struct dsa_chip_data {
	/*
	 * How to access the switch configuration registers.
	 */
	struct device	*host_dev;  // 指向该switch使用的mii设备
	int		sw_addr;            // switch地址序号，来自"switch"dts节点中的"reg"属性，最大不超过PHY_MAX_ADDR

	/* set to size of eeprom if supported by the switch */
	int		eeprom_len;

	/* Device tree node pointer for this specific switch chip
	 * used during switch setup in case additional properties
	 * and resources needs to be used
     * 指向该switch实例对应的dts节点
	 */
	struct device_node *of_node;

	/*
	 * The names of the switch's ports.  Use "cpu" to
	 * designate the switch port that the cpu is connected to,
	 * "dsa" to indicate that this port is a DSA link to
	 * another switch, NULL to indicate the port is unused,
	 * or any other string to indicate this is a physical port.
     * 记录了switch每个端口的名字
	 */
	char		*port_names[DSA_MAX_PORTS];
	struct device_node *port_dn[DSA_MAX_PORTS];

	/*
	 * An array (with nr_chips elements) of which element [a]
	 * indicates which port on this switch should be used to
	 * send packets to that are destined for switch a.  Can be
	 * NULL if there is only one switch chip.
     * 级联switch情况下的switch端口路由表，不级联switch情况下该字段为NULL
	 */
	s8		*rtable;
};

// 用于描述DSA实例的配置信息
struct dsa_platform_data {
	/*
	 * Reference to a Linux network interface that connects
	 * to the root switch chip of the tree.
     * 指向该DSA实例的宿主device
	 */
	struct device	*netdev;
	struct net_device *of_netdev;   // 指向该DSA实例的宿主网络接口

	/*
	 * Info structs describing each of the switch chips
	 * connected via this network interface.
	 */
	int		nr_chips;               // 该DSA实例实际管理的级联switch数量
	struct dsa_chip_data	*chip;  // 这些级联switch配置信息的描述集合
};

struct packet_type;

// DSA实例，被记录在DSA设备device->p->driver_data，并和跟宿主netdev->dsa_ptr关联
struct dsa_switch_tree {
	/*
	 * Configuration data for the platform device that owns
	 * this dsa switch tree instance.
     * 指向该DSA实例的配置信息
	 */
	struct dsa_platform_data	*pd;

	/*
	 * Reference to network device to use, and which tagging
	 * protocol to use.
     * 指向该DSA实例的宿主netdev
	 */
	struct net_device	*master_netdev;
	int			(*rcv)(struct sk_buff *skb,
				       struct net_device *dev,
				       struct packet_type *pt,
				       struct net_device *orig_dev);
	enum dsa_tag_protocol	tag_protocol;   // 记录了该DSA实例使用的dsa-tag类型(原始类型就是ETH_P_DSA)，同步自该DSA实例的CPU口所属switch

	/*
	 * The switch and port to which the CPU is attached.
	 */
	s8			cpu_switch;                 // CPU口所属的switch序号
	s8			cpu_port;                   // CPU口在所属switch上对应的端口序号

	/*
	 * Link state polling.
	 */
	int			link_poll_needed;           // 标识该DSA是否开启定时轮寻link状态
	struct work_struct	link_poll_work;     // 轮寻link状态的工作队列
	struct timer_list	link_poll_timer;    // 轮寻link状态的定时器

	/*
	 * Data for the individual switch chips.
     * 记录了该DSA实例包含的所有switchd
	 */
	struct dsa_switch	*ds[DSA_MAX_SWITCHES];
};

// switch实例
struct dsa_switch {
	/*
	 * Parent switch tree, and switch index.
     * 指向该switch所属的DSA实例
	 */
	struct dsa_switch_tree	*dst;
	int			index;              // 分配给该switch的序号

	/*
	 * Tagging protocol understood by this switch
	 */
	enum dsa_tag_protocol	tag_protocol;

	/*
	 * Configuration data for this switch.
     * 指向该switch的配置信息
	 */
	struct dsa_chip_data	*pd;

	/*
	 * The used switch driver.
     * 指向该switch使用的驱动
	 */
	struct dsa_switch_driver	*drv;

	/*
	 * Reference to host device to use.
     * 指向该switch使用的主mii-bus设备
	 */
	struct device		*master_dev;

#ifdef CONFIG_NET_DSA_HWMON
	/*
	 * Hardware monitoring information
	 */
	char			hwmon_name[IFNAMSIZ + 8];
	struct device		*hwmon_dev;
#endif

	/*
	 * Slave mii_bus and devices for the individual ports.
	 */
	u32			dsa_port_mask;                  // 开启了dsa功能的端口集合
	u32			phys_port_mask;                 // 物理口集合
	u32			phys_mii_mask;
	struct mii_bus		*slave_mii_bus;         // 指向该switch使用的从mii-bus设备(创建该switch时创建，执行读写操作最终还是要通过主mii-bus来完成)
	struct net_device	*ports[DSA_MAX_PORTS];  // 该switch包含的所有端口对应的netdev集合
};

// 判断指定switch上的指定端口是否是CPU口
static inline bool dsa_is_cpu_port(struct dsa_switch *ds, int p)
{
    // 是否是CPU口需要同时满足2个条件：该switch是否是CPU口所在switch；该端口是否是CPU口所在switch上对应端口
	return !!(ds->index == ds->dst->cpu_switch && p == ds->dst->cpu_port);
}

static inline bool dsa_is_dsa_port(struct dsa_switch *ds, int p)
{
	return !!((ds->dsa_port_mask) & (1 << p));
}

static inline bool dsa_is_port_initialized(struct dsa_switch *ds, int p)
{
	return ds->phys_port_mask & (1 << p) && ds->ports[p];
}

// 获取指定switch上的上行口
static inline u8 dsa_upstream_port(struct dsa_switch *ds)
{
	struct dsa_switch_tree *dst = ds->dst;

	/*
	 * If this is the root switch (i.e. the switch that connects
	 * to the CPU), return the cpu port number on this switch.
	 * Else return the (DSA) port number that connects to the
	 * switch that is one hop closer to the cpu.
     * 上行口的定义：
     *          如果是root switch就是CPU口;
     *          否则就是该级联switch上更靠近CPU的DSA口
	 */
	if (dst->cpu_switch == ds->index)
		return dst->cpu_port;
	else
		return ds->pd->rtable[dst->cpu_switch];
}

struct switchdev_trans;
struct switchdev_obj;
struct switchdev_obj_port_fdb;
struct switchdev_obj_port_vlan;

// switch驱动
struct dsa_switch_driver {
	struct list_head	list;

	enum dsa_tag_protocol	tag_protocol;   // 该switch使用的DSA-tag类型
	int			priv_size;                  // 私有空间大小

	/*
	 * Probing and setup.
	 */
	char	*(*probe)(struct device *host_dev, int sw_addr);    // 通过mii-bus设备发起对指定switch的探测操作
	int	(*setup)(struct dsa_switch *ds);                        // 初始化探测到的switch
	int	(*set_addr)(struct dsa_switch *ds, u8 *addr);
	u32	(*get_phy_flags)(struct dsa_switch *ds, int port);

	/*
	 * Access to the switch's PHY registers.
	 */
	int	(*phy_read)(struct dsa_switch *ds, int port, int regnum);// 读指定端口(即phy)上的指定寄存器 
	int	(*phy_write)(struct dsa_switch *ds, int port,            // 写指定端口(即phy)上的指定寄存器
			     int regnum, u16 val);

	/*
	 * Link state polling and IRQ handling.
	 */
	void	(*poll_link)(struct dsa_switch *ds);    // 查询该switch所有端口的链路状态   ->mv88e6xxx_poll_link

	/*
	 * Link state adjustment (called from libphy)
	 */
	void	(*adjust_link)(struct dsa_switch *ds, int port,
				struct phy_device *phydev);
	void	(*fixed_link_update)(struct dsa_switch *ds, int port,
				struct fixed_phy_status *st);

	/*
	 * ethtool hardware statistics.
	 */
	void	(*get_strings)(struct dsa_switch *ds, int port, uint8_t *data); // 获取该switch指定端口的统计项名
	void	(*get_ethtool_stats)(struct dsa_switch *ds,                                                         
				     int port, uint64_t *data);                             // 获取该switch指定端口的统计信息值
	int	(*get_sset_count)(struct dsa_switch *ds);                           // 获取该switch的统计项数量

	/*
	 * ethtool Wake-on-LAN
	 */
	void	(*get_wol)(struct dsa_switch *ds, int port,
			   struct ethtool_wolinfo *w);
	int	(*set_wol)(struct dsa_switch *ds, int port,
			   struct ethtool_wolinfo *w);

	/*
	 * Suspend and resume
	 */
	int	(*suspend)(struct dsa_switch *ds);
	int	(*resume)(struct dsa_switch *ds);

	/*
	 * Port enable/disable
	 */
	int	(*port_enable)(struct dsa_switch *ds, int port,
			       struct phy_device *phy);
	void	(*port_disable)(struct dsa_switch *ds, int port,
				struct phy_device *phy);

	/*
	 * EEE setttings
	 */
	int	(*set_eee)(struct dsa_switch *ds, int port,
			   struct phy_device *phydev,
			   struct ethtool_eee *e);
	int	(*get_eee)(struct dsa_switch *ds, int port,
			   struct ethtool_eee *e);

#ifdef CONFIG_NET_DSA_HWMON
	/* Hardware monitoring */
	int	(*get_temp)(struct dsa_switch *ds, int *temp);
	int	(*get_temp_limit)(struct dsa_switch *ds, int *temp);
	int	(*set_temp_limit)(struct dsa_switch *ds, int temp);
	int	(*get_temp_alarm)(struct dsa_switch *ds, bool *alarm);
#endif

	/* EEPROM access */
	int	(*get_eeprom_len)(struct dsa_switch *ds);
	int	(*get_eeprom)(struct dsa_switch *ds,
			      struct ethtool_eeprom *eeprom, u8 *data);
	int	(*set_eeprom)(struct dsa_switch *ds,
			      struct ethtool_eeprom *eeprom, u8 *data);

	/*
	 * Register access.
	 */
	int	(*get_regs_len)(struct dsa_switch *ds, int port);
	void	(*get_regs)(struct dsa_switch *ds, int port,
			    struct ethtool_regs *regs, void *p);

	/*
	 * Bridge integration
	 */
	int	(*port_join_bridge)(struct dsa_switch *ds, int port,
				    u32 br_port_mask);
	int	(*port_leave_bridge)(struct dsa_switch *ds, int port,
				     u32 br_port_mask);
	int	(*port_stp_update)(struct dsa_switch *ds, int port,
				   u8 state);

	/*
	 * VLAN support
	 */
	int	(*port_vlan_prepare)(struct dsa_switch *ds, int port,
				     const struct switchdev_obj_port_vlan *vlan,
				     struct switchdev_trans *trans);
	int	(*port_vlan_add)(struct dsa_switch *ds, int port,
				 const struct switchdev_obj_port_vlan *vlan,
				 struct switchdev_trans *trans);
	int	(*port_vlan_del)(struct dsa_switch *ds, int port,
				 const struct switchdev_obj_port_vlan *vlan);
	int	(*port_pvid_get)(struct dsa_switch *ds, int port, u16 *pvid);
	int	(*vlan_getnext)(struct dsa_switch *ds, u16 *vid,
				unsigned long *ports, unsigned long *untagged);

	/*
	 * Forwarding database
	 */
	int	(*port_fdb_prepare)(struct dsa_switch *ds, int port,
				    const struct switchdev_obj_port_fdb *fdb,
				    struct switchdev_trans *trans);
	int	(*port_fdb_add)(struct dsa_switch *ds, int port,
				const struct switchdev_obj_port_fdb *fdb,
				struct switchdev_trans *trans);
	int	(*port_fdb_del)(struct dsa_switch *ds, int port,
				const struct switchdev_obj_port_fdb *fdb);
	int	(*port_fdb_dump)(struct dsa_switch *ds, int port,
				 struct switchdev_obj_port_fdb *fdb,
				 int (*cb)(struct switchdev_obj *obj));
};

void register_switch_driver(struct dsa_switch_driver *type);
void unregister_switch_driver(struct dsa_switch_driver *type);
struct mii_bus *dsa_host_dev_to_mii_bus(struct device *dev);

static inline void *ds_to_priv(struct dsa_switch *ds)
{
	return (void *)(ds + 1);
}

static inline bool dsa_uses_tagged_protocol(struct dsa_switch_tree *dst)
{
	return dst->rcv != NULL;
}
#endif
