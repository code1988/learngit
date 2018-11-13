#ifndef _AF_NETLINK_H
#define _AF_NETLINK_H

#include <linux/rhashtable.h>
#include <linux/atomic.h>
#include <linux/workqueue.h>
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

// 定义了netlink套接字结构
struct netlink_sock {
	/* struct sock has to be the first member of netlink_sock */
	struct sock		sk;         // 该netlink套接字的sock结构
	u32			portid;         // 记录了该netlink套接字绑定的单播地址，对内核来说就是0
	u32			dst_portid;     // 记录了该netlink套接字的默认目的单播地址(缺省为0,当用户进程调用connect时可以指定)
	u32			dst_group;      // 记录了该netlink套接字的默认目的组播地址(缺省为0,当用户进程调用connect时可以指定)
	u32			flags;          // 用来标识该netlink套接字的属性，比如NETLINK_KERNEL_SOCKET
	u32			subscriptions;  // 记录该netlink套接字当前阅订的组播数量
	u32			ngroups;        // 记录该netlink套接字支持的最大组播数量
	unsigned long		*groups;// 指向该netlink套接字的组播空间
	unsigned long		state;  // 3.14.38版本中只用来设置拥挤标志
	size_t			max_recvmsg_len;
	wait_queue_head_t	wait;   // 该netlink套接字的等待队列，当接收队列拥挤时，那些继续发送netlink单播消息到该套接字的用户发送进程将会加入等待队列
	bool			bound;
	bool			cb_running;         // 用来标志该netlink套接字是否处于dump操作中
	struct netlink_callback	cb;         // 用来记录该netlink套接字当前有效的操作集合
	struct mutex		*cb_mutex;      // 这把锁在内核netlink套接字创建时传入，相同协议类型的netlink套接字共用一把锁
	struct mutex		cb_def_mutex;
	void			(*netlink_rcv)(struct sk_buff *skb);    // 指向具体协议类型特有的input回调函数(只对内核netlink套接字有意义)
	int			(*netlink_bind)(struct net *net, int group);// 指向具体协议类型特有的bind操作(只对用户进程netlink套接字有意义，从所属netlink_table中的bind成员继承)
	void			(*netlink_unbind)(struct net *net, int group);
	struct module		*module;
#ifdef CONFIG_NETLINK_MMAP
	struct mutex		pg_vec_lock;
	struct netlink_ring	rx_ring;
	struct netlink_ring	tx_ring;
	atomic_t		mapped;
#endif /* CONFIG_NETLINK_MMAP */

	struct rhash_head	node;
	struct rcu_head		rcu;
	struct work_struct	work;
};

// 获取sock结构所属的netlink套接字
static inline struct netlink_sock *nlk_sk(struct sock *sk)
{
	return container_of(sk, struct netlink_sock, sk);
}

static inline bool netlink_skb_is_mmaped(const struct sk_buff *skb)
{
#ifdef CONFIG_NETLINK_MMAP
	return NETLINK_CB(skb).flags & NETLINK_SKB_MMAPED;
#else
	return false;
#endif /* CONFIG_NETLINK_MMAP */
}

// netlink表每个表项的数据结构，该结构维护了一个具体的netlink协议
struct netlink_table {
	struct rhashtable	hash;           // hash表控制块，内部的hash表记录了已经创建的同种协议类型的所有netlink套接字
	struct hlist_head	mc_list;        // 这个hash桶头节点用于记录同种协议类型下所有阅订了组播功能的套接字
	struct listeners __rcu	*listeners; // 记录了同种协议类型下所有被阅订了的组播消息集合
	unsigned int		flags;          // 这里的标志位来自配置netlink_kernel_cfg,目前主要记录了该协议类型允许的用户态操作权限
	unsigned int		groups;         // 记录了该协议类型支持的最大组播数量
	struct mutex		*cb_mutex;      // 记录了该协议类型的锁
	struct module		*module;
	int			(*bind)(struct net *net, int group);        // 该协议类型私有的bind函数
	void			(*unbind)(struct net *net, int group);
	bool			(*compare)(struct net *net, struct sock *sock); // 该协议私有的net命名空间比较函数
	int			registered;             // 标记该协议类型是否已经注册，0表示未注册，>=1表示已经有注册过
};

extern struct netlink_table *nl_table;
extern rwlock_t nl_table_lock;

#endif
