/*
 * Device tree integration for the pin control subsystem
 * pin-control子系统相关的dts信息解析接口
 *      属性                属性值              含义
 *      "pinctrl-names"     "xxx", ...          pin-control状态名 / 状态名列表
 *      "pinctrl-x"         <&xxx ...>, ...     一个独立的pin-control状态信息
 *                                              属性值格式表示一个独立的状态可以包含多个phandle(一个phandle表示一条可选的pin配置表项，对应了一个设备节点)
 *
 * Copyright (C) 2012 NVIDIA CORPORATION. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/slab.h>

#include "core.h"
#include "devicetree.h"

/**
 * struct pinctrl_dt_map - mapping table chunk parsed from device tree
 * @node: list node for struct pinctrl's @dt_maps field
 * @pctldev: the pin controller that allocated this struct, and will free it
 * @maps: the mapping table entries
 */
struct pinctrl_dt_map {
	struct list_head node;
	struct pinctrl_dev *pctldev;
	struct pinctrl_map *map;
	unsigned num_maps;
};

static void dt_free_map(struct pinctrl_dev *pctldev,
		     struct pinctrl_map *map, unsigned num_maps)
{
	if (pctldev) {
		const struct pinctrl_ops *ops = pctldev->desc->pctlops;
		ops->dt_free_map(pctldev, map, num_maps);
	} else {
		/* There is no pctldev for PIN_MAP_TYPE_DUMMY_STATE */
		kfree(map);
	}
}

void pinctrl_dt_free_maps(struct pinctrl *p)
{
	struct pinctrl_dt_map *dt_map, *n1;

	list_for_each_entry_safe(dt_map, n1, &p->dt_maps, node) {
		pinctrl_unregister_map(dt_map->map);
		list_del(&dt_map->node);
		dt_free_map(dt_map->pctldev, dt_map->map,
			    dt_map->num_maps);
		kfree(dt_map);
	}

	of_node_put(p->dev->of_node);
}

static int dt_remember_or_free_map(struct pinctrl *p, const char *statename,
				   struct pinctrl_dev *pctldev,
				   struct pinctrl_map *map, unsigned num_maps)
{
	int i;
	struct pinctrl_dt_map *dt_map;

	/* Initialize common mapping table entry fields */
	for (i = 0; i < num_maps; i++) {
		map[i].dev_name = dev_name(p->dev);
		map[i].name = statename;
		if (pctldev)
			map[i].ctrl_dev_name = dev_name(pctldev->dev);
	}

	/* Remember the converted mapping table entries */
	dt_map = kzalloc(sizeof(*dt_map), GFP_KERNEL);
	if (!dt_map) {
		dev_err(p->dev, "failed to alloc struct pinctrl_dt_map\n");
		dt_free_map(pctldev, map, num_maps);
		return -ENOMEM;
	}

	dt_map->pctldev = pctldev;
	dt_map->map = map;
	dt_map->num_maps = num_maps;
	list_add_tail(&dt_map->node, &p->dt_maps);

	return pinctrl_register_map(map, num_maps, false, true);
}

struct pinctrl_dev *of_pinctrl_get(struct device_node *np)
{
	struct pinctrl_dev *pctldev;

	pctldev = get_pinctrl_dev_from_of_node(np);
	if (!pctldev)
		return NULL;

	return pctldev;
}

/* 解析一个记录了pin配置信息的dts节点
 * @p           - 指向一个需要被继续填充的pinctrl句柄
 * @statename   - 指向一个pinctrl状态名
 * @np_config   - 指向一个记录了pin配置信息的dts节点
 */
static int dt_to_map_one_config(struct pinctrl *p, const char *statename,
				struct device_node *np_config)
{
	struct device_node *np_pctldev;
	struct pinctrl_dev *pctldev;
	const struct pinctrl_ops *ops;
	int ret;
	struct pinctrl_map *map;
	unsigned num_maps;

	/* Find the pin controller containing np_config */
	np_pctldev = of_node_get(np_config);
	for (;;) {
		np_pctldev = of_get_next_parent(np_pctldev);
		if (!np_pctldev || of_node_is_root(np_pctldev)) {
			dev_info(p->dev, "could not find pctldev for node %s, deferring probe\n",
				np_config->full_name);
			of_node_put(np_pctldev);
			/* OK let's just assume this will appear later then */
			return -EPROBE_DEFER;
		}
		pctldev = get_pinctrl_dev_from_of_node(np_pctldev);
		if (pctldev)
			break;
		/* Do not defer probing of hogs (circular loop) */
		if (np_pctldev == p->dev->of_node) {
			of_node_put(np_pctldev);
			return -ENODEV;
		}
	}
	of_node_put(np_pctldev);

	/*
	 * Call pinctrl driver to parse device tree node, and
	 * generate mapping table entries
     * 生成对应的映射单元
	 */
	ops = pctldev->desc->pctlops;
	if (!ops->dt_node_to_map) {
		dev_err(p->dev, "pctldev %s doesn't support DT\n",
			dev_name(pctldev->dev));
		return -ENODEV;
	}
	ret = ops->dt_node_to_map(pctldev, np_config, &map, &num_maps);
	if (ret < 0)
		return ret;

