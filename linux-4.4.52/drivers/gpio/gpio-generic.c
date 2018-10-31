/*
 * Generic driver for memory-mapped GPIO controllers.
 *
 * Copyright 2008 MontaVista Software, Inc.
 * Copyright 2008,2010 Anton Vorontsov <cbouatmailru@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * ....``.```~~~~````.`.`.`.`.```````'',,,.........`````......`.......
 * ...``                                                         ```````..
 * ..The simplest form of a GPIO controller that the driver supports is``
 *  `.just a single "data" register, where GPIO state can be read and/or `
 *    `,..written. ,,..``~~~~ .....``.`.`.~~.```.`.........``````.```````
 *        `````````
                                    ___
_/~~|___/~|   . ```~~~~~~       ___/___\___     ,~.`.`.`.`````.~~...,,,,...
__________|~$@~~~        %~    /o*o*o*o*o*o\   .. Implementing such a GPIO .
o        `                     ~~~~\___/~~~~    ` controller in FPGA is ,.`
                                                 `....trivial..'~`.```.```
 *                                                    ```````
 *  .```````~~~~`..`.``.``.
 * .  The driver supports  `...       ,..```.`~~~```````````````....````.``,,
 * .   big-endian notation, just`.  .. A bit more sophisticated controllers ,
 *  . register the device with -be`. .with a pair of set/clear-bit registers ,
 *   `.. suffix.  ```~~`````....`.`   . affecting the data register and the .`
 *     ``.`.``...```                  ```.. output pins are also supported.`
 *                        ^^             `````.`````````.,``~``~``~~``````
 *                                                   .                  ^^
 *   ,..`.`.`...````````````......`.`.`.`.`.`..`.`.`..
 * .. The expectation is that in at least some cases .    ,-~~~-,
 *  .this will be used with roll-your-own ASIC/FPGA .`     \   /
 *  .logic in Verilog or VHDL. ~~~`````````..`````~~`       \ /
 *  ..````````......```````````                             \o_
 *                                                           |
 *                              ^^                          / \
 *
 *           ...`````~~`.....``.`..........``````.`.``.```........``.
 *            `  8, 16, 32 and 64 bits registers are supported, and``.
 *            . the number of GPIOs is determined by the width of   ~
 *             .. the registers. ,............```.`.`..`.`.~~~.`.`.`~
 *               `.......````.```
 */

#include <linux/init.h>
#include <linux/err.h>
#include <linux/bug.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/compiler.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/log2.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/basic_mmio_gpio.h>

static void bgpio_write8(void __iomem *reg, unsigned long data)
{
	writeb(data, reg);
}

static unsigned long bgpio_read8(void __iomem *reg)
{
	return readb(reg);
}

static void bgpio_write16(void __iomem *reg, unsigned long data)
{
	writew(data, reg);
}

static unsigned long bgpio_read16(void __iomem *reg)
{
	return readw(reg);
}

// 写32位gpio寄存器(小端)
static void bgpio_write32(void __iomem *reg, unsigned long data)
{
	writel(data, reg);
}

// 读32位gpio寄存器(小端)
static unsigned long bgpio_read32(void __iomem *reg)
{
	return readl(reg);
}

#if BITS_PER_LONG >= 64
static void bgpio_write64(void __iomem *reg, unsigned long data)
{
	writeq(data, reg);
}

static unsigned long bgpio_read64(void __iomem *reg)
{
	return readq(reg);
}
#endif /* BITS_PER_LONG >= 64 */

static void bgpio_write16be(void __iomem *reg, unsigned long data)
{
	iowrite16be(data, reg);
}

static unsigned long bgpio_read16be(void __iomem *reg)
{
	return ioread16be(reg);
}

static void bgpio_write32be(void __iomem *reg, unsigned long data)
{
	iowrite32be(data, reg);
}

static unsigned long bgpio_read32be(void __iomem *reg)
{
	return ioread32be(reg);
}

// 根据gpio编号返回对应的寄存器位
static unsigned long bgpio_pin2mask(struct bgpio_chip *bgc, unsigned int pin)
{
	return 1 << pin;
}

