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

#include <unistd.h>
#include <signal.h>

#include <libubox/blobmsg_json.h>
#include "libubus.h"
#include "count.h"

static struct ubus_context *ctx;    // 指向注册服务到ubusd的客户端的总控制块
static struct ubus_subscriber test_event;   // 对象的阅订控制块
static struct blob_buf b;           // 定义一个静态全局BLOB控制块

// 方法hello的参数枚举
enum {
	HELLO_ID,
	HELLO_MSG,
	__HELLO_MAX
};

// 方法hello的策略
static const struct blobmsg_policy hello_policy[] = {
	[HELLO_ID] = { .name = "id", .type = BLOBMSG_TYPE_INT32 },
	[HELLO_MSG] = { .name = "msg", .type = BLOBMSG_TYPE_STRING },
};

// 对象hello的请求控制块
struct hello_request {
	struct ubus_request_data req;   // ubus请求控制块
	struct uloop_timeout timeout;   // 给对象hello的请求控制块定义一个超时控制块
	int fd;
	int idx;
	char data[];
};

static void test_hello_fd_reply(struct uloop_timeout *t)
{
	struct hello_request *req = container_of(t, struct hello_request, timeout);
	char *data;

	data = alloca(strlen(req->data) + 32);
	sprintf(data, "msg%d: %s\n", ++req->idx, req->data);
	if (write(req->fd, data, strlen(data)) < 0) {
		close(req->fd);
		free(req);
		return;
	}

	uloop_timeout_set(&req->timeout, 1000);
}

// 对象hello请求超时后的回调函数
static void test_hello_reply(struct uloop_timeout *t)
{
	struct hello_request *req = container_of(t, struct hello_request, timeout); // 通过偏移定时器控制块地址获取父结构helle请求控制块地址
	int fds[2];

	blob_buf_init(&b, 0);                           // BLOB控制块初始化
	blobmsg_add_string(&b, "message", req->data);   // 添加一条string类型消息：key="message",value=req->data
	ubus_send_reply(ctx, &req->req, b.head);        // 发送回应

    // 创建管道
	if (pipe(fds) == -1) {
		fprintf(stderr, "Failed to create pipe\n");
		return;
	}
	ubus_request_set_fd(ctx, &req->req, fds[0]);    // 设置请求控制块中的fd
	ubus_complete_deferred_request(ctx, &req->req, 0);
	req->fd = fds[1];

	req->timeout.cb = test_hello_fd_reply;
	test_hello_fd_reply(t);
}

// 方法名“hello”对应的处理函数
static int test_hello(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct hello_request *hreq;
	struct blob_attr *tb[__HELLO_MAX];
	const char *format = "%s received a message: %s";
	const char *msgstr = "(unknown)";

    // 解析消息，根据预先设置的策略过滤
	blobmsg_parse(hello_policy, ARRAY_SIZE(hello_policy), tb, blob_data(msg), blob_len(msg));

	if (tb[HELLO_MSG])
    {
	    struct blobmsg_hdr *hdr = (struct blobmsg_hdr *) blob_data(tb[HELLO_MSG]);

        // 获取"msg"的值
		msgstr = blobmsg_data(tb[HELLO_MSG]);
    }

    // 组织回复的字符串
	hreq = calloc(1, sizeof(*hreq) + strlen(format) + strlen(obj->name) + strlen(msgstr) + 1);
	sprintf(hreq->data, format, obj->name, msgstr);
    printf("hello request data: %s\n",hreq->data);   // "test received a message: code"

    // ubus延迟请求设置
	ubus_defer_request(ctx, req, &hreq->req);
	hreq->timeout.cb = test_hello_reply;        // 注册请求超时后的回调函数
	uloop_timeout_set(&hreq->timeout, 3000);    // 设置请求超时时间

	return 0;
}

// 方法watch的参数枚举
enum {
	WATCH_ID,
	WATCH_COUNTER,
	__WATCH_MAX
};

// 方法watch的策略
static const struct blobmsg_policy watch_policy[__WATCH_MAX] = {
	[WATCH_ID] = { .name = "id", .type = BLOBMSG_TYPE_INT32 },
	[WATCH_COUNTER] = { .name = "counter", .type = BLOBMSG_TYPE_INT32 },
};

// 阅订者收到删除通知的回调函数
static void
test_handle_remove(struct ubus_context *ctx, struct ubus_subscriber *s,
                   uint32_t id)
{
	fprintf(stderr, "Object %08x went away\n", id);
}

// 阅订者收到普通通知的回调函数
static int
test_notify(struct ubus_context *ctx, struct ubus_object *obj,
	    struct ubus_request_data *req, const char *method,
	    struct blob_attr *msg)
{
#if 0
	char *str;

	str = blobmsg_format_json(msg, true);
	fprintf(stderr, "Received notification '%s': %s\n", method, str);
	free(str);
#endif

	return 0;
}

