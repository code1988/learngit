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

#define DSA_MAX_SWITCHES	4   // DSA驱动支持的最大交换机数量
#define DSA_MAX_PORTS		12  // DSA驱动支持的最大端口数

// 用于描述switch的配置信息
struct dsa_chip_data {
	/*
	 * How to access the switch configuration registers.
	 */
	struct device	*mii_bus;   // 指向该switch使用的mii总线
	int		sw_addr;            // switch地址序号，来自"switch"dts节点中的"reg"属性，最大不超过PHY_MAX_ADDR

	/*
	 * The names of the switch's ports.  Use "cpu" to
	 * designate the switch port that the cpu is connected to,
	 * "dsa" to indicate that this port is a DSA link to
	 * another switch, NULL to indicate the port is unused,
	 * or any other string to indicate this is a physical port.
     *
     * 记录了switch每个端口的名字
	 */
	char		*port_names[DSA_MAX_PORTS];

	/*
	 * An array (with nr_chips elements) of which element [a]
	 * indicates which port on this switch should be used to
	 * send packets to that are destined for switch a.  Can be
	 * NULL if there is only one switch chip.
     *
     * 级联switch情况下的switch端口路由表，不级联switch情况下该字段为NULL
	 */
	s8		*rtable;
};

// 用于描述DSA实例的配置信息
struct dsa_platform_data {
	/*
	 * Reference to a Linux network interface that connects
	 * to the root switch chip of the tree.
     *
     * 指向该DSA实例的宿主device
	 */
	struct device	*netdev;

	/*
	 * Info structs describing each of the switch chips
	 * connected via this network interface.
	 */
	int		nr_chips;               // 级联switch数量
	struct dsa_chip_data	*chip;  // 所有级联switch配置信息的描述集合
};

// DSA实例，被记录在DSA设备device->p->driver_data，并和跟宿主netdev->dsa_ptr关联
struct dsa_switch_tree {
	/*
	 * Configuration data for the platform device that owns
	 * this dsa switch tree instance.
     *
     * 指向该DSA实例的配置信息
	 */
	struct dsa_platform_data	*pd;

	/*
	 * Reference to network device to use, and which tagging
	 * protocol to use.
     *
     * 指向该DSA实例的宿主netdev
	 */
	struct net_device	*master_netdev;
	__be16			tag_protocol;   // 记录了该DSA实例使用的dsa-tag类型(原始类型就是ETH_P_DSA)，同步自该DSA实例的CPU口所属switch

	/*
	 * The switch and port to which the CPU is attached.
	 */
	s8			cpu_switch; // CPU口所属的switch序号
	s8			cpu_port;   // CPU口在所属switch上对应的端口序号

	/*
	 * Link state polling.
	 */
	int			link_poll_needed;       // 标识该DSA是否开启定时轮寻link状态
	struct work_struct	link_poll_work; // 轮寻link状态的工作队列
	struct timer_list	link_poll_timer;// 轮寻link状态的定时器

	/*
	 * Data for the individual switch chips.
     * 
     * 记录了该DSA实例包含的所有switchd
	 */
	struct dsa_switch	*ds[DSA_MAX_SWITCHES];
};

// switch实例
struct dsa_switch {
	/*
	 * Parent switch tree, and switch index.
     * 
     * 指向该switch所属的DSA实例
	 */
	struct dsa_switch_tree	*dst;
	int			index;      // 分配给该switch的序号

	/*
	 * Configuration data for this switch.
     *
     * 指向该switch的配置信息
	 */
	struct dsa_chip_data	*pd;

	/*
	 * The used switch driver.
     *
     * 指向该switch使用的驱动
	 */
	struct dsa_switch_driver	*drv;

	/*
	 * Reference to mii bus to use.
     *
     * 指向该switch使用的主mii-bus设备
	 */
	struct mii_bus		*master_mii_bus;

	/*
	 * Slave mii_bus and devices for the individual ports.
	 */
	u32			dsa_port_mask;      // 开启了dsa功能的端口集合
	u32			phys_port_mask;     // 物理口集合
	struct mii_bus		*slave_mii_bus;         // 指向该switch使用的从mii-bus设备(该创建该switch时创建，执行读写操作最终还是要通过主mii-bus来完成)
	struct net_device	*ports[DSA_MAX_PORTS];  // 该switch包含的所有端口对应的netdev集合
};

// 判断指定switch上的指定端口是否是CPU口
static inline bool dsa_is_cpu_port(struct dsa_switch *ds, int p)
{
    // 是否是CPU口需要同时满足2个条件：该switch是否是CPU口所在switch；该端口是否是CPU口所在switch上对应端口
	return !!(ds->index == ds->dst->cpu_switch && p == ds->dst->cpu_port);
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
     *
     * 上行口的定义：
     *          如果是root switch就是CPU口;
     *          否则就是该级联switch上更靠近CPU的DSA口
	 */
	if (dst->cpu_switch == ds->index)
		return dst->cpu_port;
	else
		return ds->pd->rtable[dst->cpu_switch];
}

// switch驱动
struct dsa_switch_driver {
	struct list_head	list;

	__be16			tag_protocol;   // 该switch使用的DSA-tag类型
	int			priv_size;          // 私有空间大小

	/*
	 * Probing and setup.
	 */
	char	*(*probe)(struct mii_bus *bus, int sw_addr);    // 通过mii-bus设备发起对指定switch的探测操作
	int	(*setup)(struct dsa_switch *ds);                    // 初始化探测到的switch
	int	(*set_addr)(struct dsa_switch *ds, u8 *addr);

	/*
	 * Access to the switch's PHY registers.
	 */
	int	(*phy_read)(struct dsa_switch *ds, int port, int regnum);   // 读端口寄存器
	int	(*phy_write)(struct dsa_switch *ds, int port,               // 写端口寄存器
			     int regnum, u16 val);

	/*
	 * Link state polling and IRQ handling.
	 */
	void	(*poll_link)(struct dsa_switch *ds);            // 查询该switch所有端口的链路状态

	/*
	 * ethtool hardware statistics.
	 */
	void	(*get_strings)(struct dsa_switch *ds, int port, uint8_t *data); // 获取该switch指定端口的统计项名
	void	(*get_ethtool_stats)(struct dsa_switch *ds,
				     int port, uint64_t *data);                             // 获取该switch指定端口的统计信息值
	int	(*get_sset_count)(struct dsa_switch *ds);                           // 获取该switch的统计项数量
};

void register_switch_driver(struct dsa_switch_driver *type);
void unregister_switch_driver(struct dsa_switch_driver *type);

/*
 * The original DSA tag format and some other tag formats have no
 * ethertype, which means that we need to add a little hack to the
 * networking receive path to make sure that received frames get
 * the right ->protocol assigned to them when one of those tag
 * formats is in use.
 * 判断传入的DSA管理块是否使用了ETH_P_DSA类型的dsa-tag
 */
static inline bool dsa_uses_dsa_tags(struct dsa_switch_tree *dst)
{
	return !!(dst->tag_protocol == htons(ETH_P_DSA));
}

static inline bool dsa_uses_trailer_tags(struct dsa_switch_tree *dst)
{
	return !!(dst->tag_protocol == htons(ETH_P_TRAILER));
}

#endif
