/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014 Nicira, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <config.h>
#include "poll-loop.h"
#include <errno.h>
#include <inttypes.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include "coverage.h"
#include "openvswitch/dynamic-string.h"
#include "fatal-signal.h"
#include "openvswitch/list.h"
#include "ovs-thread.h"
#include "seq.h"
#include "socket-util.h"
#include "timeval.h"
#include "openvswitch/vlog.h"
#include "openvswitch/hmap.h"
#include "hash.h"

VLOG_DEFINE_THIS_MODULE(poll_loop);

COVERAGE_DEFINE(poll_create_node);
COVERAGE_DEFINE(poll_zero_timeout);

// 定义了一个poll节点结构
struct poll_node {
    struct hmap_node hmap_node;
    struct pollfd pollfd;       /* Events to pass to time_poll().  该poll节点关联的pollfd */
    HANDLE wevent;              /* Events for WaitForMultipleObjects(). */
    const char *where;          /* Where poll_node was created.  创建该poll节点的位置 */
};

// 这个线程私有数据结构用于管理一个poll模型
struct poll_loop {
    /* All active poll waiters. 
     * 这张hash表记录了所有注册到该poll模型管理块中的poll节点
     * */
    struct hmap poll_nodes;

    /* Time at which to wake up the next call to poll_block(), LLONG_MIN to
     * wake up immediately, or LLONG_MAX to wait forever. 
     * 下次唤醒poll模型的时间点,LLONG_MIN表示非阻塞,LLONG_MAX表示不超时
     * */
    long long int timeout_when; /* In msecs as returned by time_msec(). */
    const char *timeout_where;  /* Where 'timeout_when' was set. */
};

static struct poll_loop *poll_loop(void);

/* Look up the node with same fd or wevent. 
 * 查找当前线程的poll模型管理块中是否已经存在指定fd关联的poll节点
 * */
static struct poll_node *
find_poll_node(struct poll_loop *loop, int fd, HANDLE wevent)
{
    struct poll_node *node;

    /* Both 'fd' and 'wevent' cannot be set. */
    ovs_assert(!fd != !wevent);

    HMAP_FOR_EACH_WITH_HASH (node, hmap_node,
                             hash_2words(fd, (uint32_t)wevent),
                             &loop->poll_nodes) {
        if ((fd && node->pollfd.fd == fd)
            || (wevent && node->wevent == wevent)) {
            return node;
        }
    }
    return NULL;
}

/* On Unix based systems:
 *
 *     Registers 'fd' as waiting for the specified 'events' (which should be
 *     POLLIN or POLLOUT or POLLIN | POLLOUT).  The following call to
 *     poll_block() will wake up when 'fd' becomes ready for one or more of the
 *     requested events. The 'fd's are given to poll() function later.
 *
 * On Windows system:
 *
 *     If 'fd' is specified, create a new 'wevent'. Association of 'fd' and
 *     'wevent' for 'events' happens in poll_block(). If 'wevent' is specified,
 *     it is assumed that it is unrelated to any sockets and poll_block()
 *     will wake up on any event on that 'wevent'. It is an error to pass
 *     both 'wevent' and 'fd'.
 *
 * 注册指定fd以及其关心的事件到poll模型管理块中
 *
 * The event registration is one-shot: only the following call to
 * poll_block() is affected.  The event will need to be re-registered after
 * poll_block() is called if it is to persist.
 *
 * ('where' is used in debug logging.  Commonly one would use poll_fd_wait() to
 * automatically provide the caller's source file and line number for
 * 'where'.) */