static unsigned long bgpio_pin2mask_be(struct bgpio_chip *bgc,
				       unsigned int pin)
{
	return 1 << (bgc->bits - 1 - pin);
}

static int bgpio_get_set(struct gpio_chip *gc, unsigned int gpio)
{
	struct bgpio_chip *bgc = to_bgpio_chip(gc);
	unsigned long pinmask = bgc->pin2mask(bgc, gpio);

	if (bgc->dir & pinmask)
		return !!(bgc->read_reg(bgc->reg_set) & pinmask);
	else
		return !!(bgc->read_reg(bgc->reg_dat) & pinmask);
}

// gpio_chip->get的唯一方法
static int bgpio_get(struct gpio_chip *gc, unsigned int gpio)
{
	struct bgpio_chip *bgc = to_bgpio_chip(gc);

	return !!(bgc->read_reg(bgc->reg_dat) & bgc->pin2mask(bgc, gpio));
}

static void bgpio_set_none(struct gpio_chip *gc, unsigned int gpio, int val)
{
}

// gpio_chip->set方法之一,当bgc->reg_set和bgc->reg_clr都没有设置时启用
static void bgpio_set(struct gpio_chip *gc, unsigned int gpio, int val)
{
	struct bgpio_chip *bgc = to_bgpio_chip(gc);
	unsigned long mask = bgc->pin2mask(bgc, gpio);
	unsigned long flags;

	spin_lock_irqsave(&bgc->lock, flags);

	if (val)
		bgc->data |= mask;
	else
		bgc->data &= ~mask;

	bgc->write_reg(bgc->reg_dat, bgc->data);

	spin_unlock_irqrestore(&bgc->lock, flags);
}

// gpio_chip->set方法之一,当bgc->reg_set和bgc->reg_clr同时设置时启用
static void bgpio_set_with_clear(struct gpio_chip *gc, unsigned int gpio,
				 int val)
{
	struct bgpio_chip *bgc = to_bgpio_chip(gc);
	unsigned long mask = bgc->pin2mask(bgc, gpio);

	if (val)
		bgc->write_reg(bgc->reg_set, mask);
	else
		bgc->write_reg(bgc->reg_clr, mask);
}

// gpio_chip->set方法之一,当只有bgc->reg_set设置时启用
static void bgpio_set_set(struct gpio_chip *gc, unsigned int gpio, int val)
{
	struct bgpio_chip *bgc = to_bgpio_chip(gc);
	unsigned long mask = bgc->pin2mask(bgc, gpio);
	unsigned long flags;

	spin_lock_irqsave(&bgc->lock, flags);

	if (val)
		bgc->data |= mask;
	else
		bgc->data &= ~mask;

	bgc->write_reg(bgc->reg_set, bgc->data);

	spin_unlock_irqrestore(&bgc->lock, flags);
}

static void bgpio_multiple_get_masks(struct bgpio_chip *bgc,
				     unsigned long *mask, unsigned long *bits,
				     unsigned long *set_mask,
				     unsigned long *clear_mask)
{
	int i;

	*set_mask = 0;
	*clear_mask = 0;

	for (i = 0; i < bgc->bits; i++) {
		if (*mask == 0)
			break;
		if (__test_and_clear_bit(i, mask)) {
			if (test_bit(i, bits))
				*set_mask |= bgc->pin2mask(bgc, i);
			else
				*clear_mask |= bgc->pin2mask(bgc, i);
		}
	}
}

static void bgpio_set_multiple_single_reg(struct bgpio_chip *bgc,
					  unsigned long *mask,
					  unsigned long *bits,
					  void __iomem *reg)
{
	unsigned long flags;
	unsigned long set_mask, clear_mask;

	spin_lock_irqsave(&bgc->lock, flags);

	bgpio_multiple_get_masks(bgc, mask, bits, &set_mask, &clear_mask);

	bgc->data |= set_mask;
	bgc->data &= ~clear_mask;

	bgc->write_reg(reg, bgc->data);

	spin_unlock_irqrestore(&bgc->lock, flags);
}

static void bgpio_set_multiple(struct gpio_chip *gc, unsigned long *mask,
			       unsigned long *bits)
{
	struct bgpio_chip *bgc = to_bgpio_chip(gc);

	bgpio_set_multiple_single_reg(bgc, mask, bits, bgc->reg_dat);
}

