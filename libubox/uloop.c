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
 */
#include <sys/time.h>
#include <sys/types.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#include "uloop.h"
#include "utils.h"

#ifdef USE_EPOLL
#include <sys/epoll.h>
#endif
#include <sys/wait.h>

// 当前被触发事件的控制块,内容基本类似底层库结构epoll_event
struct uloop_fd_event {
	struct uloop_fd *fd;    // 当前被触发事件的fd控制块
	unsigned int events;    // 被触发的事件类型
};

struct uloop_fd_stack {
	struct uloop_fd_stack *next;
	struct uloop_fd *fd;
	unsigned int events;
};

static struct uloop_fd_stack *fd_stack = NULL;

#define ULOOP_MAX_EVENTS 10     // 事件池容纳事件的最大数量,不超过epoll_create函数的size

static struct list_head timeouts = LIST_HEAD_INIT(timeouts);    // 创建了一个定时器链表的头结点，并初始化
static struct list_head processes = LIST_HEAD_INIT(processes);  // 创建了一个子进程链表的头结点，并初始化

static int poll_fd = -1;        // epoll句柄
bool uloop_cancelled = false;   // SIGINT信号触发后的标志
bool uloop_handle_sigchld = true;   // 决定SIGCHLD信号是否安装
static bool do_sigchld = false; // SIGCHLD信号触发后的标志

static struct uloop_fd_event cur_fds[ULOOP_MAX_EVENTS]; // 当前被触发的事件集合
static int cur_fd, cur_nfds;    // cur_fd:当前正在处理的触发事件索引号  cur_nfds:当前触发的事件数量

#ifdef USE_EPOLL

/**
 * FIXME: uClibc < 0.9.30.3 does not define EPOLLRDHUP for Linux >= 2.6.17
 */
#ifndef EPOLLRDHUP
#define EPOLLRDHUP 0x2000
#endif

// 创建epoll句柄
int uloop_init(void)
{
	if (poll_fd >= 0)
		return 0;

    // 创建一个epoll句柄，监听的数目上限32
	poll_fd = epoll_create(32);
	if (poll_fd < 0)
		return -1;

    // 实现close-on-exec功能
	fcntl(poll_fd, F_SETFD, fcntl(poll_fd, F_GETFD) | FD_CLOEXEC);
	return 0;
}

// 将需要监听的fd以及要对其监听的事件类型注册到epoll(中间封装)
static int register_poll(struct uloop_fd *fd, unsigned int flags)
{
	struct epoll_event ev;
	int op = fd->registered ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;

	memset(&ev, 0, sizeof(struct epoll_event));

	if (flags & ULOOP_READ)
		ev.events |= EPOLLIN | EPOLLRDHUP;

	if (flags & ULOOP_WRITE)
		ev.events |= EPOLLOUT;

	if (flags & ULOOP_EDGE_TRIGGER)
		ev.events |= EPOLLET;

	ev.data.fd = fd->fd;
	ev.data.ptr = fd;   // 将需要监听的fd保存到对应的epoll监听控制块
	fd->flags = flags;

    // 注册需要监听的fd和事件类型到epoll(底层库函数)
	return epoll_ctl(poll_fd, op, fd->fd, &ev);
}

// epoll容纳触发事件的事件池
static struct epoll_event events[ULOOP_MAX_EVENTS];

static int __uloop_fd_delete(struct uloop_fd *sock)
{
	sock->flags = 0;
	return epoll_ctl(poll_fd, EPOLL_CTL_DEL, sock->fd, 0);
}