static void
poll_create_node(int fd, HANDLE wevent, short int events, const char *where)
{
    // 获取当前线程私有的poll模型管理块
    struct poll_loop *loop = poll_loop();
    struct poll_node *node;

    COVERAGE_INC(poll_create_node);

    /* Both 'fd' and 'wevent' cannot be set. */
    ovs_assert(!fd != !wevent);

    /* Check for duplicate.  If found, "or" the events. 
     * 查找当前线程的poll模型管理块中是否已经存在该fd关联的poll节点
     * */
    node = find_poll_node(loop, fd, wevent);
    if (node) {
        // 如果已经存在该fd关联的poll节点,则无需再创建,直接将新增的事件追加进去即可
        node->pollfd.events |= events;
    } else {
        // 如果尚未存在则为该fd新建一个poll节点
        node = xzalloc(sizeof *node);
        // 将新建的poll节点插入当前线程的poll模型管理块的hash表中
        hmap_insert(&loop->poll_nodes, &node->hmap_node,
                    hash_2words(fd, (uint32_t)wevent));
        node->pollfd.fd = fd;
        node->pollfd.events = events;
#ifdef _WIN32
        if (!wevent) {
            wevent = CreateEvent(NULL, FALSE, FALSE, NULL);
        }
#endif
        node->wevent = wevent;
        node->where = where;
    }
}

/* Registers 'fd' as waiting for the specified 'events' (which should be POLLIN
 * or POLLOUT or POLLIN | POLLOUT).  The following call to poll_block() will
 * wake up when 'fd' becomes ready for one or more of the requested events.
 * 注册指定fd以及其关心的事件到poll模型管理块中
 *
 * On Windows, 'fd' must be a socket.
 *
 * The event registration is one-shot: only the following call to poll_block()
 * is affected.  The event will need to be re-registered after poll_block() is
 * called if it is to persist.
 *
 * ('where' is used in debug logging.  Commonly one would use poll_fd_wait() to
 * automatically provide the caller's source file and line number for
 * 'where'.) */
void
poll_fd_wait_at(int fd, short int events, const char *where)
{
    poll_create_node(fd, 0, events, where);
}

#ifdef _WIN32
/* Registers for the next call to poll_block() to wake up when 'wevent' is
 * signaled.
 *
 * The event registration is one-shot: only the following call to poll_block()
 * is affected.  The event will need to be re-registered after poll_block() is
 * called if it is to persist.
 *
 * ('where' is used in debug logging.  Commonly one would use
 * poll_wevent_wait() to automatically provide the caller's source file and
 * line number for 'where'.) */
void
poll_wevent_wait_at(HANDLE wevent, const char *where)
{
    poll_create_node(0, wevent, 0, where);
}
#endif /* _WIN32 */

/* Causes the following call to poll_block() to block for no more than 'msec'
 * milliseconds.  If 'msec' is nonpositive, the following call to poll_block()
 * will not block at all.
 *
 * The timer registration is one-shot: only the following call to poll_block()
 * is affected.  The timer will need to be re-registered after poll_block() is
 * called if it is to persist.
 *
 * ('where' is used in debug logging.  Commonly one would use poll_timer_wait()
 * to automatically provide the caller's source file and line number for
 * 'where'.) */
void
poll_timer_wait_at(long long int msec, const char *where)
{
    long long int now = time_msec();
    long long int when;

    if (msec <= 0) {
        /* Wake up immediately. */
        when = LLONG_MIN;
    } else if ((unsigned long long int) now + msec <= LLONG_MAX) {
        /* Normal case. */
        when = now + msec;
    } else {
        /* now + msec would overflow. */
        when = LLONG_MAX;
    }

    poll_timer_wait_until_at(when, where);
}

/* Causes the following call to poll_block() to wake up when the current time,
 * as returned by time_msec(), reaches 'when' or later.  If 'when' is earlier
 * than the current time, the following call to poll_block() will not block at
 * all.
 * 设置下次唤醒poll模型的时间点
 *
 * The timer registration is one-shot: only the following call to poll_block()
 * is affected.  The timer will need to be re-registered after poll_block() is
 * called if it is to persist.
 *
 * ('where' is used in debug logging.  Commonly one would use
 * poll_timer_wait_until() to automatically provide the caller's source file
 * and line number for 'where'.) */
