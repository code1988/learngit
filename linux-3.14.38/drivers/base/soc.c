/*
 * Copyright (C) ST-Ericsson SA 2011
 *
 * Author: Lee Jones <lee.jones@linaro.org> for ST-Ericsson.
 * License terms:  GNU General Public License (GPL), version 2
 */

#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/slab.h>
#include <linux/idr.h>
#include <linux/spinlock.h>
#include <linux/sys_soc.h>
#include <linux/err.h>

static DEFINE_IDA(soc_ida);
static DEFINE_SPINLOCK(soc_lock);

static ssize_t soc_info_get(struct device *dev,
			    struct device_attribute *attr,
			    char *buf);

// 定义了soc设备模型，顾名思义，是将整个片上系统整体抽象
struct soc_device {
	struct device dev;  // 封装的linux基本设备结构
	struct soc_device_attribute *attr;  // 指向对应的soc属性集合
	int soc_dev_num;    // soc设备的唯一编码
};

// 定义了一条soc总线
static struct bus_type soc_bus_type = {
	.name  = "soc",
};

static DEVICE_ATTR(machine,  S_IRUGO, soc_info_get,  NULL);
static DEVICE_ATTR(family,   S_IRUGO, soc_info_get,  NULL);
static DEVICE_ATTR(soc_id,   S_IRUGO, soc_info_get,  NULL);
static DEVICE_ATTR(revision, S_IRUGO, soc_info_get,  NULL);

// 返回soc设备对应的通用意义上设备对象
struct device *soc_device_to_device(struct soc_device *soc_dev)
{
	return &soc_dev->dev;
}

static umode_t soc_attribute_mode(struct kobject *kobj,
                                 struct attribute *attr,
                                 int index)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct soc_device *soc_dev = container_of(dev, struct soc_device, dev);

	if ((attr == &dev_attr_machine.attr)
	    && (soc_dev->attr->machine != NULL))
		return attr->mode;
	if ((attr == &dev_attr_family.attr)
	    && (soc_dev->attr->family != NULL))
		return attr->mode;
	if ((attr == &dev_attr_revision.attr)
	    && (soc_dev->attr->revision != NULL))
		return attr->mode;
	if ((attr == &dev_attr_soc_id.attr)
	    && (soc_dev->attr->soc_id != NULL))
	        return attr->mode;

	/* Unknown or unfilled attribute. */
	return 0;
}

static ssize_t soc_info_get(struct device *dev,
			    struct device_attribute *attr,
			    char *buf)
{
	struct soc_device *soc_dev = container_of(dev, struct soc_device, dev);

	if (attr == &dev_attr_machine)
		return sprintf(buf, "%s\n", soc_dev->attr->machine);
	if (attr == &dev_attr_family)
		return sprintf(buf, "%s\n", soc_dev->attr->family);
	if (attr == &dev_attr_revision)
		return sprintf(buf, "%s\n", soc_dev->attr->revision);
	if (attr == &dev_attr_soc_id)
		return sprintf(buf, "%s\n", soc_dev->attr->soc_id);

	return -EINVAL;

}

static struct attribute *soc_attr[] = {
	&dev_attr_machine.attr,
	&dev_attr_family.attr,
	&dev_attr_soc_id.attr,
	&dev_attr_revision.attr,
	NULL,
};

// 定义了一个soc属性组
static const struct attribute_group soc_attr_group = {
	.attrs = soc_attr,
	.is_visible = soc_attribute_mode,
};

// 定义了一张记录所有soc属性组的表（显然，3.14.38中，只有一个表项）
static const struct attribute_group *soc_attr_groups[] = {
	&soc_attr_group,
	NULL,
};

// 释放device所在的soc_device结构
static void soc_release(struct device *dev)
{
	struct soc_device *soc_dev = container_of(dev, struct soc_device, dev);

	kfree(soc_dev);
}

/* 注册soc设备(归根到底，还是要执行通用的设备注册流程)
 * @soc_dev_attr - 用于设置soc设备的属性
 */
struct soc_device *soc_device_register(struct soc_device_attribute *soc_dev_attr)
{
	struct soc_device *soc_dev;
	int ret;

	soc_dev = kzalloc(sizeof(*soc_dev), GFP_KERNEL);
	if (!soc_dev) {
	        ret = -ENOMEM;
		goto out1;
	}

	/* Fetch a unique (reclaimable) SOC ID. 
     * 生成一个唯一的soc ID号
     * */
	do {
		if (!ida_pre_get(&soc_ida, GFP_KERNEL)) {
			ret = -ENOMEM;
			goto out2;
		}

		spin_lock(&soc_lock);
		ret = ida_get_new(&soc_ida, &soc_dev->soc_dev_num);
		spin_unlock(&soc_lock);

	} while (ret == -EAGAIN);

	if (ret)
	         goto out2;

    // 填充新创建的soc设备结构
	soc_dev->attr = soc_dev_attr;
	soc_dev->dev.bus = &soc_bus_type;
	soc_dev->dev.groups = soc_attr_groups;
	soc_dev->dev.release = soc_release;

    // 以"soc + soc唯一编码"的形式设置soc设备的设备名
	dev_set_name(&soc_dev->dev, "soc%d", soc_dev->soc_dev_num);

    // 最后，执行通用的设备注册流程
	ret = device_register(&soc_dev->dev);
	if (ret)
		goto out3;

	return soc_dev;

out3:
	ida_remove(&soc_ida, soc_dev->soc_dev_num);
out2:
	kfree(soc_dev);
out1:
	return ERR_PTR(ret);
}

/* Ensure soc_dev->attr is freed prior to calling soc_device_unregister. */
void soc_device_unregister(struct soc_device *soc_dev)
{
	ida_remove(&soc_ida, soc_dev->soc_dev_num);

	device_unregister(&soc_dev->dev);
}

static int __init soc_bus_register(void)
{
	return bus_register(&soc_bus_type);
}
core_initcall(soc_bus_register);

static void __exit soc_bus_unregister(void)
{
	ida_destroy(&soc_ida);

	bus_unregister(&soc_bus_type);
}
module_exit(soc_bus_unregister);