// 获取触发的事件,返回触发的事件数量
static int uloop_fetch_events(int timeout)
{
	int n, nfds;

    // 等待事件触发（epoll底层库函数)
	nfds = epoll_wait(poll_fd, events, ARRAY_SIZE(events), timeout);

    // 将触发的事件一一从events拷贝到cur_fds
	for (n = 0; n < nfds; ++n) 
    {
		struct uloop_fd_event *cur = &cur_fds[n];
		struct uloop_fd *u = events[n].data.ptr;
		unsigned int ev = 0;

		cur->fd = u;
		if (!u)
			continue;

        // 处理描述符发生错误或被挂断事件
		if (events[n].events & (EPOLLERR|EPOLLHUP)) 
        {
			u->error = true;

            // 如果没有给这个fd控制块注册出错回调函数,则删除这个fd控制块
			if (!(u->flags & ULOOP_ERROR_CB))
				uloop_fd_delete(u);
		}

        // 如果触发的事件不属于有效事件，直接丢弃
		if(!(events[n].events & (EPOLLRDHUP|EPOLLIN|EPOLLOUT|EPOLLERR|EPOLLHUP))) {
			cur->fd = NULL;
			continue;
		}

        // 处理对端断开连接事件
		if(events[n].events & EPOLLRDHUP)
			u->eof = true;

        // 处理可读事件
		if(events[n].events & EPOLLIN)
			ev |= ULOOP_READ;

        // 处理可写事件
		if(events[n].events & EPOLLOUT)
			ev |= ULOOP_WRITE;

		cur->events = ev;
	}

	return nfds;
}

#endif

static bool uloop_fd_stack_event(struct uloop_fd *fd, int events)
{
	struct uloop_fd_stack *cur;

	/*
	 * Do not buffer events for level-triggered fds, they will keep firing.
	 * Caller needs to take care of recursion issues.
     * 不要缓存带有边沿触发属性的fd控制块
	 */
	if (!(fd->flags & ULOOP_EDGE_TRIGGER))
		return false;

	for (cur = fd_stack; cur; cur = cur->next) 
    {
		if (cur->fd != fd)
			continue;

		if (events < 0)
			cur->fd = NULL;
		else
			cur->events |= events | ULOOP_EVENT_BUFFERED;

		return true;
	}

	return false;
}

// 等待获取事件、处理事件(特别要注意，timeout值是跟定时器链表首节点中的定时值相关的;epoll的超时处理是在上级函数里完成的)
static void uloop_run_events(int timeout)
{
	struct uloop_fd_event *cur;
	struct uloop_fd *fd;

    // 如果当前没有触发事件,则需要在这里等待事件的产生
	if (!cur_nfds) 
    {
		cur_fd = 0;
        // 获取触发的事件,返回值cur_fnds<=0通通意味着超时
		cur_nfds = uloop_fetch_events(timeout);
		if (cur_nfds < 0)
			cur_nfds = 0;
	}

    // 处理所有被触发的事件
	while (cur_nfds > 0) 
    {
		struct uloop_fd_stack stack_cur;
		unsigned int events;

		cur = &cur_fds[cur_fd++];
		cur_nfds--;

		fd = cur->fd;
		events = cur->events;
		if (!fd)
			continue;

		if (!fd->cb)
			continue;

		if (uloop_fd_stack_event(fd, cur->events))
			continue;

		stack_cur.next = fd_stack;
		stack_cur.fd = fd;
		fd_stack = &stack_cur;
		do {
			stack_cur.events = 0;
			fd->cb(fd, events);
			events = stack_cur.events & ULOOP_EVENT_MASK;
		} while (stack_cur.fd && events);
		fd_stack = stack_cur.next;

		return;
	}
}

// 将需要监听的fd以及要对其监听的事件类型注册到epoll(顶层封装)
int uloop_fd_add(struct uloop_fd *sock, unsigned int flags)
{
	unsigned int fl;
	int ret;

    // epoll监听的事件必须包含读或写
	if (!(flags & (ULOOP_READ | ULOOP_WRITE)))
		return uloop_fd_delete(sock);

    // 监听套接字设置非阻塞属性
	if (!sock->registered && !(flags & ULOOP_BLOCKING)) {
		fl = fcntl(sock->fd, F_GETFL, 0);
		fl |= O_NONBLOCK;
		fcntl(sock->fd, F_SETFL, fl);
	}

    // 将需要监听的fd以及要对其监听的事件类型注册到epoll(中间封装)
	ret = register_poll(sock, flags);
	if (ret < 0)
		goto out;

	sock->registered = true;    // 本fd控制块标记为已经注册
	sock->eof = false;

out:
	return ret;
}

