#ifndef _AF_NETLINK_H
#define _AF_NETLINK_H

#include <net/sock.h>

#define NLGRPSZ(x)	(ALIGN(x, sizeof(unsigned long) * 8) / 8)
#define NLGRPLONGS(x)	(NLGRPSZ(x)/sizeof(unsigned long))

struct netlink_ring {
	void			**pg_vec;
	unsigned int		head;
	unsigned int		frames_per_block;
	unsigned int		frame_size;
	unsigned int		frame_max;

	unsigned int		pg_vec_order;
	unsigned int		pg_vec_pages;
	unsigned int		pg_vec_len;

	atomic_t		pending;
};

// 内核这边的netlink-socket控制块
struct netlink_sock {
	/* struct sock has to be the first member of netlink_sock */
	struct sock		sk;     // 通用的socket控制块
	u32			portid;     // 记录了本socket绑定的id号，对内核来说就是0
	u32			dst_portid; // 记录了目的id号，用户空间bind()时设定，通常就是对应的进程id
	u32			dst_group;
	u32			flags;
	u32			subscriptions;
	u32			ngroups;
	unsigned long		*groups;
	unsigned long		state;
	wait_queue_head_t	wait;
	bool			cb_running; // 用来标志该netlink-socket是否处于dump操作中
	struct netlink_callback	cb; // 用来记录该netlink-socket当前有效的操作集合
	struct mutex		*cb_mutex;
	struct mutex		cb_def_mutex;
	void			(*netlink_rcv)(struct sk_buff *skb);    // 指向所属的某个netlink协议的input回调函数
	void			(*netlink_bind)(int group);             // 指向某个netlink协议自身特有的bind操作(如果未指定，就采用通用策略)
	struct module		*module;
#ifdef CONFIG_NETLINK_MMAP
	struct mutex		pg_vec_lock;
	struct netlink_ring	rx_ring;
	struct netlink_ring	tx_ring;
	atomic_t		mapped;
#endif /* CONFIG_NETLINK_MMAP */
};

// 获取指定socket控制块所属的netlink-socket控制块
static inline struct netlink_sock *nlk_sk(struct sock *sk)
{
	return container_of(sk, struct netlink_sock, sk);
}

// netlink每个协议表项中的hash表控制块
struct nl_portid_hash {
	struct hlist_head	*table;     // 这里才真正指向一张hash表
	unsigned long		rehash_time;

	unsigned int		mask;
	unsigned int		shift;

	unsigned int		entries;
	unsigned int		max_shift;

	u32			rnd;
};

// netlink表每个表项的数据结构
struct netlink_table {
	struct nl_portid_hash	hash;       // hash表控制块，这张hash表用来索引同种协议类型的不同netlink套接字实例
	struct hlist_head	mc_list;        // 这是用于组播的hash表
	struct listeners __rcu	*listeners; // 记录了监听者的掩码
	unsigned int		flags;
	unsigned int		groups;         // 记录了该协议类型支持的最大组播数量
	struct mutex		*cb_mutex;
	struct module		*module;
	void			(*bind)(int group);
	bool			(*compare)(struct net *net, struct sock *sock); // net比较函数
	int			registered;     // 标记是否注册，1=标记 0=未标记
};

extern struct netlink_table *nl_table;
extern rwlock_t nl_table_lock;

#endif
