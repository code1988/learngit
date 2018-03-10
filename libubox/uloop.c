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

#ifdef USE_KQUEUE
#include <sys/event.h>
#endif
#ifdef USE_EPOLL
#include <sys/epoll.h>
#endif
#include <sys/wait.h>

// 当前被触发事件的控制块,内容基本类似底层库结构epoll_event
struct uloop_fd_event {
	struct uloop_fd *fd;    // 当前被触发事件隶属的fd控制块
	unsigned int events;    // 被触发的事件类型
};

// 定义尚未处理的触发fd控制块节点
struct uloop_fd_stack {
	struct uloop_fd_stack *next;
	struct uloop_fd *fd;
	unsigned int events;
};

static struct uloop_fd_stack *fd_stack = NULL;  // 指向存储尚未处理的触发fd控制块的链表头部,这个链表池实际应该是并没有被用起来

#define ULOOP_MAX_EVENTS 10 // epoll监听池容纳事件的最大数量,不超过epoll_create函数的size

static struct list_head timeouts = LIST_HEAD_INIT(timeouts);    // 创建了一个定时器链表的头结点，并初始化
static struct list_head processes = LIST_HEAD_INIT(processes);  // 创建了一个子进程链表的头结点，并初始化

static int poll_fd = -1;        // epoll句柄
bool uloop_cancelled = false;   // SIGINT信号触发后的标志
static bool do_sigchld = false; // SIGCHLD信号触发后的标志

static struct uloop_fd_event cur_fds[ULOOP_MAX_EVENTS]; // 当前被触发的事件集合,可以理解为libubox库对下层events数组处理后的封装
static int cur_fd, cur_nfds;    // cur_fd:当前正在处理的触发事件索引号  cur_nfds:当前触发的事件数量

// USE_KQUEUE宏定义开关位于makefile
#ifdef USE_KQUEUE
// 略
#endif

// USE_EPOLL宏定义开关位于makefile
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
    
    // 创建一个epoll句柄，监听的fd数目上限32个
	poll_fd = epoll_create(32);
	if (poll_fd < 0)
		return -1;

    // 实现close-on-exec功能
	fcntl(poll_fd, F_SETFD, fcntl(poll_fd, F_GETFD) | FD_CLOEXEC);
	return 0;
}

