/*
 * Copyright (C) ST-Ericsson SA 2011
 * Author: Lee Jones <lee.jones@linaro.org> for ST-Ericsson.
 * License terms:  GNU General Public License (GPL), version 2
 */
#ifndef __SOC_BUS_H
#define __SOC_BUS_H

#include <linux/device.h>

// 定义了soc的属性集合
struct soc_device_attribute {
	const char *machine;    // 板卡型号，通常来自设备树root节点下的"model"属性值
	const char *family;     // 该soc所属的家族(比如"Freescale i.MX")
	const char *revision;   // 该soc版本号，一个"*.*"的字符串
	const char *soc_id;     // 类似于板卡型号(比如"i.MX6UL")
};

/**
 * soc_device_register - register SoC as a device
 * @soc_plat_dev_attr: Attributes passed from platform to be attributed to a SoC
 */
struct soc_device *soc_device_register(
	struct soc_device_attribute *soc_plat_dev_attr);

/**
 * soc_device_unregister - unregister SoC device
 * @dev: SoC device to be unregistered
 */
void soc_device_unregister(struct soc_device *soc_dev);

/**
 * soc_device_to_device - helper function to fetch struct device
 * @soc: Previously registered SoC device container
 */
struct device *soc_device_to_device(struct soc_device *soc);

#endif /* __SOC_BUS_H */