/* 这是对象"test"的方法"watch"的回调函数
 * 执行该回调函数，表示server进程将阅订指定id对应的对象上的服务
 */
static int test_watch(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__WATCH_MAX];
	int ret;

	blobmsg_parse(watch_policy, __WATCH_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[WATCH_ID])
		return UBUS_STATUS_INVALID_ARGUMENT;

    char *msgstr;
	if (tb[WATCH_ID])
    {
	    struct blobmsg_hdr *hdr = (struct blobmsg_hdr *) blob_data(tb[WATCH_ID]);
        printf("blobmsg_hdr_len: %d, name: %s\n",hdr->namelen,hdr->name);

        // 获取"id"的值
		msgstr = blobmsg_data(tb[WATCH_ID]);
        printf("msgstr: %d\n",*(int *)msgstr);
    }

	if (tb[WATCH_COUNTER])
    {
	    struct blobmsg_hdr *hdr = (struct blobmsg_hdr *) blob_data(tb[WATCH_COUNTER]);
        printf("blobmsg_hdr_len: %d, name: %s\n",hdr->namelen,hdr->name);

        // 获取"counter"的值
		msgstr = blobmsg_data(tb[WATCH_COUNTER]);
        printf("msgstr: %d\n",*(int *)msgstr);
    }

	test_event.remove_cb = test_handle_remove;  // 注册通知删除时的回调函数
	test_event.cb = test_notify;                // 注册通知到来时的回调函数
	ret = ubus_subscribe(ctx, &test_event, blobmsg_get_u32(tb[WATCH_ID]));  // 完成阅订
	fprintf(stderr, "Watching object %08x: %s\n", blobmsg_get_u32(tb[WATCH_ID]), ubus_strerror(ret));
	return ret;
}

enum {
    COUNT_TO,
    COUNT_STRING,
    __COUNT_MAX
};

static const struct blobmsg_policy count_policy[__COUNT_MAX] = {
    [COUNT_TO] = { .name = "to", .type = BLOBMSG_TYPE_INT32 },
    [COUNT_STRING] = { .name = "string", .type = BLOBMSG_TYPE_STRING },
};

static int test_count(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__COUNT_MAX];
	char *s1, *s2;
	uint32_t num;

    printf(">>>>>>>>>>>>>>> method: %s\n",method);
	blobmsg_parse(count_policy, __COUNT_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[COUNT_TO] || !tb[COUNT_STRING])
		return UBUS_STATUS_INVALID_ARGUMENT;

	num = blobmsg_get_u32(tb[COUNT_TO]);
	s1 = blobmsg_get_string(tb[COUNT_STRING]);
	s2 = count_to_number(num);
	if (!s1 || !s2) {
		free(s2);
		return UBUS_STATUS_UNKNOWN_ERROR;
	}
	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "rc", strcmp(s1, s2));
	ubus_send_reply(ctx, req, b.head);
	free(s2);

	return 0;
}

// 测试例子-对象的方法的集合(这里定义了3条方法)
static const struct ubus_method test_methods[] = {
	UBUS_METHOD("hello", test_hello, hello_policy),
	UBUS_METHOD("watch", test_watch, watch_policy),
	UBUS_METHOD("count", test_count, count_policy),
};

// 测试例子-对象的类型
static struct ubus_object_type test_object_type =
	UBUS_OBJECT_TYPE("test", test_methods);

// 测试例子-对象
static struct ubus_object test_object = {
	.name = "test",
	.type = &test_object_type,
	.methods = test_methods,
	.n_methods = ARRAY_SIZE(test_methods),
};

// 客户端注册服务
static void server_main(void)
{
	int ret;

    // 注册对象：test_object
	ret = ubus_add_object(ctx, &test_object);
	if (ret)
		fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));

    // 注册阅订控制块：test_event
	ret = ubus_register_subscriber(ctx, &test_event);
	if (ret)
		fprintf(stderr, "Failed to add watch handler: %s\n", ubus_strerror(ret));

	uloop_run();
}

int main(int argc, char **argv)
{
	const char *ubus_socket = NULL;
	int ch;

    // 获取命令行参数
	while ((ch = getopt(argc, argv, "cs:")) != -1) {
		switch (ch) {
		case 's':
			ubus_socket = optarg;
			break;
		default:
			break;
		}
	}

	argc -= optind;
	argv += optind;

	uloop_init();               // 创建epoll句柄
	signal(SIGPIPE, SIG_IGN);   // 防止对端关闭后本端程序退出

    // 创建ubus客户端并发起连接（五层封装）
	ctx = ubus_connect(ubus_socket);
	if (!ctx) {
		fprintf(stderr, "Failed to connect to ubus\n");
		return -1;
	}

    // 到这里意味着ubus客户端成功连接ubusd,注册fd到epoll
	ubus_add_uloop(ctx);

    // 客户端注册服务
	server_main();

	ubus_free(ctx);
	uloop_done();

	return 0;
}
