/*
 * net/dsa/mv88e6131.c - Marvell 88e6095/6095f/6131 switch chip support
 * Copyright (c) 2008-2009 Marvell Semiconductor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/phy.h>
#include <net/dsa.h>
#include "mv88e6xxx.h"

static const struct mv88e6xxx_switch_id mv88e6131_table[] = {
	{ PORT_SWITCH_ID_6085, "Marvell 88E6085" },
	{ PORT_SWITCH_ID_6095, "Marvell 88E6095/88E6095F" },
	{ PORT_SWITCH_ID_6131, "Marvell 88E6131" },
	{ PORT_SWITCH_ID_6131_B2, "Marvell 88E6131 (B2)" },
	{ PORT_SWITCH_ID_6185, "Marvell 88E6185" },
};

/* mv88e6131系列switch执行探测指定switch的操作
 * //@bus     - 探测操作最终需要通过mdio设备进行
 * @sw_addr - 需要探测的switch地址序号
 *
 * @返回值  成功探测到该switch则返回switch名，探测失败则返回NULL
 */
static char *mv88e6131_probe(struct device *host_dev, int sw_addr)
{
	return mv88e6xxx_lookup_name(host_dev, sw_addr, mv88e6131_table,
				     ARRAY_SIZE(mv88e6131_table));
}

/* mv88e6131系列switch执行全局参数初始化操作
 * 包括使能PHY轮寻、设置缺省的MAC老化时间、使能地址学习、配置映射寄存器优先级、设置vlan协议类型、
 * 禁止ARP镜像、禁止端口级联、IGMP/ARP设为高优先级等
 */
static int mv88e6131_setup_global(struct dsa_switch *ds)
{
	u32 upstream_port = dsa_upstream_port(ds);
	int ret;
	u32 reg;

	ret = mv88e6xxx_setup_global(ds);
	if (ret)
		return ret;

	/* Enable the PHY polling unit, don't discard packets with
	 * excessive collisions, use a weighted fair queueing scheme
	 * to arbitrate between packet queues, set the maximum frame
	 * size to 1632, and mask all interrupt sources.
	 */
	REG_WRITE(REG_GLOBAL, GLOBAL_CONTROL,
		  GLOBAL_CONTROL_PPU_ENABLE | GLOBAL_CONTROL_MAX_FRAME_1632);

	/* Set the VLAN ethertype to 0x8100. */
	REG_WRITE(REG_GLOBAL, GLOBAL_CORE_TAG_TYPE, 0x8100);

	/* Disable ARP mirroring, and configure the upstream port as
	 * the port to which ingress and egress monitor frames are to
	 * be sent.
	 */
	reg = upstream_port << GLOBAL_MONITOR_CONTROL_INGRESS_SHIFT |
		upstream_port << GLOBAL_MONITOR_CONTROL_EGRESS_SHIFT |
		GLOBAL_MONITOR_CONTROL_ARP_DISABLED;
	REG_WRITE(REG_GLOBAL, GLOBAL_MONITOR_CONTROL, reg);

	/* Disable cascade port functionality unless this device
	 * is used in a cascade configuration, and set the switch's
	 * DSA device number.
	 */
	if (ds->dst->pd->nr_chips > 1)
		REG_WRITE(REG_GLOBAL, GLOBAL_CONTROL_2,
			  GLOBAL_CONTROL_2_MULTIPLE_CASCADE |
			  (ds->index & 0x1f));
	else
		REG_WRITE(REG_GLOBAL, GLOBAL_CONTROL_2,
			  GLOBAL_CONTROL_2_NO_CASCADE |
			  (ds->index & 0x1f));

	/* Force the priority of IGMP/MLD snoop frames and ARP frames
	 * to the highest setting.
	 */
	REG_WRITE(REG_GLOBAL2, GLOBAL2_PRIO_OVERRIDE,
		  GLOBAL2_PRIO_OVERRIDE_FORCE_SNOOP |
		  7 << GLOBAL2_PRIO_OVERRIDE_SNOOP_SHIFT |
		  GLOBAL2_PRIO_OVERRIDE_FORCE_ARP |
		  7 << GLOBAL2_PRIO_OVERRIDE_ARP_SHIFT);

	return 0;
}

