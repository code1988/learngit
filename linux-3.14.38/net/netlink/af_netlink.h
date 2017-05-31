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

// netlink套接字控制块
struct netlink_sock {
	/* struct sock has to be the first member of netlink_sock */
	struct sock		sk;     // 套接口在网络层的表示
	u32			portid;     // 内核自己的pid，为0
	u32			dst_portid;
	u32			dst_group;
	u32			flags;
	u32			subscriptions;
	u32			ngroups;
	unsigned long		*groups;
	unsigned long		state;
	wait_queue_head_t	wait;
	bool			cb_running;
	struct netlink_callback	cb;
	struct mutex		*cb_mutex;
	struct mutex		cb_def_mutex;
	void			(*netlink_rcv)(struct sk_buff *skb);
	void			(*netlink_bind)(int group);
	struct module		*module;
#ifdef CONFIG_NETLINK_MMAP
	struct mutex		pg_vec_lock;
	struct netlink_ring	rx_ring;
	struct netlink_ring	tx_ring;
	atomic_t		mapped;
#endif /* CONFIG_NETLINK_MMAP */
};

static inline struct netlink_sock *nlk_sk(struct sock *sk)
{
	return container_of(sk, struct netlink_sock, sk);
}

struct nl_portid_hash {
	struct hlist_head	*table;
	unsigned long		rehash_time;

	unsigned int		mask;
	unsigned int		shift;

	unsigned int		entries;
	unsigned int		max_shift;

	u32			rnd;
};

// netlink表每个表项的数据结构
struct netlink_table {
	struct nl_portid_hash	hash;       // hash表
	struct hlist_head	mc_list;
	struct listeners __rcu	*listeners;
	unsigned int		flags;
	unsigned int		groups;         // 多播组
	struct mutex		*cb_mutex;
	struct module		*module;
	void			(*bind)(int group);
	bool			(*compare)(struct net *net, struct sock *sock); // net比较函数
	int			registered;     // 标记是否注册，1=标记 0=未标记
};

extern struct netlink_table *nl_table;
extern rwlock_t nl_table_lock;

#endif
