/*
 * Copyright (C) 2015 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/can/platform/flexcan.h>
#include <linux/gpio.h>
#include <linux/irqchip.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/phy.h>
#include <linux/regmap.h>
#include <linux/mfd/syscon.h>
#include <linux/mfd/syscon/imx6q-iomuxc-gpr.h>
#include <linux/pm_opp.h>
#include <linux/fec.h>
#include <linux/netdevice.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include "common.h"
#include "cpuidle.h"

// imx6ul板卡的以太网控制器时钟初始化
static void __init imx6ul_enet_clk_init(void)
{
	struct regmap *gpr;

    // 根据"compatible"属性值查找对应的iomuxc-gpr模块的regmap句柄
	gpr = syscon_regmap_lookup_by_compatible("fsl,imx6ul-iomuxc-gpr");
    // 这里使能了ENET1和ENET2的输出时钟
	if (!IS_ERR(gpr))
		regmap_update_bits(gpr, IOMUXC_GPR1, IMX6UL_GPR1_ENET_CLK_DIR,
				   IMX6UL_GPR1_ENET_CLK_OUTPUT);
	else
		pr_err("failed to find fsl,imx6ul-iomux-gpr regmap\n");

}

static int ksz8081_phy_fixup(struct phy_device *dev)
{
	if (dev && dev->interface == PHY_INTERFACE_MODE_MII) {
		phy_write(dev, 0x1f, 0x8110);
		phy_write(dev, 0x16, 0x201);
	} else if (dev && dev->interface == PHY_INTERFACE_MODE_RMII) {
		phy_write(dev, 0x1f, 0x8190);
		phy_write(dev, 0x16, 0x202);
	}

	return 0;
}

// imx6ul板卡的以太网控制器phy初始化
#define PHY_ID_KSZ8081	0x00221560
static void __init imx6ul_enet_phy_init(void)
{
	phy_register_fixup_for_uid(PHY_ID_KSZ8081, 0xffffffff,	ksz8081_phy_fixup);
}

// imx6ul板卡的以太网控制器初始化
static inline void imx6ul_enet_init(void)
{
	imx6ul_enet_clk_init();
	imx6ul_enet_phy_init();
	imx6_enet_mac_init("fsl,imx6ul-fec");
}

// 将imx6ul设备树中定义的各种device node加入到系统中
static void __init imx6ul_init_machine(void)
{
	struct device *parent;

    // 看门狗初始化(imx6ul实际使用1号看门狗)
	mxc_arch_reset_init_dt();

    // imx6ul型号的soc层面初始化，初始化成功后返回一个device指针，对应一个soc设备
	parent = imx_soc_device_init();
	if (parent == NULL)
		pr_warn("failed to initialize soc device\n");

    // 从root node开始遍历设备树device_node结构，创建对应的platform设备，并注册到platform总线上，如果匹配到驱动就要开始probe...
	of_platform_populate(NULL, of_default_bus_match_table,
					NULL, parent);

    // 以太网控制器初始化
	imx6ul_enet_init();
	imx_anatop_init();
	imx6ul_pm_init();
}

// imx6ul中断控制器初始化
static void __init imx6ul_init_irq(void)
{
	imx_init_revision_from_anatop();
	imx_src_init();
	imx_gpc_init();
	irqchip_init();
}

// 定义了imx6ul平台支持的compatible字符串列表，用于跟dtb的root node的compatible字符串列表匹配
static const char *imx6ul_dt_compat[] __initdata = {
	"fsl,imx6ul",
	NULL,
};

static void __init imx6ul_init_late(void)
{
	platform_device_register_simple("imx6q-cpufreq", -1, NULL, 0);

	imx6ul_cpuidle_init();
}

static void __init imx6ul_map_io(void)
{
	debug_ll_io_init();
	imx6_pm_map_io();
#ifdef CONFIG_CPU_FREQ
	imx_busfreq_map_io();
#endif
}

// 定义了一个imx6ul平台的machine描述符
DT_MACHINE_START(IMX6UL, "Freescale i.MX6 UltraLite (Device Tree)")
	.map_io		= imx6ul_map_io,
	.init_irq	= imx6ul_init_irq,
	.init_machine	= imx6ul_init_machine,
	.init_late	= imx6ul_init_late,
	.dt_compat	= imx6ul_dt_compat,
	.restart	= mxc_restart,
MACHINE_END