void
poll_timer_wait_until_at(long long int when, const char *where)
{
    struct poll_loop *loop = poll_loop();
    if (when < loop->timeout_when) {
        loop->timeout_when = when;
        loop->timeout_where = where;
    }
}

/* Causes the following call to poll_block() to wake up immediately, without
 * blocking.
 * 设置立即唤醒poll模型
 *
 * ('where' is used in debug logging.  Commonly one would use
 * poll_immediate_wake() to automatically provide the caller's source file and
 * line number for 'where'.) */
void
poll_immediate_wake_at(const char *where)
{
    poll_timer_wait_at(0, where);
}

/* Logs, if appropriate, that the poll loop was awakened by an event
 * registered at 'where' (typically a source file and line number).  The other
 * arguments have two possible interpretations:
 *
 *   - If 'pollfd' is nonnull then it should be the "struct pollfd" that caused
 *     the wakeup.  'timeout' is ignored.
 *
 *   - If 'pollfd' is NULL then 'timeout' is the number of milliseconds after
 *     which the poll loop woke up.
 */
static void
log_wakeup(const char *where, const struct pollfd *pollfd, int timeout)
{
    static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(10, 10);
    enum vlog_level level;
    int cpu_usage;
    struct ds s;

    cpu_usage = get_cpu_usage();
    if (VLOG_IS_DBG_ENABLED()) {
        level = VLL_DBG;
    } else if (cpu_usage > 50
               && !thread_is_pmd()
               && !VLOG_DROP_INFO(&rl)) {
        level = VLL_INFO;
    } else {
        return;
    }

    ds_init(&s);
    ds_put_cstr(&s, "wakeup due to ");
    if (pollfd) {
        char *description = describe_fd(pollfd->fd);
        if (pollfd->revents & POLLIN) {
            ds_put_cstr(&s, "[POLLIN]");
        }
        if (pollfd->revents & POLLOUT) {
            ds_put_cstr(&s, "[POLLOUT]");
        }
        if (pollfd->revents & POLLERR) {
            ds_put_cstr(&s, "[POLLERR]");
        }
        if (pollfd->revents & POLLHUP) {
            ds_put_cstr(&s, "[POLLHUP]");
        }
        if (pollfd->revents & POLLNVAL) {
            ds_put_cstr(&s, "[POLLNVAL]");
        }
        ds_put_format(&s, " on fd %d (%s)", pollfd->fd, description);
        free(description);
    } else {
        ds_put_format(&s, "%d-ms timeout", timeout);
    }
    if (where) {
        ds_put_format(&s, " at %s", where);
    }
    if (cpu_usage >= 0) {
        ds_put_format(&s, " (%d%% CPU usage)", cpu_usage);
    }
    VLOG(level, "%s", ds_cstr(&s));
    ds_destroy(&s);
}

// 释放指定poll模型管理块中的所有poll节点
static void
free_poll_nodes(struct poll_loop *loop)
{
    struct poll_node *node, *next;

    HMAP_FOR_EACH_SAFE (node, next, hmap_node, &loop->poll_nodes) {
        hmap_remove(&loop->poll_nodes, &node->hmap_node);
#ifdef _WIN32
        if (node->wevent && node->pollfd.fd) {
            WSAEventSelect(node->pollfd.fd, NULL, 0);
            CloseHandle(node->wevent);
        }
#endif
        free(node);
    }
}

/* Blocks until one or more of the events registered with poll_fd_wait()
 * occurs, or until the minimum duration registered with poll_timer_wait()
 * elapses, or not at all if poll_immediate_wake() has been called. */