static void bgpio_set_multiple_set(struct gpio_chip *gc, unsigned long *mask,
				   unsigned long *bits)
{
	struct bgpio_chip *bgc = to_bgpio_chip(gc);

	bgpio_set_multiple_single_reg(bgc, mask, bits, bgc->reg_set);
}

static void bgpio_set_multiple_with_clear(struct gpio_chip *gc,
					  unsigned long *mask,
					  unsigned long *bits)
{
	struct bgpio_chip *bgc = to_bgpio_chip(gc);
	unsigned long set_mask, clear_mask;

	bgpio_multiple_get_masks(bgc, mask, bits, &set_mask, &clear_mask);

	if (set_mask)
		bgc->write_reg(bgc->reg_set, set_mask);
	if (clear_mask)
		bgc->write_reg(bgc->reg_clr, clear_mask);
}

// gpio_chip->direction_input方法之一,当bgc->reg_dir没有设置时启用
static int bgpio_simple_dir_in(struct gpio_chip *gc, unsigned int gpio)
{
	return 0;
}

static int bgpio_dir_out_err(struct gpio_chip *gc, unsigned int gpio,
				int val)
{
	return -EINVAL;
}

// gpio_chip->direction_output方法之一,当bgc->reg_dir没有设置时启用
static int bgpio_simple_dir_out(struct gpio_chip *gc, unsigned int gpio,
				int val)
{
	gc->set(gc, gpio, val);

	return 0;
}

// gpio_chip->direction_input方法之一,当bgc->reg_dir设置为输出寄存器时启用
static int bgpio_dir_in(struct gpio_chip *gc, unsigned int gpio)
{
	struct bgpio_chip *bgc = to_bgpio_chip(gc);
	unsigned long flags;

	spin_lock_irqsave(&bgc->lock, flags);

	bgc->dir &= ~bgc->pin2mask(bgc, gpio);
	bgc->write_reg(bgc->reg_dir, bgc->dir);

	spin_unlock_irqrestore(&bgc->lock, flags);

	return 0;
}

static int bgpio_get_dir(struct gpio_chip *gc, unsigned int gpio)
{
	struct bgpio_chip *bgc = to_bgpio_chip(gc);

	return (bgc->read_reg(bgc->reg_dir) & bgc->pin2mask(bgc, gpio)) ?
	       GPIOF_DIR_OUT : GPIOF_DIR_IN;
}

// gpio_chip->direction_output方法之一,当bgc->reg_dir设置为输出寄存器时启用
static int bgpio_dir_out(struct gpio_chip *gc, unsigned int gpio, int val)
{
	struct bgpio_chip *bgc = to_bgpio_chip(gc);
	unsigned long flags;

	gc->set(gc, gpio, val);

	spin_lock_irqsave(&bgc->lock, flags);

	bgc->dir |= bgc->pin2mask(bgc, gpio);
	bgc->write_reg(bgc->reg_dir, bgc->dir);

	spin_unlock_irqrestore(&bgc->lock, flags);

	return 0;
}

// gpio_chip->direction_input方法之一,当bgc->reg_dir设置为输入寄存器时启用
static int bgpio_dir_in_inv(struct gpio_chip *gc, unsigned int gpio)
{
	struct bgpio_chip *bgc = to_bgpio_chip(gc);
	unsigned long flags;

	spin_lock_irqsave(&bgc->lock, flags);

	bgc->dir |= bgc->pin2mask(bgc, gpio);
	bgc->write_reg(bgc->reg_dir, bgc->dir);

	spin_unlock_irqrestore(&bgc->lock, flags);

	return 0;
}

// gpio_chip->direction_output方法之一,当bgc->reg_dir设置为输入寄存器时启用
static int bgpio_dir_out_inv(struct gpio_chip *gc, unsigned int gpio, int val)
{
	struct bgpio_chip *bgc = to_bgpio_chip(gc);
	unsigned long flags;

	gc->set(gc, gpio, val);

	spin_lock_irqsave(&bgc->lock, flags);

	bgc->dir &= ~bgc->pin2mask(bgc, gpio);
	bgc->write_reg(bgc->reg_dir, bgc->dir);

	spin_unlock_irqrestore(&bgc->lock, flags);

	return 0;
}

