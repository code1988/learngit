/*
 * IMX pinmux core definitions
 *
 * Copyright (C) 2012-2015 Freescale Semiconductor, Inc.
 * Copyright (C) 2012 Linaro Ltd.
 *
 * Author: Dong Aisheng <dong.aisheng@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __DRIVERS_PINCTRL_IMX_H
#define __DRIVERS_PINCTRL_IMX_H

struct platform_device;

/**
 * struct imx_pin_group - describes a single i.MX pin
 * @pin: the pin_id of this pin
 * @mux_mode: the mux mode for this pin.
 * @input_reg: the select input register offset for this pin if any
 *	0 if no select input setting needed.
 * @input_val: the select input value for this pin.
 * @configs: the config for this pin.
 */
struct imx_pin {
	unsigned int pin;
	unsigned int mux_mode;
	u16 input_reg;
	unsigned int input_val;
	unsigned long config;
};

/**
 * struct imx_pin_group - describes an IMX pin group
 * @name: the name of this specific pin group
 * @npins: the number of pins in this group array, i.e. the number of
 *	elements in .pins so we can iterate over that array
 * @pin_ids: array of pin_ids. pinctrl forces us to maintain such an array
 * @pins: array of pins
 */
struct imx_pin_group {
	const char *name;
	unsigned npins;
	unsigned int *pin_ids;
	struct imx_pin *pins;
};

/**
 * struct imx_pmx_func - describes IMX pinmux functions
 * @name: the name of this specific function
 * @groups: corresponding pin groups
 * @num_groups: the number of groups
 */
struct imx_pmx_func {
	const char *name;
	const char **groups;
	unsigned num_groups;
};

/**
 * struct imx_pin_reg - describe a pin reg map
 * @mux_reg: mux register offset
 * @conf_reg: config register offset
 */
struct imx_pin_reg {
	u16 mux_reg;
	u16 conf_reg;
};

// 定义了imx系列通用的板卡pin-control模块的low-level描述符
struct imx_pinctrl_soc_info {
	struct device *dev;                     // 驱动-设备匹配后，该字段指向匹配的设备
	const struct pinctrl_pin_desc *pins;    // 指向一张pin脚描述表
	unsigned int npins;                     // pin脚描述表的表项数量
	struct imx_pin_reg *pin_regs;
	struct imx_pin_group *groups;
	unsigned int ngroups;
	struct imx_pmx_func *functions;
	unsigned int nfunctions;
	unsigned int flags;
	u32 grp_index;
};

#define ZERO_OFFSET_VALID	0x1
#define SHARE_MUX_CONF_REG	0x2
#define SHARE_INPUT_SELECT_REG 0x4

#define NO_MUX		0x0
#define NO_PAD		0x0

// 定义一个imx平台上的pin脚
#define IMX_PINCTRL_PIN(pin) PINCTRL_PIN(pin, #pin)

#define PAD_CTL_MASK(len)	((1 << len) - 1)
#define IMX_MUX_MASK	0x7
#define IOMUXC_CONFIG_SION	(0x1 << 4)

int imx_pinctrl_probe(struct platform_device *pdev,
			struct imx_pinctrl_soc_info *info);
int imx_pinctrl_remove(struct platform_device *pdev);
#endif /* __DRIVERS_PINCTRL_IMX_H */