	/* Stash the mapping table chunk away for later use */
	return dt_remember_or_free_map(p, statename, pctldev, map, num_maps);
}

// 给指定pin-control状态名创建一个dummy的状态
static int dt_remember_dummy_state(struct pinctrl *p, const char *statename)
{
	struct pinctrl_map *map;

	map = kzalloc(sizeof(*map), GFP_KERNEL);
	if (!map) {
		dev_err(p->dev, "failed to alloc struct pinctrl_map\n");
		return -ENOMEM;
	}

	/* There is no pctldev for PIN_MAP_TYPE_DUMMY_STATE */
	map->type = PIN_MAP_TYPE_DUMMY_STATE;

	return dt_remember_or_free_map(p, statename, NULL, map, 1);
}

static int dt_gpio_assert_pinctrl(struct pinctrl *p)
{
	struct device_node *np = p->dev->of_node;
	enum of_gpio_flags flags;
	int gpio;
	int index = 0;
	int ret;

	if (!of_find_property(np, "pinctrl-assert-gpios", NULL))
		return 0; /* Missing the property, so nothing to be done */

	for (;; index++) {
		gpio = of_get_named_gpio_flags(np, "pinctrl-assert-gpios",
					       index, &flags);
		if (gpio < 0)
			break; /* End of the phandle list */

		if (!gpio_is_valid(gpio))
			return -EINVAL;

		ret = devm_gpio_request_one(p->dev, gpio, GPIOF_OUT_INIT_LOW,
					    NULL);
		if (ret < 0)
			return ret;

		if (flags & OF_GPIO_ACTIVE_LOW)
			continue;

		if (gpio_cansleep(gpio))
			gpio_set_value_cansleep(gpio, 1);
		else
			gpio_set_value(gpio, 1);
	}

	return 0;
}

/* 从指定device的dts节点中提取相应的pinctrl信息
 * p    用于存放提取出来的pinctrl信息的句柄
 */
int pinctrl_dt_to_map(struct pinctrl *p)
{
	struct device_node *np = p->dev->of_node;
	int state, ret;
	char *propname;
	struct property *prop;
	const char *statename;
	const __be32 *list;
	int size, config;
	phandle phandle;
	struct device_node *np_config;

	/* CONFIG_OF enabled, p->dev not instantiated from DT 
     * 如果该device不存在对应的dts节点就没必要继续下去，直接退出
     * */
	if (!np) {
		dev_dbg(p->dev, "no of_node; not parsing pinctrl DT\n");
		return 0;
	}

	ret = dt_gpio_assert_pinctrl(p);
	if (ret) {
		dev_dbg(p->dev, "failed to assert pinctrl setting: %d\n", ret);
		return ret;
	}

	/* We may store pointers to property names within the node */
	of_node_get(np);

	/* For each defined state ID 
     * 遍历该dts节点中定义的每个pin/pin group状态
     * */
	for (state = 0; ; state++) {
		/* Retrieve the pinctrl-* property 
         * 在该dts节点中检索存储了pin/pin group状态的属性条目(从属性名"pinctrl-0"开始检索)
         * */
		propname = kasprintf(GFP_KERNEL, "pinctrl-%d", state);
		prop = of_find_property(np, propname, &size);
		kfree(propname);
		if (!prop)
			break;
		list = prop->value;     // 每个"pinctrl-x"属性中包含的属性值是一张地址表
		size /= sizeof(*list);  // 记录每张地址表中的表项数量

		/* Determine whether pinctrl-names property names the state 
         * 在该dts节点中检索存储了pin/pin group状态名列表的属性条目"pinctrl-names"，并从中获取当前状态对应的状态名
         * */
		ret = of_property_read_string_index(np, "pinctrl-names",
						    state, &statename);
		/*
		 * If not, statename is just the integer state ID. But rather
		 * than dynamically allocate it and have to free it later,
		 * just point part way into the property name for the string.
         * 如果不存在跟当前pin/pin group状态对应的状态名，则缺省使用"pinctrl-"后面的序号作为该状态的状态名
		 */
		if (ret < 0) {
			/* strlen("pinctrl-") == 8 */
			statename = prop->name + 8;
		}

		/* For every referenced pin configuration node in it 
         * 遍历当前pin/pin group状态包含的每条pin操作表项
         * */
		for (config = 0; config < size; config++) {
			phandle = be32_to_cpup(list++);

			/* Look up the pin configuration node 
             * 查找该phandle对应的dts节点
             * */
			np_config = of_find_node_by_phandle(phandle);
			if (!np_config) {
				dev_err(p->dev,
					"prop %s index %i invalid phandle\n",
					prop->name, config);
				ret = -EINVAL;
				goto err;
			}

			/* Parse the node 
             * 解析该phandle对应的dts节点，将得到的信息填充到pinctrl句柄中
             * */
			ret = dt_to_map_one_config(p, statename, np_config);
			of_node_put(np_config);
			if (ret < 0)
				goto err;
		}

		/* No entries in DT? Generate a dummy state table entry 
         * 如果不存在pin操作表项，就创建一个dummy的pin-control状态
         * */
		if (!size) {
			ret = dt_remember_dummy_state(p, statename);
			if (ret < 0)
				goto err;
		}
	}

	return 0;

err:
	pinctrl_dt_free_maps(p);
	return ret;
}
