
#include <sys/epoll.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>

#include "log.h"
#include "epoll_loop.h"
#include "jrppd.h"

int epoll_fd = -1;
static LIST_HEAD(timeouts);

int init_epoll(void)
{
	int r = epoll_create(128);
	if (r < 0) {
		LOG_ERROR("epoll_create failed: %m\n");
		return -1;
	}
	epoll_fd = r;
	return 0;
}

int add_epoll(struct epoll_event_handler *h)
{
	struct epoll_event ev = {
		.events = EPOLLIN,
		.data.ptr = h,
	};
	h->ref_ev = NULL;
	int r = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, h->fd, &ev);
	if (r < 0) {
		LOG_ERROR("epoll_ctl_add failed: %m\n");
		return -1;
	}
	return 0;
}

int remove_epoll(struct epoll_event_handler *h)
{
	int r = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, h->fd, NULL);
	if (r < 0) {
		LOG_ERROR("epoll_ctl_del failed: %m\n");
		return -1;
	}
	if (h->ref_ev && h->ref_ev->data.ptr == h) {
		h->ref_ev->data.ptr = NULL;
		h->ref_ev = NULL;
	}
	return 0;
}

void clear_epoll(void)
{
	if (epoll_fd >= 0)
		close(epoll_fd);
}

static int timer_diff(struct timeval *second, struct timeval *first)
{
	return (second->tv_sec - first->tv_sec) * 1000 + (second->tv_usec - first->tv_usec) / 1000;
}

// 删除定时器
int timer_del(struct uloop_timeout *timer)
{
    if(!timer->pending)
        return -1;

    list_del(&timer->list);
    timer->pending = false;

    return 0;
}

// 添加(复位)定时器
int timer_add(struct uloop_timeout *timer, int msecs)
{
    struct timeval *time = &timer->time;

    if(timer->pending)
        timer_del(timer);

    gettimeofday(time,NULL);

    time->tv_sec += msecs / 1000;
    time->tv_usec += msecs % 1000 * 1000;
    if(time->tv_usec > 1000000)
    {
        time->tv_sec++;
        time->tv_usec -= 1000000;
    }

    struct uloop_timeout *t;
    struct list_head *h = &timeouts;
    list_for_each_entry(t,&timeouts,list)
    {
        if(timer_diff(&t->time,time) > 0)
        {
            h = &t->list;
            break;
        }
    }

    list_add_tail(&timer->list,h);
    timer->pending = true;

    return 0;
}


static void timer_process(struct timeval *cur)
{
    struct uloop_timeout *t;

    while(!list_empty(&timeouts))
    {
        t = list_first_entry(&timeouts,struct uloop_timeout,list);
        if(timer_diff(&t->time,cur) > 0)
            break;

        timer_del(t);

        if(t->cb)
            t->cb(t);
    }
}

static int timer_next_timeout(struct timeval *cur)
{
    struct uloop_timeout *t;
    int diff;

    if(list_empty(&timeouts))
        return -1;

    t = list_first_entry(&timeouts,struct uloop_timeout,list);
    diff = timer_diff(&t->time,cur);

    return (diff < 0)?0:diff;
}

int epoll_main_loop(void)
{
#define EV_SIZE 8
	struct epoll_event ev[EV_SIZE];

	while (1) 
    {
		int r, i;
		struct timeval tv;

		gettimeofday(&tv, NULL);
        timer_process(&tv);

		gettimeofday(&tv, NULL);
        r = epoll_wait(epoll_fd,ev,EV_SIZE,timer_next_timeout(&tv));
		if (r < 0 && errno != EINTR) {
			LOG_ERROR("epoll_wait error: %m\n");
			return -1;
		}
		for (i = 0; i < r; i++) {
			struct epoll_event_handler *p = ev[i].data.ptr;
			if (p != NULL)
				p->ref_ev = &ev[i];
		}
		for (i = 0; i < r; i++) {
			struct epoll_event_handler *p = ev[i].data.ptr;
			if (p && p->handler)
				p->handler(ev[i].events, p);
		}
		for (i = 0; i < r; i++) {
			struct epoll_event_handler *p = ev[i].data.ptr;
			if (p != NULL)
				p->ref_ev = NULL;
		}
	}

	return 0;
}