static int bgpio_get_dir_inv(struct gpio_chip *gc, unsigned int gpio)
{
	struct bgpio_chip *bgc = to_bgpio_chip(gc);

	return (bgc->read_reg(bgc->reg_dir) & bgc->pin2mask(bgc, gpio)) ?
	       GPIOF_DIR_IN : GPIOF_DIR_OUT;
}

/* 为指定gpio控制器配置底层通用的寄存器read/write方法
 * @bit_be  标识gpio寄存器内是否是大端格式
 * @byte_be 标识gpio寄存器组之间是否是大端格式
 */
static int bgpio_setup_accessors(struct device *dev,
				 struct bgpio_chip *bgc,
				 bool bit_be,
				 bool byte_be)
{

    // 根据gpio寄存器位长执行设置对应的read_reg和write_reg方法
	switch (bgc->bits) {
	case 8:
		bgc->read_reg	= bgpio_read8;
		bgc->write_reg	= bgpio_write8;
		break;
	case 16:
		if (byte_be) {
			bgc->read_reg	= bgpio_read16be;
			bgc->write_reg	= bgpio_write16be;
		} else {
			bgc->read_reg	= bgpio_read16;
			bgc->write_reg	= bgpio_write16;
		}
		break;
	case 32:
		if (byte_be) {
			bgc->read_reg	= bgpio_read32be;
			bgc->write_reg	= bgpio_write32be;
		} else {
			bgc->read_reg	= bgpio_read32;
			bgc->write_reg	= bgpio_write32;
		}
		break;
#if BITS_PER_LONG >= 64
	case 64:
		if (byte_be) {
			dev_err(dev,
				"64 bit big endian byte order unsupported\n");
			return -EINVAL;
		} else {
			bgc->read_reg	= bgpio_read64;
			bgc->write_reg	= bgpio_write64;
		}
		break;
#endif /* BITS_PER_LONG >= 64 */
	default:
		dev_err(dev, "unsupported data width %u bits\n", bgc->bits);
		return -EINVAL;
	}

	bgc->pin2mask = bit_be ? bgpio_pin2mask_be : bgpio_pin2mask;

	return 0;
}

/*
 * Create the device and allocate the resources.  For setting GPIO's there are
 * three supported configurations:
 * 为指定gpio控制器配置底层寄存器资源和基本的get/set方法
 *
 *	- single input/output register resource (named "dat").
 *	- set/clear pair (named "set" and "clr").
 *	- single output register resource and single input resource ("set" and
 *	dat").
 *
 * For the single output register, this drives a 1 by setting a bit and a zero
 * by clearing a bit.  For the set clr pair, this drives a 1 by setting a bit
 * in the set register and clears it by setting a bit in the clear register.
 * The configuration is detected by which resources are present.
 *
 * For setting the GPIO direction, there are three supported configurations:
 *
 *	- simple bidirection GPIO that requires no configuration.
 *	- an output direction register (named "dirout") where a 1 bit
 *	indicates the GPIO is an output.
 *	- an input direction register (named "dirin") where a 1 bit indicates
 *	the GPIO is an input.
 */
static int bgpio_setup_io(struct bgpio_chip *bgc,
			  void __iomem *dat,
			  void __iomem *set,
			  void __iomem *clr,
			  unsigned long flags)
{

    // reg_dat寄存器必须设置
	bgc->reg_dat = dat;
	if (!bgc->reg_dat)
		return -EINVAL;

    // 显然gpio_chip->set方法需要进行适配
	if (set && clr) {
        // 同时传入了set和clr寄存器的情况
		bgc->reg_set = set;
		bgc->reg_clr = clr;
		bgc->gc.set = bgpio_set_with_clear;
		bgc->gc.set_multiple = bgpio_set_multiple_with_clear;
	} else if (set && !clr) {
        // 只传入了set寄存器的情况
		bgc->reg_set = set;
		bgc->gc.set = bgpio_set_set;
		bgc->gc.set_multiple = bgpio_set_multiple_set;
	} else if (flags & BGPIOF_NO_OUTPUT) {
		bgc->gc.set = bgpio_set_none;
		bgc->gc.set_multiple = NULL;
	} else {
        // 其他情况
		bgc->gc.set = bgpio_set;
		bgc->gc.set_multiple = bgpio_set_multiple;
	}

    // 3.14.38中gpio_chip->get方法唯一；4.4.52中显然不唯一
	if (!(flags & BGPIOF_UNREADABLE_REG_SET) &&
	    (flags & BGPIOF_READ_OUTPUT_REG_SET))
		bgc->gc.get = bgpio_get_set;
	else
		bgc->gc.get = bgpio_get;

	return 0;
}

