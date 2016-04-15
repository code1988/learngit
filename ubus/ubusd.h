/*
 * Copyright (C) 2011-2014 Felix Fietkau <nbd@openwrt.org>
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

#ifndef __UBUSD_H
#define __UBUSD_H

#include <libubox/list.h>
#include <libubox/uloop.h>
#include <libubox/blobmsg.h>
#include "ubus_common.h"
#include "ubusd_id.h"
#include "ubusd_obj.h"
#include "ubusmsg.h"

#define UBUSD_CLIENT_BACKLOG	32
#define UBUS_OBJ_HASH_BITS	4

extern struct blob_buf b;

// ubus消息控制块
struct ubus_msg_buf {
	uint32_t refcount;          // 属性空间类型标记（~0:使用外部共享空间 1：使用内部分配空间）/* ~0: uses external data buffer */
	struct ubus_msghdr hdr;     // 消息头
	struct blob_attr *data;     // 指向BOLB属性空间(也就是消息内容)
	int fd;
	int len;                    // 记录BLOB属性空间长度（也就是消息内容长度）
};

// 定义客户端链表节点
struct ubus_client {
	struct ubus_id id;          // ID用于标识不同的客户端
	struct uloop_fd sock;       // 客户端fd控制块

	struct list_head objects;   // 链表模块

	struct ubus_msg_buf *tx_queue[UBUSD_CLIENT_BACKLOG];    // 待发送的消息池
	unsigned int txq_cur, txq_tail, txq_ofs;    // txq_cur:发送中的消息索引号 txq_tail:最后一个消息索引号 txq_ofs:记录已经发送的消息长度

	struct ubus_msg_buf *pending_msg;
	int pending_msg_offset;
	int pending_msg_fd;         // -1表示复位状态
	struct {
		struct ubus_msghdr hdr;
		struct blob_attr data;
	} hdrbuf;
};

struct ubus_path {
	struct list_head list;
	const char name[];
};

struct ubus_msg_buf *ubus_msg_new(void *data, int len, bool shared);
void ubus_msg_send(struct ubus_client *cl, struct ubus_msg_buf *ub, bool free);
void ubus_msg_free(struct ubus_msg_buf *ub);

struct ubus_client *ubusd_proto_new_client(int fd, uloop_fd_handler cb);
void ubusd_proto_receive_message(struct ubus_client *cl, struct ubus_msg_buf *ub);
void ubusd_proto_free_client(struct ubus_client *cl);

void ubusd_event_init(void);
void ubusd_event_cleanup_object(struct ubus_object *obj);
void ubusd_send_obj_event(struct ubus_object *obj, bool add);


#endif