/* mv88e6131系列switch执行初始化操作
 * @ds  要操作的switch实例
 */
static int mv88e6131_setup(struct dsa_switch *ds)
{
	struct mv88e6xxx_priv_state *ps = ds_to_priv(ds);
	int ret;

	ret = mv88e6xxx_setup_common(ds);
	if (ret < 0)
		return ret;

	mv88e6xxx_ppu_state_init(ds);

	switch (ps->id) {
	case PORT_SWITCH_ID_6085:
	case PORT_SWITCH_ID_6185:
		ps->num_ports = 10;
		break;
	case PORT_SWITCH_ID_6095:
		ps->num_ports = 11;
		break;
	case PORT_SWITCH_ID_6131:
	case PORT_SWITCH_ID_6131_B2:
		ps->num_ports = 8;
		break;
	default:
		return -ENODEV;
	}

    // 对该switch执行复位操作
	ret = mv88e6xxx_switch_reset(ds, false);
	if (ret < 0)
		return ret;

    // 对该switch执行全局参数初始化操作
	ret = mv88e6131_setup_global(ds);
	if (ret < 0)
		return ret;

    // 对该switch所有端口参数执行初始化
	return mv88e6xxx_setup_ports(ds);
}

// mv88e6131系列返回switch物理端口对应的phy地址(该系列端口号就是phy地址)
static int mv88e6131_port_to_phy_addr(struct dsa_switch *ds, int port)
{
	struct mv88e6xxx_priv_state *ps = ds_to_priv(ds);

    // 检查端口是否合法(该系列不超过11个端口)
	if (port >= 0 && port < ps->num_ports)
		return port;

	return -EINVAL;
}

/* mv88e6131系列switch读指定端口的指定寄存器的接口(将作为DSA从mii-bus设备的读操作驱动)
 * @ds      要操作的switch实例
 * @port    要操作的端口号
 * @regnum  端口上的寄存器序号
 */
static int
mv88e6131_phy_read(struct dsa_switch *ds, int port, int regnum)
{
	int addr = mv88e6131_port_to_phy_addr(ds, port);

	if (addr < 0)
		return addr;

	return mv88e6xxx_phy_read_ppu(ds, addr, regnum);
}

/* mv88e6131系列switch写指定端口的指定寄存器的接口(将作为DSA从mii-bus设备的写操作驱动)
 * @ds      要操作的switch实例
 * @port    要操作的端口号
 * @regnum  端口上的寄存器序号
 * @val     要写入的值
 */
static int
mv88e6131_phy_write(struct dsa_switch *ds,
			      int port, int regnum, u16 val)
{
	int addr = mv88e6131_port_to_phy_addr(ds, port);

	if (addr < 0)
		return addr;

	return mv88e6xxx_phy_write_ppu(ds, addr, regnum, val);
}

// 定义了mv88e6131系列共用的switch驱动
struct dsa_switch_driver mv88e6131_switch_driver = {
	.tag_protocol		= DSA_TAG_PROTO_DSA,    // 该switch驱动使用ETH_P_DSA协议
	.priv_size		= sizeof(struct mv88e6xxx_priv_state),
	.probe			= mv88e6131_probe,
	.setup			= mv88e6131_setup,
	.set_addr		= mv88e6xxx_set_addr_direct,
	.phy_read		= mv88e6131_phy_read,
	.phy_write		= mv88e6131_phy_write,
	.get_strings		= mv88e6xxx_get_strings,
	.get_ethtool_stats	= mv88e6xxx_get_ethtool_stats,
	.get_sset_count		= mv88e6xxx_get_sset_count,
	.adjust_link		= mv88e6xxx_adjust_link,
};

MODULE_ALIAS("platform:mv88e6085");
MODULE_ALIAS("platform:mv88e6095");
MODULE_ALIAS("platform:mv88e6095f");
MODULE_ALIAS("platform:mv88e6131");