/* 为指定gpio控制器配置底层方向设置寄存器资源和相关的方向设置方法
 *
 * 备注:这两个寄存器只允许配置其中一个
 */
static int bgpio_setup_direction(struct bgpio_chip *bgc,
				 void __iomem *dirout,
				 void __iomem *dirin,
				 unsigned long flags)
{
	if (dirout && dirin) {
        // 不允许同时传入
		return -EINVAL;
	} else if (dirout) {
        // 只传入了dirout寄存器的情况
		bgc->reg_dir = dirout;
		bgc->gc.direction_output = bgpio_dir_out;
		bgc->gc.direction_input = bgpio_dir_in;
		bgc->gc.get_direction = bgpio_get_dir;
	} else if (dirin) {
        // 只传入了dirin寄存器的情况
		bgc->reg_dir = dirin;
		bgc->gc.direction_output = bgpio_dir_out_inv;
		bgc->gc.direction_input = bgpio_dir_in_inv;
		bgc->gc.get_direction = bgpio_get_dir_inv;
	} else {
        // 其他情况
		if (flags & BGPIOF_NO_OUTPUT)
			bgc->gc.direction_output = bgpio_dir_out_err;
		else
			bgc->gc.direction_output = bgpio_simple_dir_out;
		bgc->gc.direction_input = bgpio_simple_dir_in;
	}

	return 0;
}

static int bgpio_request(struct gpio_chip *chip, unsigned gpio_pin)
{
	if (gpio_pin < chip->ngpio)
		return 0;

	return -EINVAL;
}

int bgpio_remove(struct bgpio_chip *bgc)
{
	gpiochip_remove(&bgc->gc);
	return 0;
}
EXPORT_SYMBOL_GPL(bgpio_remove);

/* 初始化指定的基础内存映射gpio控制器
 * @bgc     指向要初始化的gpio控制器
 * @dev     指向该gpio控制器关联的device
 * @sz      gpio寄存器长度(32位平台通常就是4) 
 * @dat     pad status寄存器地址
 * @set     data寄存器地址
 * @clr     clear寄存器地址
 * @dirout  输出寄存器地址
 * @dirin   输入寄存器地址
 */
int bgpio_init(struct bgpio_chip *bgc, struct device *dev,
	       unsigned long sz, void __iomem *dat, void __iomem *set,
	       void __iomem *clr, void __iomem *dirout, void __iomem *dirin,
	       unsigned long flags)
{
	int ret;

	if (!is_power_of_2(sz))
		return -EINVAL;

	bgc->bits = sz * 8;
	if (bgc->bits > BITS_PER_LONG)
		return -EINVAL;

	spin_lock_init(&bgc->lock);
	bgc->gc.dev = dev;
	bgc->gc.label = dev_name(dev);
	bgc->gc.base = -1;
	bgc->gc.ngpio = bgc->bits;
	bgc->gc.request = bgpio_request;

    // 为该gpio控制器配置底层寄存器资源和基本的get/set方法
	ret = bgpio_setup_io(bgc, dat, set, clr, flags);
	if (ret)
		return ret;

    // 为该gpio控制器配置底层通用的寄存器read/write方法
	ret = bgpio_setup_accessors(dev, bgc, flags & BGPIOF_BIG_ENDIAN,
				    flags & BGPIOF_BIG_ENDIAN_BYTE_ORDER);
	if (ret)
		return ret;

    // 为该gpio控制器配置底层方向设置寄存器资源和相关的方向设置方法
	ret = bgpio_setup_direction(bgc, dirout, dirin, flags);
	if (ret)
		return ret;

    // 记录当前的data和dir值
	bgc->data = bgc->read_reg(bgc->reg_dat);
	if (bgc->gc.set == bgpio_set_set &&
			!(flags & BGPIOF_UNREADABLE_REG_SET))
		bgc->data = bgc->read_reg(bgc->reg_set);
	if (bgc->reg_dir && !(flags & BGPIOF_UNREADABLE_REG_DIR))
		bgc->dir = bgc->read_reg(bgc->reg_dir);

	return ret;
}
EXPORT_SYMBOL_GPL(bgpio_init);

