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
 * 特别要注意事项：
 *  一个ubus客户端控制块(ubus_context)上同时只能操作一个ubus对象，
 *  也就是说，在本次ubus对象操作未完成期间，绝不允许对一个新的ubus对象发起操作，因为会导致原有对象空间数据被flush掉，甚至导致进程崩溃
 */

#ifndef __LIBUBUS_H
#define __LIBUBUS_H

#include <libubox/avl.h>
#include <libubox/list.h>
#include <libubox/blobmsg.h>
#include <libubox/uloop.h>
#include <stdint.h>
#include "ubusmsg.h"
#include "ubus_common.h"

#define UBUS_MAX_NOTIFY_PEERS	16

struct ubus_context;
struct ubus_msg_src;
struct ubus_object;
struct ubus_request;
struct ubus_request_data;
struct ubus_object_data;
struct ubus_event_handler;
struct ubus_subscriber;
struct ubus_notify_request;

// 消息控制块
struct ubus_msghdr_buf {
	struct ubus_msghdr hdr; // 消息头
	struct blob_attr *data; // 指向分配的消息空间
};

typedef void (*ubus_lookup_handler_t)(struct ubus_context *ctx,
				      struct ubus_object_data *obj,
				      void *priv);
typedef int (*ubus_handler_t)(struct ubus_context *ctx, struct ubus_object *obj,
			      struct ubus_request_data *req,
			      const char *method, struct blob_attr *msg);
typedef void (*ubus_state_handler_t)(struct ubus_context *ctx, struct ubus_object *obj);
typedef void (*ubus_remove_handler_t)(struct ubus_context *ctx,
				      struct ubus_subscriber *obj, uint32_t id);
typedef void (*ubus_event_handler_t)(struct ubus_context *ctx, struct ubus_event_handler *ev,
				     const char *type, struct blob_attr *msg);
typedef void (*ubus_data_handler_t)(struct ubus_request *req,
				    int type, struct blob_attr *msg);
typedef void (*ubus_fd_handler_t)(struct ubus_request *req, int fd);
typedef void (*ubus_complete_handler_t)(struct ubus_request *req, int ret);
typedef void (*ubus_notify_complete_handler_t)(struct ubus_notify_request *req,
					       int idx, int ret);
typedef void (*ubus_connect_handler_t)(struct ubus_context *ctx);

//给一个对象类型ubus_object_type的成员赋值
#define UBUS_OBJECT_TYPE(_name, _methods)		\
	{						\
		.name = _name,				\
		.id = 0,				\
		.n_methods = ARRAY_SIZE(_methods),	\
		.methods = _methods			\
	}

#define __UBUS_METHOD_NOARG(_name, _handler)		\
	.name = _name,					\
	.handler = _handler

#define __UBUS_METHOD(_name, _handler, _policy)		\
	__UBUS_METHOD_NOARG(_name, _handler),		\
	.policy = _policy,				\
	.n_policy = ARRAY_SIZE(_policy)

#define UBUS_METHOD(_name, _handler, _policy)		\
	{ __UBUS_METHOD(_name, _handler, _policy) }

#define UBUS_METHOD_MASK(_name, _handler, _policy, _mask) \
	{						\
		__UBUS_METHOD(_name, _handler, _policy),\
		.mask = _mask				\
	}

#define UBUS_METHOD_NOARG(_name, _handler)		\
	{ __UBUS_METHOD_NOARG(_name, _handler) }

// 方法
struct ubus_method {
	const char *name;       // 方法名
	ubus_handler_t handler; // 对应的处理函数

	unsigned long mask;
	const struct blobmsg_policy *policy;
	int n_policy;
};

// 对象类型
struct ubus_object_type {
	const char *name;                   // 对象名
	uint32_t id;

	const struct ubus_method *methods;  // 指向方法控制块数组
	int n_methods;                      // 记录方法控制块数组的元素数量
};

// ubus对象
struct ubus_object {
	struct avl_node avl;            // avl树，所有ubus对象都通过它管理

	const char *name;               // 对象名
	uint32_t id;                    // 每个ubus对象都分配有一个唯一ID，avl树就是基于这个ID对所有ubus对象进行管理

	const char *path;
	struct ubus_object_type *type;      // 对象类型

	ubus_state_handler_t subscribe_cb;  // 提供阅订服务者使用，当本服务有被阅订时触发此回调
	bool has_subscribers;               // 提供阅订服务者使用，记录了本服务当前被阅订的数量

	const struct ubus_method *methods;  // 指向方法
	int n_methods;      // 记录方法的数量
};

// 阅订控制块
struct ubus_subscriber {
	struct ubus_object obj; // 对象

	ubus_handler_t cb;                  // 普通通知的回调函数
	ubus_remove_handler_t remove_cb;    // 删除通知的回调函数
};

struct ubus_event_handler {
	struct ubus_object obj;

	ubus_event_handler_t cb;
};

/* ubus客户端控制块
 * 对于全局形式的ubus_context变量，一定要注意不能嵌套使用，切记！！
 */
struct ubus_context {
	struct list_head requests;
	struct avl_tree objects;            // avl树控制块
	struct list_head pending;

	struct uloop_fd sock;               // 需要被epoll监听的fd控制块
	struct uloop_timeout pending_timer; // 客户端超时回调函数

	uint32_t local_id;
	uint16_t request_seq;
	int stack_depth;

	void (*connection_lost)(struct ubus_context *ctx);

	struct ubus_msghdr_buf msgbuf;  // 消息控制块
	uint32_t msgbuf_data_len;       // 记录消息空间大小
	int msgbuf_reduction_counter;
};

struct ubus_object_data {
	uint32_t id;
	uint32_t type_id;
	const char *path;
	struct blob_attr *signature;
};

