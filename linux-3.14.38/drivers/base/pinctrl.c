/*
 * Driver core interface to the pinctrl subsystem.
 *
 * Copyright (C) 2012 ST-Ericsson SA
 * Written on behalf of Linaro for ST-Ericsson
 * Based on bits of regulator core, gpio core and clk core
 *
 * Author: Linus Walleij <linus.walleij@linaro.org>
 *
 * License terms: GNU General Public License (GPL) version 2
 */

#include <linux/device.h>
#include <linux/pinctrl/devinfo.h>
#include <linux/pinctrl/consumer.h>
#include <linux/slab.h>

/**
 * pinctrl_bind_pins() - called by the device core before probe
 * 将指定device和对应的pin-control句柄绑定(显然本函数就是封装了一系列pin-control子系统API)
 * 绑定之后会将该pin-control缺省状态设置到相关的硬件中
 *
 * @dev: the device that is just about to probe
 *
 * 备注：本接口只会在really_probe中，设备probe之前被调用到
 */
int pinctrl_bind_pins(struct device *dev)
{
	int ret;

	dev->pins = devm_kzalloc(dev, sizeof(*(dev->pins)), GFP_KERNEL);
	if (!dev->pins)
		return -ENOMEM;

    // 创建该device对应的pin-control句柄(带垃圾回收机制)
	dev->pins->p = devm_pinctrl_get(dev);
	if (IS_ERR(dev->pins->p)) {
		dev_dbg(dev, "no pinctrl handle\n");
		ret = PTR_ERR(dev->pins->p);
		goto cleanup_alloc;
	}

    // 从该pin-control句柄中检索缺省状态的描述符
	dev->pins->default_state = pinctrl_lookup_state(dev->pins->p,
					PINCTRL_STATE_DEFAULT);
	if (IS_ERR(dev->pins->default_state)) {
		dev_dbg(dev, "no default pinctrl state\n");
		ret = 0;
		goto cleanup_get;
	}

    // 通过该pin-control设置缺省状态到相关硬件中
	ret = pinctrl_select_state(dev->pins->p, dev->pins->default_state);
	if (ret) {
		dev_dbg(dev, "failed to activate default pinctrl state\n");
		goto cleanup_get;
	}

#ifdef CONFIG_PM
	/*
	 * If power management is enabled, we also look for the optional
	 * sleep and idle pin states, with semantics as defined in
	 * <linux/pinctrl/pinctrl-state.h>
	 */
	dev->pins->sleep_state = pinctrl_lookup_state(dev->pins->p,
					PINCTRL_STATE_SLEEP);
	if (IS_ERR(dev->pins->sleep_state))
		/* Not supplying this state is perfectly legal */
		dev_dbg(dev, "no sleep pinctrl state\n");

	dev->pins->idle_state = pinctrl_lookup_state(dev->pins->p,
					PINCTRL_STATE_IDLE);
	if (IS_ERR(dev->pins->idle_state))
		/* Not supplying this state is perfectly legal */
		dev_dbg(dev, "no idle pinctrl state\n");
#endif

	return 0;

	/*
	 * If no pinctrl handle or default state was found for this device,
	 * let's explicitly free the pin container in the device, there is
	 * no point in keeping it around.
	 */
cleanup_get:
	devm_pinctrl_put(dev->pins->p);
cleanup_alloc:
	devm_kfree(dev, dev->pins);
	dev->pins = NULL;

	/* Only return deferrals */
	if (ret != -EPROBE_DEFER)
		ret = 0;

	return ret;
}