#ifdef CONFIG_GPIO_GENERIC_PLATFORM

static void __iomem *bgpio_map(struct platform_device *pdev,
			       const char *name,
			       resource_size_t sane_sz)
{
	struct resource *r;
	resource_size_t sz;

	r = platform_get_resource_byname(pdev, IORESOURCE_MEM, name);
	if (!r)
		return NULL;

	sz = resource_size(r);
	if (sz != sane_sz)
		return IOMEM_ERR_PTR(-EINVAL);

	return devm_ioremap_resource(&pdev->dev, r);
}

static int bgpio_pdev_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource *r;
	void __iomem *dat;
	void __iomem *set;
	void __iomem *clr;
	void __iomem *dirout;
	void __iomem *dirin;
	unsigned long sz;
	unsigned long flags = pdev->id_entry->driver_data;
	int err;
	struct bgpio_chip *bgc;
	struct bgpio_pdata *pdata = dev_get_platdata(dev);

	r = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dat");
	if (!r)
		return -EINVAL;

	sz = resource_size(r);

	dat = bgpio_map(pdev, "dat", sz);
	if (IS_ERR(dat))
		return PTR_ERR(dat);

	set = bgpio_map(pdev, "set", sz);
	if (IS_ERR(set))
		return PTR_ERR(set);

	clr = bgpio_map(pdev, "clr", sz);
	if (IS_ERR(clr))
		return PTR_ERR(clr);

	dirout = bgpio_map(pdev, "dirout", sz);
	if (IS_ERR(dirout))
		return PTR_ERR(dirout);

	dirin = bgpio_map(pdev, "dirin", sz);
	if (IS_ERR(dirin))
		return PTR_ERR(dirin);

	bgc = devm_kzalloc(&pdev->dev, sizeof(*bgc), GFP_KERNEL);
	if (!bgc)
		return -ENOMEM;

	err = bgpio_init(bgc, dev, sz, dat, set, clr, dirout, dirin, flags);
	if (err)
		return err;

	if (pdata) {
		if (pdata->label)
			bgc->gc.label = pdata->label;
		bgc->gc.base = pdata->base;
		if (pdata->ngpio > 0)
			bgc->gc.ngpio = pdata->ngpio;
	}

	platform_set_drvdata(pdev, bgc);

	return gpiochip_add(&bgc->gc);
}

static int bgpio_pdev_remove(struct platform_device *pdev)
{
	struct bgpio_chip *bgc = platform_get_drvdata(pdev);

	return bgpio_remove(bgc);
}

static const struct platform_device_id bgpio_id_table[] = {
	{
		.name		= "basic-mmio-gpio",
		.driver_data	= 0,
	}, {
		.name		= "basic-mmio-gpio-be",
		.driver_data	= BGPIOF_BIG_ENDIAN,
	},
	{ }
};
MODULE_DEVICE_TABLE(platform, bgpio_id_table);

static struct platform_driver bgpio_driver = {
	.driver = {
		.name = "basic-mmio-gpio",
	},
	.id_table = bgpio_id_table,
	.probe = bgpio_pdev_probe,
	.remove = bgpio_pdev_remove,
};

module_platform_driver(bgpio_driver);

#endif /* CONFIG_GPIO_GENERIC_PLATFORM */

MODULE_DESCRIPTION("Driver for basic memory-mapped GPIO controllers");
MODULE_AUTHOR("Anton Vorontsov <cbouatmailru@gmail.com>");
MODULE_LICENSE("GPL");
