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

#ifndef __CRPP_H
#define __CRPP_H

#include <stdio.h>
#include <syslog.h>


#include <libubox/uloop.h>
#include <libubox/utils.h>
#include <uci.h>
#include <json-c/json.h>

#include "log.h"

#define __init __attribute__((constructor))
#define PORT_NUM 10

#define IS_EMPTY_MAC(mac) ((mac[0] + mac[1] + mac[2] + mac[3] + mac[4] + mac[5]) == 0) 
#define PORT_IDX    "port-"
#define ENTRY_IDX   "idx-"
#define OCTET_NUM   (PORT_NUM/8 + ((PORT_NUM%8)?1:0))

typedef enum {
    CRPP_ENABLED,
    CRPP_DISABLED
}eCrppState;

typedef struct {
    eCrppState state;  
    struct list_head fd_l;
    void (*notify_cb)(struct list_head *head);
}crpp_t;

#endif