// 删除指定的fd控制块
int uloop_fd_delete(struct uloop_fd *fd)
{
	int i;

	for (i = 0; i < cur_nfds; i++) {
		if (cur_fds[cur_fd + i].fd != fd)
			continue;

		cur_fds[cur_fd + i].fd = NULL;
	}

	if (!fd->registered)
		return 0;

	fd->registered = false;
	uloop_fd_stack_event(fd, -1);
	return __uloop_fd_delete(fd);
}

// 时间比较
static int tv_diff(struct timeval *t1, struct timeval *t2)
{
	return
		(t1->tv_sec - t2->tv_sec) * 1000 +
		(t1->tv_usec - t2->tv_usec) / 1000;
}

int uloop_timeout_add(struct uloop_timeout *timeout)
{
	struct uloop_timeout *tmp;
	struct list_head *h = &timeouts;

	if (timeout->pending)
		return -1;

	list_for_each_entry(tmp, &timeouts, list) {
		if (tv_diff(&tmp->time, &timeout->time) > 0) {
			h = &tmp->list;
			break;
		}
	}

	list_add_tail(&timeout->list, h);
	timeout->pending = true;

	return 0;
}

// 获取当前时间（从系统启动开始计时）
static void uloop_gettime(struct timeval *tv)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	tv->tv_sec = ts.tv_sec;
	tv->tv_usec = ts.tv_nsec / 1000;
}

// 给指定的定时器控制块设置一个超时值（ms单位）,内部封装了将节点插入链表的动作
int uloop_timeout_set(struct uloop_timeout *timeout, int msecs)
{
	struct timeval *time = &timeout->time;

	if (timeout->pending)
		uloop_timeout_cancel(timeout);

	uloop_gettime(&timeout->time);

	time->tv_sec += msecs / 1000;
	time->tv_usec += (msecs % 1000) * 1000;

	if (time->tv_usec > 1000000) {
		time->tv_sec++;
		time->tv_usec %= 1000000;
	}

    // 将设置好超时值的定时器节点插入定时器链表
	return uloop_timeout_add(timeout);
}

// 超时处理:删除超时定时器节点
int uloop_timeout_cancel(struct uloop_timeout *timeout)
{
	if (!timeout->pending)
		return -1;

	list_del(&timeout->list);
	timeout->pending = false;

	return 0;
}

int uloop_timeout_remaining(struct uloop_timeout *timeout)
{
	struct timeval now;

	if (!timeout->pending)
		return -1;

	uloop_gettime(&now);

	return tv_diff(&timeout->time, &now);
}

int uloop_process_add(struct uloop_process *p)
{
	struct uloop_process *tmp;
	struct list_head *h = &processes;

	if (p->pending)
		return -1;

	list_for_each_entry(tmp, &processes, list) {
		if (tmp->pid > p->pid) {
			h = &tmp->list;
			break;
		}
	}

	list_add_tail(&p->list, h);
	p->pending = true;

	return 0;
}

// 从链表中删除指定的子进程节点
int uloop_process_delete(struct uloop_process *p)
{
	if (!p->pending)
		return -1;

	list_del(&p->list);
	p->pending = false;

	return 0;
}

// 给子进程收尸
static void uloop_handle_processes(void)
{
	struct uloop_process *p, *tmp;
	pid_t pid;
	int ret;

	do_sigchld = false; // 复位SIGCHLD触发标记

	while (1) 
    {
        // 非阻塞获取已经结束的子进程pid
		pid = waitpid(-1, &ret, WNOHANG);
		if (pid <= 0)
			return;

        // 遍历子进程链表，给指定pid子进程收尸
		list_for_each_entry_safe(p, tmp, &processes, list) 
        {
            // 通过pid匹配，找到对应的子进程节点
			if (p->pid < pid)
				continue;

			if (p->pid > pid)
				break;

            // 删除匹配成功的子进程节点
			uloop_process_delete(p);

            // 执行这个节点注册的回调函数
			p->cb(p, ret);
		}
	}

}

