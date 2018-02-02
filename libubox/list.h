/*-
 * Copyright (c) 2011 Felix Fietkau <nbd@openwrt.org>
 * Copyright (c) 2010 Isilon Systems, Inc.
 * Copyright (c) 2010 iX Systems, Inc.
 * Copyright (c) 2010 Panasas, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 功能描述：链表操作的通用库。将head_list模块加入你的结构体即完成了链表的创建,
 *           这个库还提供了截断、移动、插入、遍历等链表操作
 * 备注：整个链表库操作基本类似kernel
 *       特别注意头节点和首节点的区别，链表头节点不带数据，用来链接首、尾节点；
 *       首节点是第一个带数据的节点，它前面一个节点就是头节点
 */
#ifndef _LINUX_LIST_H_
#define _LINUX_LIST_H_

#include <stddef.h>
#include <stdbool.h>

#define	prefetch(x)

#ifndef container_of
// 由结构体成员member的地址ptr，求结构体type的地址
#define container_of(ptr, type, member)					\
	({								\
		const typeof(((type *) NULL)->member) *__mptr = (ptr);	\
		(type *) ((char *) __mptr - offsetof(type, member));	\
	})
#endif

// 双向循环链表模块
struct list_head {
	struct list_head *next;
	struct list_head *prev;
};

// 创建并初始化链表头节点(前驱和后继指针都指向自己)
#define LIST_HEAD_INIT(name) { &(name), &(name) }
#undef LIST_HEAD
#define LIST_HEAD(name)	struct list_head name = LIST_HEAD_INIT(name)

// 初始化链表头节点(前驱和后继指针都指向自己)
static inline void
INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list->prev = list;
}

// 判断链表是否为空(也用于判断该节点是否已经被插入链表)
static inline bool
list_empty(const struct list_head *head)
{
	return (head->next == head);
}

// 判断当前节点是不是首节点
static inline bool
list_is_first(const struct list_head *list,
	      const struct list_head *head)
{
	return list->prev == head;
}

// 判断当前节点是不是尾节点
static inline bool
list_is_last(const struct list_head *list,
	     const struct list_head *head)
{
	return list->next == head;
}

// 删除指定节点(内部调用)
static inline void
_list_del(struct list_head *entry)
{
	entry->next->prev = entry->prev;
	entry->prev->next = entry->next;
}

// 删除指定节点
static inline void
list_del(struct list_head *entry)
{
	_list_del(entry);
	entry->next = entry->prev = NULL;
}

// 插入指定节点(内部调用)
static inline void
_list_add(struct list_head *_new, struct list_head *prev,
    struct list_head *next)
{

	next->prev = _new;
	_new->next = next;
	_new->prev = prev;
	prev->next = _new;
}

// 将指定节点从原链表中删除，然后作为新链表的头节点完成初始化
static inline void
list_del_init(struct list_head *entry)
{
	_list_del(entry);
	INIT_LIST_HEAD(entry);
}

// 根据成员地址找到父结构地址
#define	list_entry(ptr, type, field)	container_of(ptr, type, field)
// 根据头节点找到带数据的首节点的父结构地址
#define	list_first_entry(ptr, type, field)	list_entry((ptr)->next, type, field)
// 根据头节点找到带数据的尾节点的父结构地址
#define	list_last_entry(ptr, type, field)	list_entry((ptr)->prev, type, field)

// 遍历链表
#define	list_for_each(p, head)						\
	for (p = (head)->next; p != (head); p = p->next)

// 遍历链表(安全模式,避免多线程操作时，后继节点被删除的风险)
#define	list_for_each_safe(p, n, head)					\
	for (p = (head)->next, n = p->next; p != (head); p = n, n = p->next)

// 遍历链表的父结构
#define list_for_each_entry(p, h, field)				\
	for (p = list_first_entry(h, typeof(*p), field); &p->field != (h); \
	    p = list_entry(p->field.next, typeof(*p), field))

// 遍历链表的父结构(安全模式)
#define list_for_each_entry_safe(p, n, h, field)			\
	for (p = list_first_entry(h, typeof(*p), field),		\
	    n = list_entry(p->field.next, typeof(*p), field); &p->field != (h);\
	    p = n, n = list_entry(n->field.next, typeof(*n), field))

#define	list_for_each_entry_reverse(p, h, field)			\
	for (p = list_last_entry(h, typeof(*p), field); &p->field != (h); \
	    p = list_entry(p->field.prev, typeof(*p), field))

#define	list_for_each_prev(p, h) for (p = (h)->prev; p != (h); p = p->prev)
#define	list_for_each_prev_safe(p, n, h) for (p = (h)->prev, n = p->prev; p != (h); p = n, n = p->prev)

/* 将_new节点插入到head节点的后一个节点位置
 *
 * 备注：如果head节点是链表的头结构，则实际是插入到链表首部
 *       如果head节点是一个普通节点，则实际是插入到后一个节点位置
 */
static inline void
list_add(struct list_head *_new, struct list_head *head)
{
	_list_add(_new, head, head->next);
}

/* 将_new节点插入到head节点的前一个节点位置
 *
 * 备注：如果head节点是链表的头结构，则实际是插入到链表尾部
 *       如果head节点是一个普通节点，则实际是插入到前一个节点位置
 */
static inline void
list_add_tail(struct list_head *_new, struct list_head *head)
{
	_list_add(_new, head->prev, head);
}

static inline void
list_move(struct list_head *list, struct list_head *head)
{
	_list_del(list);
	list_add(list, head);
}

static inline void
list_move_tail(struct list_head *entry, struct list_head *head)
{
	_list_del(entry);
	list_add_tail(entry, head);
}

static inline void
_list_splice(const struct list_head *list, struct list_head *prev,
    struct list_head *next)
{
	struct list_head *first;
	struct list_head *last;

	if (list_empty(list))
		return;

	first = list->next;
	last = list->prev;
	first->prev = prev;
	prev->next = first;
	last->next = next;
	next->prev = last;
}

static inline void
list_splice(const struct list_head *list, struct list_head *head)
{
	_list_splice(list, head, head->next);
}

static inline void
list_splice_tail(struct list_head *list, struct list_head *head)
{
	_list_splice(list, head->prev, head);
}

static inline void
list_splice_init(struct list_head *list, struct list_head *head)
{
	_list_splice(list, head, head->next);
	INIT_LIST_HEAD(list);
}

static inline void
list_splice_tail_init(struct list_head *list, struct list_head *head)
{
	_list_splice(list, head->prev, head);
	INIT_LIST_HEAD(list);
}

#endif /* _LINUX_LIST_H_ */
