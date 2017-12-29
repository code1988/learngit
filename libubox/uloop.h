/*
 * uloop - event loop implementation
 *
 * Copyright (C) 2010-2013 Felix Fietkau <nbd@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * 功能描述：轮询加入监听的所有描述符;处理运行过程中的定时器;管理子进程
 * 备注：支持epoll和kqueue作为执行后端
 */
#ifndef _ULOOP_H__
#define _ULOOP_H__

#include <sys/time.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <signal.h>

#if defined(__APPLE__) || defined(__FreeBSD__)
#define USE_KQUEUE
#else
#define USE_EPOLL
#endif

#include "list.h"

struct uloop_fd;
struct uloop_timeout;
struct uloop_process;

typedef void (*uloop_fd_handler)(struct uloop_fd *u, unsigned int events);
typedef void (*uloop_timeout_handler)(struct uloop_timeout *t);
typedef void (*uloop_process_handler)(struct uloop_process *c, int ret);

#define ULOOP_READ		(1 << 0)
#define ULOOP_WRITE		(1 << 1)
#define ULOOP_EDGE_TRIGGER	(1 << 2)
#define ULOOP_BLOCKING		(1 << 3)

#define ULOOP_EVENT_MASK	(ULOOP_READ | ULOOP_WRITE)

/* internal flags */
#define ULOOP_EVENT_BUFFERED	(1 << 4)
#ifdef USE_KQUEUE
#define ULOOP_EDGE_DEFER	(1 << 5)
#endif

#define ULOOP_ERROR_CB		(1 << 6)
#define ULOOP_PRI           (1 << 7)

// 需要被epoll监听的fd管理块
struct uloop_fd
{
	uloop_fd_handler cb;    // 指向fd的回调函数
	int fd;                 // 记录文件描述符
	bool eof;               // 对端断开连接标志
	bool error;             // 本fd的出错标志
	bool registered;        // 本fd是否已经注册到epoll的标记位：0-未注册 1-已注册
	uint8_t flags;          // 记录了监听的事件类型
};

// 定时器链表节点定义
struct uloop_timeout
{
	struct list_head list;  // 链表模块
	bool pending;           // 加入链表后该标志置1

	uloop_timeout_handler cb;   // 超时处理函数
	struct timeval time;        // 定时值
};

// 子进程链表节点定义
struct uloop_process
{
	struct list_head list;      // 链表模块
	bool pending;               // 加入链表后该标志置1

	uloop_process_handler cb;   // 父进程给本子进程收尸的函数
	pid_t pid;                  // 本子进程pid
};

extern bool uloop_cancelled;
extern bool uloop_handle_sigchld;

int uloop_fd_add(struct uloop_fd *sock, unsigned int flags);
int uloop_fd_delete(struct uloop_fd *sock);

int uloop_timeout_add(struct uloop_timeout *timeout);
int uloop_timeout_set(struct uloop_timeout *timeout, int msecs);
int uloop_timeout_cancel(struct uloop_timeout *timeout);
int uloop_timeout_remaining(struct uloop_timeout *timeout);

int uloop_process_add(struct uloop_process *p);
int uloop_process_delete(struct uloop_process *p);

// 强制结束uloop_run
static inline void uloop_end(void)
{
	uloop_cancelled = true; // 触发SIGINT/SIGTERM信号标志
}

int uloop_init(void);
void uloop_run(void);
void uloop_done(void);

#endif
