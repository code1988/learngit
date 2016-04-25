/*
 * Copyright (C) 2013 Felix Fietkau <nbd@openwrt.org>
 * Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _SWITCHD_H_
#define _SWITCHD_H_

#include <libubox/uloop.h>
#include <libubox/utils.h>
#include <libubus.h>

#include <stdio.h>
#include <syslog.h>

#include "log.h"

#define __init __attribute__((constructor))

extern char *ubus_socket;
extern int upgrade_running;

struct jw_switch_policy {
    char *name;
    enum blobmsg_type type;
    int(*get_handler)(struct blob_buf *buf, int port_idx);
    int(*set_handler)(int port_idx, void *var);
};

//void *jw_switch_get_context(char *, struct jw_switch_policy *, int );
const void *jw_switchd_get_context(char *name, const struct jw_switch_policy *policy_tbl, int item_max);

void switchd_connect_ubus(void);
void switchd_reconnect_ubus(int reconnect);
void ubus_init_boardinfo(struct ubus_context *ctx);

void switchd_state_next(void);
//void switchd_state_ubus_connect(void);
//void switchd_shutdown(int event);
void switchd_signal(void);
void switchd_signal_preinit(void);

void ubus_init_eth_stats(struct ubus_context *ctx);
void ubus_init_port_config(struct ubus_context *ctx);
void ubus_init_port_mirror(struct ubus_context *ctx);
void ubus_init_rate_limit(struct ubus_context *ctx);
void ubus_init_sysinfo(struct ubus_context *ctx);



#endif
