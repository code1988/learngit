/*
 * netifd - network interface daemon
 * Copyright (C) 2012 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <uci.h>

#include "netifd.h"
#include "interface.h"
#include "interface-ip.h"
#include "iprule.h"
#include "proto.h"
#include "wireless.h"
#include "config.h"

bool config_init = false;

static struct uci_context *uci_ctx;
static struct uci_package *uci_network;     // 指向/etc/config/network句柄
static struct uci_package *uci_wireless;    // 指向/etc/config/wireless句柄
static struct blob_buf b;

static int
config_section_idx(struct uci_section *s)
{
	struct uci_element *e;
	int idx = 0;

	uci_foreach_element(&uci_wireless->sections, e) {
		struct uci_section *cur = uci_to_section(e);

		if (s == cur)
			return idx;

		if (!strcmp(cur->type, s->type))
			idx++;
	}

	return -1;
}

static int
config_parse_bridge_interface(struct uci_section *s)
{
	char *name;

	name = alloca(strlen(s->e.name) + 4);
	sprintf(name, "br-%s", s->e.name);
	blobmsg_add_string(&b, "name", name);

	uci_to_blob(&b, s, bridge_device_type.config_params);
	if (!device_create(name, &bridge_device_type, b.head)) {
		D(INTERFACE, "Failed to create bridge for interface '%s'\n", s->e.name);
		return -EINVAL;
	}

	blob_buf_init(&b, 0);
	blobmsg_add_string(&b, "ifname", name);
	return 0;
}

// 解析UCI文件network中的名为interface的section
static void
config_parse_interface(struct uci_section *s, bool alias)
{
	struct interface *iface;
	const char *type = NULL, *disabled;
	struct blob_attr *config;
	bool bridge = false;

	disabled = uci_lookup_option_string(uci_ctx, s, "disabled");
	if (disabled && !strcmp(disabled, "1"))
		return;

	blob_buf_init(&b, 0);

	if (!alias)
		type = uci_lookup_option_string(uci_ctx, s, "type");
	if (type && !strcmp(type, "bridge")) {
		if (config_parse_bridge_interface(s))
			return;

		bridge = true;
	}

	uci_to_blob(&b, s, &interface_attr_list);

	iface = interface_alloc(s->e.name, b.head);
	if (!iface)
		return;

	if (iface->proto_handler && iface->proto_handler->config_params)
		uci_to_blob(&b, s, iface->proto_handler->config_params);

	if (!bridge && uci_to_blob(&b, s, simple_device_type.config_params))
		iface->device_config = true;

	config = blob_memdup(b.head);
	if (!config)
		goto error;

	if (alias) {
		if (!interface_add_alias(iface, config))
			goto error_free_config;
	} else {
		interface_add(iface, config);
	}
	return;

error_free_config:
	free(config);
error:
	free(iface);
}

static void
config_parse_route(struct uci_section *s, bool v6)
{
	void *route;

	blob_buf_init(&b, 0);
	route = blobmsg_open_array(&b, "route");
	uci_to_blob(&b, s, &route_attr_list);
	blobmsg_close_array(&b, route);
	interface_ip_add_route(NULL, blob_data(b.head), v6);
}

static void
config_parse_rule(struct uci_section *s, bool v6)
{
	void *rule;

	blob_buf_init(&b, 0);
	rule = blobmsg_open_array(&b, "rule");
	uci_to_blob(&b, s, &rule_attr_list);
	blobmsg_close_array(&b, rule);
	iprule_add(blob_data(b.head), v6);
}

static void
config_init_devices(void)
{
	struct uci_element *e;

	uci_foreach_element(&uci_network->sections, e) {
		const struct uci_blob_param_list *params = NULL;
		struct uci_section *s = uci_to_section(e);
		const struct device_type *devtype = NULL;
		struct device *dev;
		const char *type, *name;

		if (strcmp(s->type, "device") != 0)
			continue;

		name = uci_lookup_option_string(uci_ctx, s, "name");
		if (!name)
			continue;

		type = uci_lookup_option_string(uci_ctx, s, "type");
		if (type) {
			if (!strcmp(type, "8021ad"))
				devtype = &vlandev_device_type;
			else if (!strcmp(type, "8021q"))
				devtype = &vlandev_device_type;
			else if (!strcmp(type, "bridge"))
				devtype = &bridge_device_type;
			else if (!strcmp(type, "macvlan"))
				devtype = &macvlan_device_type;
			else if (!strcmp(type, "tunnel"))
				devtype = &tunnel_device_type;
		}

		if (devtype)
			params = devtype->config_params;
		if (!params)
			params = simple_device_type.config_params;

		blob_buf_init(&b, 0);
		uci_to_blob(&b, s, params);
		if (devtype) {
			dev = device_create(name, devtype, b.head);
			if (!dev)
				continue;
		} else {
			dev = device_get(name, 1);
			if (!dev)
				continue;

			dev->current_config = true;
			device_apply_config(dev, dev->type, b.head);
		}
		dev->default_config = false;
	}
}

// 打开/etc/config目录下名为config的UCI文件，返回对应的句柄
static struct uci_package *
config_init_package(const char *config)
{
	struct uci_context *ctx = uci_ctx;
	struct uci_package *p = NULL;

	if (!ctx) {
		ctx = uci_alloc_context();
		uci_ctx = ctx;

		ctx->flags &= ~UCI_FLAG_STRICT;
		if (config_path)
			uci_set_confdir(ctx, config_path);

#ifdef DUMMY_MODE
		uci_set_savedir(ctx, "./tmp");
#endif
	} else {
		p = uci_lookup_package(ctx, config);
		if (p)
			uci_unload(ctx, p);
	}

	if (uci_load(ctx, config, &p))
		return NULL;

	return p;
}

// 读取UCI文件network中配置的所有名为interface的section，来生成对应的interface节点
static void
config_init_interfaces(void)
{
	struct uci_element *e;

    // 遍历每个名为interface的section，创建对应的interface节点
	uci_foreach_element(&uci_network->sections, e) {
		struct uci_section *s = uci_to_section(e);

		if (!strcmp(s->type, "interface"))
			config_parse_interface(s, false);
	}

	uci_foreach_element(&uci_network->sections, e) {
		struct uci_section *s = uci_to_section(e);

		if (!strcmp(s->type, "alias"))
			config_parse_interface(s, true);
	}
}

static void
config_init_routes(void)
{
	struct interface *iface;
	struct uci_element *e;

	vlist_for_each_element(&interfaces, iface, node)
		interface_ip_update_start(&iface->config_ip);

	uci_foreach_element(&uci_network->sections, e) {
		struct uci_section *s = uci_to_section(e);

		if (!strcmp(s->type, "route"))
			config_parse_route(s, false);
		else if (!strcmp(s->type, "route6"))
			config_parse_route(s, true);
	}

	vlist_for_each_element(&interfaces, iface, node)
		interface_ip_update_complete(&iface->config_ip);
}

static void
config_init_rules(void)
{
	struct uci_element *e;

	iprule_update_start();

	uci_foreach_element(&uci_network->sections, e) {
		struct uci_section *s = uci_to_section(e);

		if (!strcmp(s->type, "rule"))
			config_parse_rule(s, false);
		else if (!strcmp(s->type, "rule6"))
			config_parse_rule(s, true);
	}

	iprule_update_complete();
}

static void
config_init_globals(void)
{
	struct uci_section *globals = uci_lookup_section(
			uci_ctx, uci_network, "globals");
	if (!globals)
		return;

	const char *ula_prefix = uci_lookup_option_string(
			uci_ctx, globals, "ula_prefix");
	interface_ip_set_ula_prefix(ula_prefix);

	const char *default_ps = uci_lookup_option_string(
			uci_ctx, globals, "default_ps");

	if (default_ps)
		device_set_default_ps(strcmp(default_ps, "1") ? false : true);
}

static void
config_parse_wireless_device(struct uci_section *s)
{
	struct wireless_driver *drv;
	const char *driver_name;

	driver_name = uci_lookup_option_string(uci_ctx, s, "type");
	if (!driver_name)
		return;

	drv = avl_find_element(&wireless_drivers, driver_name, drv, node);
	if (!drv)
		return;

	blob_buf_init(&b, 0);
	uci_to_blob(&b, s, drv->device.config);
	wireless_device_create(drv, s->e.name, b.head);
}

static void
config_parse_wireless_interface(struct wireless_device *wdev, struct uci_section *s)
{
	char *name;

	name = alloca(strlen(s->type) + 16);
	sprintf(name, "@%s[%d]", s->type, config_section_idx(s));

	blob_buf_init(&b, 0);
	uci_to_blob(&b, s, wdev->drv->interface.config);
	wireless_interface_create(wdev, b.head, s->anonymous ? name : s->e.name);
}

static void
config_init_wireless(void)
{
	struct wireless_device *wdev;
	struct uci_element *e;
	const char *dev_name;

	if (!uci_wireless) {
		DPRINTF("No wireless configuration found\n");
		return;
	}

	vlist_update(&wireless_devices);

	uci_foreach_element(&uci_wireless->sections, e) {
		struct uci_section *s = uci_to_section(e);
		if (strcmp(s->type, "wifi-device") != 0)
			continue;

		config_parse_wireless_device(s);
	}

	vlist_flush(&wireless_devices);

	vlist_for_each_element(&wireless_devices, wdev, node) {
		wdev->vif_idx = 0;
		vlist_update(&wdev->interfaces);
	}

	uci_foreach_element(&uci_wireless->sections, e) {
		struct uci_section *s = uci_to_section(e);

		if (strcmp(s->type, "wifi-iface") != 0)
			continue;

		dev_name = uci_lookup_option_string(uci_ctx, s, "device");
		if (!dev_name)
			continue;

		wdev = vlist_find(&wireless_devices, dev_name, wdev, node);
		if (!wdev) {
			DPRINTF("device %s not found!\n", dev_name);
			continue;
		}

		config_parse_wireless_interface(wdev, s);
	}

	vlist_for_each_element(&wireless_devices, wdev, node)
		vlist_flush(&wdev->interfaces);
}

void
config_init_all(void)
{
    // 打开UCI文件/etc/config/network
	uci_network = config_init_package("network");
	if (!uci_network) {
		fprintf(stderr, "Failed to load network config\n");
		return;
	}

    // 打开UCI文件/etc/config/wireless
	uci_wireless = config_init_package("wireless");

	vlist_update(&interfaces);
	config_init = true;
	device_lock();

	device_reset_config();
	config_init_devices();
	config_init_interfaces();   // 初始化interface模块
	config_init_routes();
	config_init_rules();
	config_init_globals();
	config_init_wireless();

	config_init = false;
	device_unlock();

	device_reset_old();
	device_init_pending();
	vlist_flush(&interfaces);
	device_free_unused(NULL);
	interface_refresh_assignments(false);
	interface_start_pending();
	wireless_start_pending();
}