void
poll_block(void)
{
    // 首先获取当前线程私有的poll模型管理块
    struct poll_loop *loop = poll_loop();
    struct poll_node *node;
    struct pollfd *pollfds;
    HANDLE *wevents = NULL;
    int elapsed;
    int retval;
    int i;

    /* Register fatal signal events before actually doing any real work for
     * poll_block. 
     * 注册用于致命信号处理的管道读端到当前线程的poll模型管理块中
     * */
    fatal_signal_wait();

    if (loop->timeout_when == LLONG_MIN) {
        COVERAGE_INC(poll_zero_timeout);
    }

    // 正常运行时略过本函数
    timewarp_run();
    // 创建一个pollfd数组,数组长度就是poll模型管理块中注册的poll节点数量
    pollfds = xmalloc(hmap_count(&loop->poll_nodes) * sizeof *pollfds);

#ifdef _WIN32
    wevents = xmalloc(hmap_count(&loop->poll_nodes) * sizeof *wevents);
#endif

    /* Populate with all the fds and events. 
     * 将所有已经注册的fd以及关联的事件填充到该pollfd数组中
     * */
    i = 0;
    HMAP_FOR_EACH (node, hmap_node, &loop->poll_nodes) {
        pollfds[i] = node->pollfd;
#ifdef _WIN32
        wevents[i] = node->wevent;
        if (node->pollfd.fd && node->wevent) {
            short int wsa_events = 0;
            if (node->pollfd.events & POLLIN) {
                wsa_events |= FD_READ | FD_ACCEPT | FD_CLOSE;
            }
            if (node->pollfd.events & POLLOUT) {
                wsa_events |= FD_WRITE | FD_CONNECT | FD_CLOSE;
            }
            WSAEventSelect(node->pollfd.fd, node->wevent, wsa_events);
        }
#endif
        i++;
    }

    // 调用ovs封装的poll接口
    retval = time_poll(pollfds, hmap_count(&loop->poll_nodes), wevents,
                       loop->timeout_when, &elapsed);
    if (retval < 0) {
        static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(1, 5);
        VLOG_ERR_RL(&rl, "poll: %s", ovs_strerror(-retval));
    } else if (!retval) {
        log_wakeup(loop->timeout_where, NULL, elapsed);
    } else if (get_cpu_usage() > 50 || VLOG_IS_DBG_ENABLED()) {
        i = 0;
        HMAP_FOR_EACH (node, hmap_node, &loop->poll_nodes) {
            if (pollfds[i].revents) {
                log_wakeup(node->where, &pollfds[i], 0);
            }
            i++;
        }
    }

    // 释放当前线程私有poll模型管理块中的所有poll节点,并复位其余字段
    free_poll_nodes(loop);
    loop->timeout_when = LLONG_MAX;
    loop->timeout_where = NULL;
    free(pollfds);
    free(wevents);

    /* Handle any pending signals before doing anything else. 
     * 检查是否已经有信号发生,如果有就执行真正的致命信号处理函数
     * */
    fatal_signal_run();

    // 销毁等待在当前线程私有seq_thread上的对象
    seq_woke();
}
// 线程私有数据struct poll_loop的析构函数
static void
free_poll_loop(void *loop_)
{
    struct poll_loop *loop = loop_;

    free_poll_nodes(loop);
    hmap_destroy(&loop->poll_nodes);
    free(loop);
}

// 获取当前线程私有的poll模型管理块, 如果不存在则创建
static struct poll_loop *
poll_loop(void)
{
    static struct ovsthread_once once = OVSTHREAD_ONCE_INITIALIZER;
    static pthread_key_t key;
    struct poll_loop *loop;

    // 创建线程私有数据struct poll_loop关联的key,显然只会执行一次
    if (ovsthread_once_start(&once)) {
        xpthread_key_create(&key, free_poll_loop);
        ovsthread_once_done(&once);
    }

    // 根据key索引关联的线程私有数据,如果不存在,则创建一个线程私有数据并关联到key
    loop = pthread_getspecific(key);
    if (!loop) {
        loop = xzalloc(sizeof *loop);
        hmap_init(&loop->poll_nodes);
        xpthread_setspecific(key, loop);
    }
    return loop;
}