static void uloop_handle_sigint(int signo)
{
	uloop_cancelled = true;
}

static void uloop_sigchld(int signo)
{
	do_sigchld = true;
}

// 安装SIGINT和SIGCHLD信号
static void uloop_setup_signals(bool add)
{
	static struct sigaction old_sigint, old_sigchld;
	struct sigaction s;

	memset(&s, 0, sizeof(struct sigaction));

    // 配置SIGINT信号的处理函数（不带参数）
	if (add) {
		s.sa_handler = uloop_handle_sigint;
		s.sa_flags = 0;
	} 
    else {
		s = old_sigint;
	}

    // 安装SIGINT信号
	sigaction(SIGINT, &s, &old_sigint);

    // 判断是否需要安装SIGCHLD信号
	if (!uloop_handle_sigchld)
		return;

    // 配置SIGCHLD信号的处理函数（不带参数）
	if (add)
		s.sa_handler = uloop_sigchld;
	else
		s = old_sigchld;

    // 安装SIGCHLD信号
	sigaction(SIGCHLD, &s, &old_sigchld);
}

// 获取定时器链表首节点距离超时的剩余时间
static int uloop_get_next_timeout(struct timeval *tv)
{
	struct uloop_timeout *timeout;
	int diff;

    // 判断定时器链表是否为空
	if (list_empty(&timeouts))
		return -1;

    // 获取定时器链表的首节点
	timeout = list_first_entry(&timeouts, struct uloop_timeout, list);

    // 判断是否超时,超时返回0
	diff = tv_diff(&timeout->time, tv);
	if (diff < 0)
		return 0;

	return diff;
}

// 超时处理
static void uloop_process_timeouts(struct timeval *tv)
{
	struct uloop_timeout *t;

    // 遍历定时器链表
	while (!list_empty(&timeouts)) 
    {
        // 获取定时器链表的首节点
		t = list_first_entry(&timeouts, struct uloop_timeout, list);

        // 判断是否超时
		if (tv_diff(&t->time, tv) > 0)
			break;

        // 删除超时的定时器节点
		uloop_timeout_cancel(t);

        // 运行超时定时器注册的处理函数
		if (t->cb)
			t->cb(t);
	}
}

static void uloop_clear_timeouts(void)
{
	struct uloop_timeout *t, *tmp;

	list_for_each_entry_safe(t, tmp, &timeouts, list)
		uloop_timeout_cancel(t);
}

static void uloop_clear_processes(void)
{
	struct uloop_process *p, *tmp;

	list_for_each_entry_safe(p, tmp, &processes, list)
		uloop_process_delete(p);
}

void uloop_run(void)
{
	static int recursive_calls = 0;
	struct timeval tv;

	/*
	 * Handlers are only updated for the first call to uloop_run() (and restored
	 * when this call is done).
     * SIGINT和SIGCHLD信号安装
	 */
	if (!recursive_calls++)
		uloop_setup_signals(true);

    // 程序进入循环，直到收到SIGINT信号
	while(!uloop_cancelled)
	{
    	uloop_gettime(&tv);             // 获取当前时间
		uloop_process_timeouts(&tv);    // 超时处理

        // 判断是否收到SIGINT信号
		if (uloop_cancelled)
			break;

        // 判断是否收到SIGCHLD信号
		if (do_sigchld)
			uloop_handle_processes();   // 收尸
        
		uloop_gettime(&tv);             // 获取当前时间

        // 等待获取事件、处理事件
		uloop_run_events(uloop_get_next_timeout(&tv));
	}

	if (!--recursive_calls)
		uloop_setup_signals(false);
}

void uloop_done(void)
{
	if (poll_fd < 0)
		return;

	close(poll_fd);
	poll_fd = -1;

	uloop_clear_timeouts();
	uloop_clear_processes();
}
