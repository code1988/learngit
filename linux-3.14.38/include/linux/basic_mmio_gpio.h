/*
 * Basic memory-mapped GPIO controllers.
 *
 * Copyright 2008 MontaVista Software, Inc.
 * Copyright 2008,2010 Anton Vorontsov <cbouatmailru@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#ifndef __BASIC_MMIO_GPIO_H
#define __BASIC_MMIO_GPIO_H

#include <linux/gpio.h>
#include <linux/types.h>
#include <linux/compiler.h>
#include <linux/spinlock_types.h>

struct bgpio_pdata {
	int base;
	int ngpio;
};

struct device;

// 定义了基础内存映射gpio控制器模型
struct bgpio_chip {
	struct gpio_chip gc;    // 封装的gpio控制器基类

	unsigned long (*read_reg)(void __iomem *reg);               // 通用的读寄存器接口
	void (*write_reg)(void __iomem *reg, unsigned long data);   // 通用的写寄存器接口

	void __iomem *reg_dat;  // 指向一个获取数据类寄存器,imx6ul平台指向GPIO_PSR寄存器
	void __iomem *reg_set;  // 指向一个设置类寄存器,imx6ul平台指向GPIO_DR寄存器
	void __iomem *reg_clr;  // 指向一个清除类寄存器,imx6ul平台没有使用
	void __iomem *reg_dir;  // 指向一个设置方向类寄存器,imx6ul平台指向GPIO_GDR寄存器

	/* Number of bits (GPIOs): <register width> * 8. 
     * 每个gpio寄存器位数,32位平台下通常就是32位
     * */
	int bits;

	/*
	 * Some GPIO controllers work with the big-endian bits notation,
	 * e.g. in a 8-bits register, GPIO7 is the least significant bit.
     * 指向gpio编号到对应寄存器位的映射关系
	 */
	unsigned long (*pin2mask)(struct bgpio_chip *bgc, unsigned int pin);

	/*
	 * Used to lock bgpio_chip->data. Also, this is needed to keep
	 * shadowed and real data registers writes together.
	 */
	spinlock_t lock;

	/* Shadowed data register to clear/set bits safely. 
     * 跟踪记录设置类/获取数据类寄存器的数据
     * */
	unsigned long data;

	/* Shadowed direction registers to clear/set direction safely. 
     * 跟踪记录设置方向类寄存器的数据
     * */
	unsigned long dir;
};

static inline struct bgpio_chip *to_bgpio_chip(struct gpio_chip *gc)
{
	return container_of(gc, struct bgpio_chip, gc);
}

int bgpio_remove(struct bgpio_chip *bgc);
int bgpio_init(struct bgpio_chip *bgc, struct device *dev,
	       unsigned long sz, void __iomem *dat, void __iomem *set,
	       void __iomem *clr, void __iomem *dirout, void __iomem *dirin,
	       unsigned long flags);

#define BGPIOF_BIG_ENDIAN		BIT(0)
#define BGPIOF_UNREADABLE_REG_SET	BIT(1) /* reg_set is unreadable */
#define BGPIOF_UNREADABLE_REG_DIR	BIT(2) /* reg_dir is unreadable */
#define BGPIOF_BIG_ENDIAN_BYTE_ORDER	BIT(3)

#endif /* __BASIC_MMIO_GPIO_H */