// 将需要监听的fd以及要对其监听的事件类型注册到epoll(2级封装)
static int register_poll(struct uloop_fd *fd, unsigned int flags)
{
	struct epoll_event ev;
	int op = fd->registered ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;    // 判断epoll_ctl要执行的动作

	memset(&ev, 0, sizeof(struct epoll_event));

    // 判断要监听的事件
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

// 从epoll的监听池中删除指定的fd
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

    // 将events数组中的触发事件处理后顺序填充到cur_fds数组
	for (n = 0; n < nfds; ++n) {
		struct uloop_fd_event *cur = &cur_fds[n];
		struct uloop_fd *u = events[n].data.ptr;
		unsigned int ev = 0;

		cur->fd = u;
		if (!u)
			continue;

        // 处理描述符发生错误或被挂断事件
		if (events[n].events & (EPOLLERR|EPOLLHUP)) {
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

// 遍历已经触发但尚未被处理的fd池fd_stack，如果跟新触发的fd吻合，则合并events集合(入参events赋值1来达到清楚节点的效果)
static bool uloop_fd_stack_event(struct uloop_fd *fd, int events)
{
	struct uloop_fd_stack *cur;

	/*
	 * Do not buffer events for level-triggered fds, they will keep firing.
	 * Caller needs to take care of recursion issues.
     * 带有水平触发属性的fd控制块不会被加入缓冲池，因为它们会一直触发epoll（如果没有被处理）
	 */
	if (!(fd->flags & ULOOP_EDGE_TRIGGER))
		return false;

    // 遍历fd_stack缓存链表池
	for (cur = fd_stack; cur; cur = cur->next) {
		if (cur->fd != fd)
			continue;

        // 达到清除节点效果
		if (events < 0)
			cur->fd = NULL;
		else
			cur->events |= events | ULOOP_EVENT_BUFFERED;

		return true;
	}

	return false;
}

/* 等待获取事件、处理事件
 * 特别要注意，timeout值是跟定时器链表首节点中的定时值相关的;epoll的超时处理是在上级函数里完成的;本函数一次只能处理一个fd
 */
static void uloop_run_events(int timeout)
{
	struct uloop_fd_event *cur;
	struct uloop_fd *fd;

    // 如果当前没有触发事件,则需要在这里等待事件的产生
	if (!cur_nfds) {
		cur_fd = 0;
        // 获取触发的事件,返回值cur_fnds<=0通通意味着超时
		cur_nfds = uloop_fetch_events(timeout);
		if (cur_nfds < 0)
			cur_nfds = 0;
	}

    /* 处理当前被触发的事件
    *  注意：当前函数不一定执行一遍就可以处理完所有的cur_nfds数量的当前被触发fd, 
    *  因为该while只能执行一个xfd事件,xfd之前的fd都会被缓存到fd_stack池,
    *  xfd之后的fd要等待下一次进入当前函数再被处理
    */
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

        // 遍历已经触发但尚未被处理的fd池fd_stack，如果跟新触发的fd吻合，则将新触发events合并进去
		if (uloop_fd_stack_event(fd, cur->events))
			continue;

        // 运行到这里意味着新触发的fd在fd_stack中并没有相同的节点，或者该fd不支持缓存（携带了水平触发属性），那么将该fd作为一个新节点插到fd_stack链表头部
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

// 将需要监听的fd以及要对其监听的事件类型注册到epoll(3级封装,默认拥有水平触发、非阻塞的特性)
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

    // 将需要监听的fd以及要对其监听的事件类型注册到epoll(2级封装)
	ret = register_poll(sock, flags);
	if (ret < 0)
		goto out;

	sock->registered = true;    // 本fd控制块标记为已经注册
	sock->eof = false;
	sock->error = false;

out:
	return ret;
}

// 删除指定的fd控制块
int uloop_fd_delete(struct uloop_fd *fd)
{
	int i;

    // 根据fd遍历当前被触发的事件集合cur_fds，删除fd相同的元素
	for (i = 0; i < cur_nfds; i++) {
		if (cur_fds[cur_fd + i].fd != fd)
			continue;

		cur_fds[cur_fd + i].fd = NULL;
	}

	if (!fd->registered)
		return 0;

	fd->registered = false;

    // 删除fd_stack链表池中的fd节点
	uloop_fd_stack_event(fd, -1);

    // 从epoll的监听池中删除指定的fd
	return __uloop_fd_delete(fd);
}

// 时间比较
static int tv_diff(struct timeval *t1, struct timeval *t2)
{
	return
		(t1->tv_sec - t2->tv_sec) * 1000 +
		(t1->tv_usec - t2->tv_usec) / 1000;
}

// 将定时器插入链表(根据定时值排序)
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

	uloop_gettime(time);

	time->tv_sec += msecs / 1000;
	time->tv_usec += (msecs % 1000) * 1000;

	if (time->tv_usec > 1000000) {
		time->tv_sec++;
		time->tv_usec -= 1000000;
	}

    // 将设置好超时值的定时器节点插入定时器链表
	return uloop_timeout_add(timeout);
}

// 删除定时器节点
int uloop_timeout_cancel(struct uloop_timeout *timeout)
{
	if (!timeout->pending)
		return -1;

	list_del(&timeout->list);
	timeout->pending = false;

	return 0;
}

// 计算剩余超时
int uloop_timeout_remaining(struct uloop_timeout *timeout)
{
	struct timeval now;

	if (!timeout->pending)
		return -1;

	uloop_gettime(&now);

	return tv_diff(&timeout->time, &now);
}

// 添加子进程节点到链表
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

	while (1) {
        // 非阻塞获取已经结束的子进程pid
		pid = waitpid(-1, &ret, WNOHANG);
		if (pid <= 0)
			return;

        // 遍历子进程链表，给指定pid子进程收尸
		list_for_each_entry_safe(p, tmp, &processes, list) {
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

// SIGINT/SIGTERM信号的回调函数
static void uloop_handle_sigint(int signo)
{
	uloop_cancelled = true;
}

// SIGCHLD信号的回调函数
static void uloop_sigchld(int signo)
{
	do_sigchld = true;
}

// 信号安装/恢复函数(只能安装有缺省处理动作的信号)
static void uloop_install_handler(int signum, void (*handler)(int), struct sigaction* old, bool add)
{
	struct sigaction s;
	struct sigaction *act;

	act = NULL;

    // 获取该信号缺省的处理动作
	sigaction(signum, NULL, &s);

    // 判断信号要执行的功能:安装/复位
	if (add) 
    {
        // 要安装信号，需要先判断该信号是否有一个缺省的处理动作
		if (s.sa_handler == SIG_DFL) { /* Do not override existing custom signal handlers */
			memcpy(old, &s, sizeof(struct sigaction));
			s.sa_handler = handler;
			s.sa_flags = 0;
			act = &s;
		}
	}
	else if (s.sa_handler == handler) { /* Do not restore if someone modified our handler */
			act = old;
	}

    // 安装信号
	if (act != NULL)
		sigaction(signum, act, NULL);
}

// 信号忽略/恢复处理函数
static void uloop_ignore_signal(int signum, bool ignore)
{
	struct sigaction s;
	void *new_handler = NULL;

    // 获取该信号旧有的处理动作
	sigaction(signum, NULL, &s);

	if (ignore) 
    {
        // 执行忽略的前提是该信号旧有的处理动作是SIG_DFL
		if (s.sa_handler == SIG_DFL) /* Ignore only if there isn't any custom handler */
			new_handler = SIG_IGN;
	} 
    else 
    {
        // 执行恢复的前提是该信号旧有的处理动作是SIG_IGN
		if (s.sa_handler == SIG_IGN) /* Restore only if noone modified our SIG_IGN */
			new_handler = SIG_DFL;
	}

    // 执行忽略/恢复动作
	if (new_handler) {
		s.sa_handler = new_handler;
		s.sa_flags = 0;
		sigaction(signum, &s, NULL);
	}
}

// 设置SIGINT、SIGTERM、SIGCHLD、SIGPIPE信号处理函数
static void uloop_setup_signals(bool add)
{
	static struct sigaction old_sigint, old_sigchld, old_sigterm;

	uloop_install_handler(SIGINT, uloop_handle_sigint, &old_sigint, add);
	uloop_install_handler(SIGTERM, uloop_handle_sigint, &old_sigterm, add);
	uloop_install_handler(SIGCHLD, uloop_sigchld, &old_sigchld, add);

	uloop_ignore_signal(SIGPIPE, add);
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

// 删除所有定时器控制块
static void uloop_clear_timeouts(void)
{
	struct uloop_timeout *t, *tmp;

    // 遍历定时器链表，释放所有节点
	list_for_each_entry_safe(t, tmp, &timeouts, list)
		uloop_timeout_cancel(t);
}

// 删除所有的子进程控制块
static void uloop_clear_processes(void)
{
	struct uloop_process *p, *tmp;

    // 遍历子进程链表，释放所有节点
	list_for_each_entry_safe(p, tmp, &processes, list)
		uloop_process_delete(p);
}

// socket通信主循环
void uloop_run(void)
{
	static int recursive_calls = 0;
	struct timeval tv;

	/*
	 * Handlers are only updated for the first call to uloop_run() (and restored
	 * when this call is done).
     * 当且仅当第一次调用本函数时需要进行信号设置
	 */
	if (!recursive_calls++)
		uloop_setup_signals(true);

    // 程序进入循环，直到收到SIGINT/SIGTERM信号
	uloop_cancelled = false;
	while(!uloop_cancelled)
	{
		uloop_gettime(&tv);             // 获取当前时间
		uloop_process_timeouts(&tv);    // 超时处理

        // 判断是否收到SIGCHLD信号
		if (do_sigchld)
			uloop_handle_processes();   // 收尸

        // 判断是否收到SIGINT信号
		if (uloop_cancelled)
			break;

		uloop_gettime(&tv);             // 获取当前时间

        // 等待获取事件、处理事件
		uloop_run_events(uloop_get_next_timeout(&tv));
	}

    // 当且仅当最后一次调用本函数时需要恢复信号设置
	if (!--recursive_calls)
		uloop_setup_signals(false);
}

// socket通信结束时需要释放相关的变量
void uloop_done(void)
{
	if (poll_fd < 0)
		return;

    // 释放socket描述符
	close(poll_fd);
	poll_fd = -1;

	uloop_clear_timeouts();     // 删除所有的定时器控制块
	uloop_clear_processes();    // 删除所有的子进程控制块
}