// ubus请求数据控制块
struct ubus_request_data {
	uint32_t object;
	uint32_t peer;
	uint16_t seq;

	/* internal use */
	bool deferred;
	int fd;
};

// ubus请求控制块
struct ubus_request {
	struct list_head list;      // 链表头

	struct list_head pending;   // 链表头
	int status_code;
	bool status_msg;
	bool blocked;
	bool cancelled;
	bool notify;

	uint32_t peer;
	uint16_t seq;               // 记录发起的请求数量

	ubus_data_handler_t raw_data_cb;
	ubus_data_handler_t data_cb;
	ubus_fd_handler_t fd_cb;
	ubus_complete_handler_t complete_cb;

	struct ubus_context *ctx;
	void *priv;                 // 对应ubus_invoke函数的入参priv，用于指向用户自定义数据块
};

struct ubus_notify_request {
	struct ubus_request req;

	ubus_notify_complete_handler_t status_cb;
	ubus_notify_complete_handler_t complete_cb;

	uint32_t pending;
	uint32_t id[UBUS_MAX_NOTIFY_PEERS + 1];
};

struct ubus_auto_conn {
	struct ubus_context ctx;
	struct uloop_timeout timer;
	const char *path;
	ubus_connect_handler_t cb;
};

struct ubus_context *ubus_connect(const char *path);
void ubus_auto_connect(struct ubus_auto_conn *conn);
int ubus_reconnect(struct ubus_context *ctx, const char *path);
void ubus_free(struct ubus_context *ctx);

const char *ubus_strerror(int error);

// 客户端注册fd到epoll
static inline void ubus_add_uloop(struct ubus_context *ctx)
{
    // 客户端注册fd到epoll,读触发,(这里调用ULOOP_BLOCKING的逻辑本意也许是说该fd之前已经做了NOBLOCKING操作，这里不用再做，但是写法上值得商榷)
	uloop_fd_add(&ctx->sock, ULOOP_BLOCKING | ULOOP_READ);
}

/* call this for read events on ctx->sock.fd when not using uloop */
static inline void ubus_handle_event(struct ubus_context *ctx)
{
	ctx->sock.cb(&ctx->sock, ULOOP_READ);
}

/* ----------- raw request handling ----------- */

/* wait for a request to complete and return its status */
int ubus_complete_request(struct ubus_context *ctx, struct ubus_request *req,
			  int timeout);

/* complete a request asynchronously */
void ubus_complete_request_async(struct ubus_context *ctx,
				 struct ubus_request *req);

/* abort an asynchronous request */
void ubus_abort_request(struct ubus_context *ctx, struct ubus_request *req);

/* ----------- objects ----------- */

int ubus_lookup(struct ubus_context *ctx, const char *path,
		ubus_lookup_handler_t cb, void *priv);

int ubus_lookup_id(struct ubus_context *ctx, const char *path, uint32_t *id);

/* make an object visible to remote connections */
int ubus_add_object(struct ubus_context *ctx, struct ubus_object *obj);

/* remove the object from the ubus connection */
int ubus_remove_object(struct ubus_context *ctx, struct ubus_object *obj);

/* add a subscriber notifications from another object */
int ubus_register_subscriber(struct ubus_context *ctx, struct ubus_subscriber *obj);

static inline int
ubus_unregister_subscriber(struct ubus_context *ctx, struct ubus_subscriber *obj)
{
	return ubus_remove_object(ctx, &obj->obj);
}

int ubus_subscribe(struct ubus_context *ctx, struct ubus_subscriber *obj, uint32_t id);
int ubus_unsubscribe(struct ubus_context *ctx, struct ubus_subscriber *obj, uint32_t id);

/* ----------- rpc ----------- */

/* invoke a method on a specific object */
int ubus_invoke(struct ubus_context *ctx, uint32_t obj, const char *method,
		struct blob_attr *msg, ubus_data_handler_t cb, void *priv,
		int timeout);

/* asynchronous version of ubus_invoke() */
int ubus_invoke_async(struct ubus_context *ctx, uint32_t obj, const char *method,
		      struct blob_attr *msg, struct ubus_request *req);

/* send a reply to an incoming object method call */
int ubus_send_reply(struct ubus_context *ctx, struct ubus_request_data *req,
		    struct blob_attr *msg);

static inline void ubus_defer_request(struct ubus_context *ctx,
				      struct ubus_request_data *req,
				      struct ubus_request_data *new_req)
{
    memcpy(new_req, req, sizeof(*req));
    req->deferred = true;
}

// 设置请求的fd（目前发现该fd来自管道创建）
static inline void ubus_request_set_fd(struct ubus_context *ctx,
				       struct ubus_request_data *req, int fd)
{
    req->fd = fd;
}

void ubus_complete_deferred_request(struct ubus_context *ctx,
				    struct ubus_request_data *req, int ret);

/*
 * send a notification to all subscribers of an object
 * if timeout < 0, no reply is expected from subscribers
 */
int ubus_notify(struct ubus_context *ctx, struct ubus_object *obj,
		const char *type, struct blob_attr *msg, int timeout);

int ubus_notify_async(struct ubus_context *ctx, struct ubus_object *obj,
		      const char *type, struct blob_attr *msg,
		      struct ubus_notify_request *req);


/* ----------- events ----------- */

int ubus_send_event(struct ubus_context *ctx, const char *id,
		    struct blob_attr *data);

int ubus_register_event_handler(struct ubus_context *ctx,
				struct ubus_event_handler *ev,
				const char *pattern);

static inline int ubus_unregister_event_handler(struct ubus_context *ctx,
						struct ubus_event_handler *ev)
{
    return ubus_remove_object(ctx, &ev->obj);
}

#endif
