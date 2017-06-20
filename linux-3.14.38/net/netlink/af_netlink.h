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

// 内核这边的netlink套接字
struct netlink_sock {
	/* struct sock has to be the first member of netlink_sock */
	struct sock		sk;     // 该netlink套接字的sock结构
	u32			portid;     // 记录了该netlink套接字绑定的单播地址，对内核来说就是0
	u32			dst_portid; // 记录了目的id号，用户空间bind()时设定，通常就是对应的进程id
	u32			dst_group;
	u32			flags;
	u32			subscriptions;  // 记录该netlink套接字当前阅订的组播数量
	u32			ngroups;        // 记录该netlink套接字支持的最大组播数量
	unsigned long		*groups;// 指向该netlink套接字的组播空间
	unsigned long		state;
	wait_queue_head_t	wait;
	bool			cb_running; // 用来标志该netlink-socket是否处于dump操作中
	struct netlink_callback	cb; // 用来记录该netlink-socket当前有效的操作集合
	struct mutex		*cb_mutex;
	struct mutex		cb_def_mutex;
	void			(*netlink_rcv)(struct sk_buff *skb);    // 指向所属的某个netlink协议的input回调函数
	void			(*netlink_bind)(int group);             // 指向某个netlink协议自身特有的bind操作(如果未指定，就采用netlink通用策略)
	struct module		*module;
#ifdef CONFIG_NETLINK_MMAP
	struct mutex		pg_vec_lock;
	struct netlink_ring	rx_ring;
	struct netlink_ring	tx_ring;
	atomic_t		mapped;
#endif /* CONFIG_NETLINK_MMAP */
};

// 获取sock结构所属的netlink套接字
static inline struct netlink_sock *nlk_sk(struct sock *sk)
{
	return container_of(sk, struct netlink_sock, sk);
}

// netlink每个协议表项中的hash表控制块，记录了同种协议类型的不同netlink套接字实例，通过portid和net(网络命名空间)进行索引
struct nl_portid_hash {
	struct hlist_head	*table;     // 这里才真正指向一张hash表 "table[]"
	unsigned long		rehash_time;

	unsigned int		mask;
	unsigned int		shift;

	unsigned int		entries;    // 记录了该hash表当前记录的netlink套接字数量
	unsigned int		max_shift;

	u32			rnd;
};

// netlink表每个表项的数据结构
struct netlink_table {
	struct nl_portid_hash	hash;       // hash表控制块，这张hash表用来索引同种协议类型的不同netlink套接字实例
	struct hlist_head	mc_list;        // 这个hash表头节点用于记录该协议类型下阅订了组播功能的套接字
	struct listeners __rcu	*listeners; // 集合该协议类型下所有阅订了的组播(同种协议类型下不同套接字监听集合)
	unsigned int		flags;          // 这里的标志位来自配置netlink_kernel_cfg,目前主要记录了该协议类型允许的用户态操作权限
	unsigned int		groups;         // 记录了该协议类型支持的最大组播数量
	struct mutex		*cb_mutex;      // 记录了该协议类型的锁
	struct module		*module;
	void			(*bind)(int group);
	bool			(*compare)(struct net *net, struct sock *sock); // 网络命名空间比较函数
	int			registered;     // 标记该协议类型是否已经注册，0表示未注册，>=1表示已经有注册过
};

extern struct netlink_table *nl_table;
extern rwlock_t nl_table_lock;

#endif
