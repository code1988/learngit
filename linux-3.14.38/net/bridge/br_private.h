/*
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#ifndef _BR_PRIVATE_H
#define _BR_PRIVATE_H

#include <linux/netdevice.h>
#include <linux/if_bridge.h>
#include <linux/netpoll.h>
#include <linux/u64_stats_sync.h>
#include <net/route.h>
#include <linux/if_vlan.h>

#define BR_HASH_BITS 8
#define BR_HASH_SIZE (1 << BR_HASH_BITS)                // 转发表的桶数量，这里就是256

#define BR_HOLD_TIME (1*HZ)

#define BR_PORT_BITS	10
#define BR_MAX_PORTS	(1<<BR_PORT_BITS)               // 网桥支持的最大端口数量，这里是1024个
#define BR_VLAN_BITMAP_LEN	BITS_TO_LONGS(VLAN_N_VID)   // 32位平台:(4096 + 31)/32

#define BR_VERSION	"2.3"

/* Control of forwarding link local multicast */
#define BR_GROUPFWD_DEFAULT	0
/* Don't allow forwarding control protocols like STP and LLDP 
 * 不允许转发控制协议报文(比如stp、lldp报文等)*/
#define BR_GROUPFWD_RESTRICTED	0x4007u

/* Path to usermode spanning tree program 用户态生成树(rstp/mstp)程序的路径 */
#define BR_STP_PROG	"/sbin/bridge-stp"

typedef struct bridge_id bridge_id;
typedef struct mac_addr mac_addr;
typedef __u16 port_id;      // 端口ID，由优先级和端口号组成

// 网桥ID结构
struct bridge_id
{
	unsigned char	prio[2];    // 优先级
	unsigned char	addr[6];    // 网桥设备MAC，来自最小的桥端口MAC
};

struct mac_addr
{
	unsigned char	addr[6];
};

struct br_ip
{
	union {
		__be32	ip4;
#if IS_ENABLED(CONFIG_IPV6)
		struct in6_addr ip6;
#endif
	} u;
	__be16		proto;
	__u16		vid;
};

#ifdef CONFIG_BRIDGE_IGMP_SNOOPING
/* our own querier 本地查询器*/
struct bridge_mcast_query {
	struct timer_list	timer;  // 查询定时器
	u32			startup_sent;   // 已发送的查询包数量 
};

/* other querier 其他查询器*/
struct bridge_mcast_querier {
	struct timer_list		timer;
	unsigned long			delay_time;
};
#endif

// 本结构记录了 网桥/桥端口 的vlan相关信息(网桥在这里其实被看做一个特殊的桥端口)
struct net_port_vlans {
	u16				port_idx;       
	u16				pvid;
	union {
		struct net_bridge_port		*port;
		struct net_bridge		*br;
	}				parent;     // 指向该vlan结构关联的网桥或桥端口
	struct rcu_head			rcu;
	unsigned long			vlan_bitmap[BR_VLAN_BITMAP_LEN];        // vlan id集合，每位代表一个vlan id，置1表示对应的vlan有效
	unsigned long			untagged_bitmap[BR_VLAN_BITMAP_LEN];
	u16				num_vlans;  // 该vlan结构记录的vlan数量
};

// 定义了转发表表项结构
struct net_bridge_fdb_entry
{
	struct hlist_node		hlist;      // 用来链接同一个hash桶中的其他net_bridge_fdb_entry结构
	struct net_bridge_port		*dst;   // 指向学习到该mac的桥端口

	struct rcu_head			rcu;
	unsigned long			updated;    // 记录了该表项被刷新时的jiffies
	unsigned long			used;
	mac_addr			addr;           // 记录了一个学习到的mac
	unsigned char			is_local;   // 标识该mac是否是自身mac
	unsigned char			is_static;  // 标识该mac是否是静态添加的
	unsigned char			added_by_user;  // 标识该mac是否是用户手动添加
	__u16				vlan_id;            // 记录了该mac所在的vlan
};

/* 定义了加入一个组播组的组播端口，本结构描述了一个组播端口的详细信息
 */
struct net_bridge_port_group {
	struct net_bridge_port		*port;          // 该组播端口对应的网桥端口
	struct net_bridge_port_group __rcu *next;   // 指向下一个相同组播组中的组播端口
	struct hlist_node		mglist;
	struct rcu_head			rcu;
	struct timer_list		timer;              // 该组播端口老化定时器
	struct br_ip			addr;               // 所在组播组的地址
	unsigned char			state;
};

/* 定义了组播组数据库转发表项，本结构描述了一个组播组的详细信息
 */
struct net_bridge_mdb_entry
{
	struct hlist_node		hlist[2];
	struct net_bridge		*br;                // 指向所属的网桥
	struct net_bridge_port_group __rcu *ports;  // 加入了该组播组的组播端口链表
	struct rcu_head			rcu;
	struct timer_list		timer;              // 该组播组老化定时器
	struct br_ip			addr;               // 该组播组的地址
	bool				mglist;
};

/* 定义了总的组播组数据库转发表
 */
struct net_bridge_mdb_htable
{
	struct hlist_head		*mhash; // 指向一张hash表，该hash表存储了所有的net_bridge_mdb_entry结构
	struct rcu_head			rcu;
	struct net_bridge_mdb_htable	*old;
	u32				size;           // hash表中存储的net_bridge_mdb_entry总数
	u32				max;            // hash表的最大值
	u32				secret;
	u32				ver;
};

// 定义了网桥端口，本结构描述了一个桥端口的详细信息
struct net_bridge_port
{
	struct net_bridge		*br;    // 指向所属网桥
	struct net_device		*dev;   // 指向对应的网络设备
	struct list_head		list;   // 用来链接所属网桥的其他桥端口

	/* STP */
	u8				priority;       // 端口优先级
	u8				state;          // 桥端口的状态,BR_STATE_*
	u16				port_no;        // 桥端口号(不同于设备的接口号，但实际使用时通常会保持一致)
	unsigned char			topology_change_ack;    // 标示该桥端口是否需要回复一个TCA(收到TCN-BPDU时置1，回复了携带TCA的配置BPDU后清0)
	unsigned char			config_pending; // 发送配置BPDU时如果hold定时器处于开启状态时置1,用于控制配置BPDU发送频率
	port_id			    port_id;            // 该STP端口ID号(优先级+桥端口号)
	u32				    path_cost;          // 该桥端口的路径成本
	port_id			designated_port;    // 所在链路的指定端口: 对于"指定端口"就是本身，对于"根端口"就是链路对端STP端口
	bridge_id		designated_bridge;  // 所在链路的指定桥: 对于"指定端口"就是所在网桥，对于"根端口"就是链路对端STP网桥
	bridge_id		designated_root;    // 指定根桥: 对于"指定端口"就是所在网桥的"根桥"，对于"根端口"就是其"指定桥"的"根桥"
	u32				designated_cost;    // 指定根路径开销: 对于"指定端口"就是所在网桥的根路径开销，对于"根端口"就是其"指定桥"的根路径开销
	unsigned long	designated_age;     // 指定年龄(只对"根端口有效")：jiffies - bpdu->message_age

	struct timer_list		forward_delay_timer;    // 该STP端口控制listening->learning和learning->forwarding状态切换时延的定时器，间隔forward_delay
	struct timer_list		hold_timer;             // 用于控制配置BPDU发送频率的定时器，间隔BR_HOLD_TIME
	struct timer_list		message_age_timer;      // 只用于"根端口"的message_age定时器，每次收到配置BPDU时刷新，超时时间为剩余的消息年龄
	struct kobject			kobj;
	struct rcu_head			rcu;

	unsigned long 			flags;          // 桥端口的标志位集合:
#define BR_HAIRPIN_MODE		0x00000001      // 
#define BR_BPDU_GUARD           0x00000002  // 表示不会从该端口收到BPDU(一旦收到将会报错并disable该端口)
#define BR_ROOT_BLOCK		0x00000004      // 表示该端口被block
#define BR_MULTICAST_FAST_LEAVE	0x00000008  //
#define BR_ADMIN_COST		0x00000010      //
#define BR_LEARNING		0x00000020          // 表示具备学习功能
#define BR_FLOOD		0x00000040          // 表示具备泛洪功能

#ifdef CONFIG_BRIDGE_IGMP_SNOOPING
	struct bridge_mcast_query	ip4_query;
#if IS_ENABLED(CONFIG_IPV6)
	struct bridge_mcast_query	ip6_query;
#endif /* IS_ENABLED(CONFIG_IPV6) */
	unsigned char			multicast_router;
	struct timer_list		multicast_router_timer;
	struct hlist_head		mglist;
	struct hlist_node		rlist;
#endif

#ifdef CONFIG_SYSFS
	char				sysfs_name[IFNAMSIZ];
#endif

#ifdef CONFIG_NET_POLL_CONTROLLER
	struct netpoll			*np;
#endif
#ifdef CONFIG_BRIDGE_VLAN_FILTERING
	struct net_port_vlans __rcu	*vlan_info; // 指向该桥端口vlan相关信息
#endif
};

#define br_port_exists(dev) (dev->priv_flags & IFF_BRIDGE_PORT)

// 基于桥端口设备获取关联的net_bridge_port
static inline struct net_bridge_port *br_port_get_rcu(const struct net_device *dev)
{
	return rcu_dereference(dev->rx_handler_data);
}

// 基于桥端口设备获取关联的net_bridge_port
static inline struct net_bridge_port *br_port_get_rtnl(const struct net_device *dev)
{
	return br_port_exists(dev) ?
		rtnl_dereference(dev->rx_handler_data) : NULL;
}

// 定义了网桥结构，本结构描述了一个网桥的详细信息
struct net_bridge
{
	spinlock_t			lock;
	struct list_head		port_list;  // 桥端口的链表头，每个节点就是一个net_bridge_port结构
	struct net_device		*dev;       // 指向对应的网桥设备

	struct pcpu_sw_netstats		__percpu *stats;
	spinlock_t			hash_lock;              // 用于下面这张转发表的自旋锁
	struct hlist_head		hash[BR_HASH_SIZE]; // 该网桥基于hash结构的转发表，散列桶中的每个节点就是一个net_bridge_fdb_entry结构
#ifdef CONFIG_BRIDGE_NETFILTER
	struct rtable 			fake_rtable;
	bool				nf_call_iptables;
	bool				nf_call_ip6tables;
	bool				nf_call_arptables;
#endif
	u16				group_fwd_mask;

	/* STP */
	bridge_id			designated_root;    // "根桥ID"
	bridge_id			bridge_id;          // 该网桥ID号
	u32				root_path_cost;         // 从该网桥的根路径开销(显然对于根桥来说为0；对于非根桥就是"根端口"的"指定根路径开销" + "根端口"的路径开销)

	unsigned long			max_age;        // (来自根桥的)最大消息生存时间(配置信息老化时间)，缺省20s
	unsigned long			hello_time;     // (来自根桥的)定时发送配置BPDU信息的间隔，缺省2s
	unsigned long			forward_delay;  // (来自根桥的)桥端口从listening->learning或者从learning->forwarding转换时间，缺省15s

	unsigned long			ageing_time;    // 桥fdb老化时间，缺省5min

	unsigned long			bridge_max_age;         // (网桥自身的)配置信息老化时间
	unsigned long			bridge_hello_time;      // (网桥自身的)发送配置BPDU信息的间隔
	unsigned long			bridge_forward_delay;   // (网桥自身的)桥端口从listening->learning或者从learning->forwarding转换时间

	u8				group_addr[ETH_ALEN];   // stp组播地址，缺省就是01:80:c2:00:00:00
	u16				root_port;              // 根端口(显然对于根桥来说根端口不存在，该字段为0)

	enum {
		BR_NO_STP, 		/* no spanning tree */
		BR_KERNEL_STP,		/* old STP in kernel */
		BR_USER_STP,		/* new RSTP in userspace */
	} stp_enabled;  // 网桥的stp功能开关

	unsigned char			topology_change;            // 用于标识网络拓扑改变(只能被"根桥"发来的TC置1的配置BPDU设置)
	unsigned char			topology_change_detected;   // 用于标识探测到网络拓扑改变(用于所有网桥)

#ifdef CONFIG_BRIDGE_IGMP_SNOOPING
	unsigned char			multicast_router;

	u8				multicast_disabled:1;       // igmp-snooping使能开关
	u8				multicast_querier:1;
	u8				multicast_query_use_ifaddr:1;

	u32				hash_elasticity;    // 每个组播组中能关联的端口个数
	u32				hash_max;           // net_bridge_mdb_htable结构中hash数组的最大值

	u32				multicast_last_member_count;
	u32				multicast_startup_query_count;

	unsigned long			multicast_last_member_interval;
	unsigned long			multicast_membership_interval;
	unsigned long			multicast_querier_interval;
	unsigned long			multicast_query_interval;           // 查询包的发送间隔
	unsigned long			multicast_query_response_interval;  // 组播查询最大回复时间
	unsigned long			multicast_startup_query_interval;

	spinlock_t			multicast_lock;
	struct net_bridge_mdb_htable __rcu *mdb;
	struct hlist_head		router_list;

	struct timer_list		multicast_router_timer;     // 定时器
	struct bridge_mcast_querier	ip4_querier;            // ipv4类型的本地查询器
	struct bridge_mcast_query	ip4_query;              // ipv4类型的其他查询器
#if IS_ENABLED(CONFIG_IPV6)
	struct bridge_mcast_querier	ip6_querier;
	struct bridge_mcast_query	ip6_query;
#endif /* IS_ENABLED(CONFIG_IPV6) */
#endif

	struct timer_list		hello_timer;            // "根桥"定时发送配置BPDU信息的定时器，间隔hello_time(只对"根桥"有用，当发现自己不是"根桥"时关闭)
	struct timer_list		tcn_timer;              // "非根桥"发送TCN-BPDU信息的定时器，间隔bridge_hello_time(只对"非根桥"有用，当收到回复的TCA时关闭)
	struct timer_list		topology_change_timer;  // "根桥"结束发送TC置1的配置BPDU信息的定时器，超时bridge_max_age + bridge_forward_delay(只对"根桥"有用)
	struct timer_list		gc_timer;
	struct kobject			*ifobj;
#ifdef CONFIG_BRIDGE_VLAN_FILTERING
	u8				vlan_enabled;               // 标示该网桥是否使能了vlan过滤功能
	struct net_port_vlans __rcu	*vlan_info;     // 指向该网桥vlan相关信息
#endif
};

// skb中针对bridge模块报文的附加信息
struct br_input_skb_cb {
	struct net_device *brdev;       // 指向对应的skb所属的网桥设备
#ifdef CONFIG_BRIDGE_IGMP_SNOOPING
	int igmp;
	int mrouters_only;
#endif
#ifdef CONFIG_BRIDGE_VLAN_FILTERING
	bool vlan_filtered;     // 用于标识对应的skb是否经过了网桥的vlan过滤
#endif
};

// 将skb结构中的cb字段自定义用于保存bridge参数控制块
#define BR_INPUT_SKB_CB(__skb)	((struct br_input_skb_cb *)(__skb)->cb)

#ifdef CONFIG_BRIDGE_IGMP_SNOOPING
# define BR_INPUT_SKB_CB_MROUTERS_ONLY(__skb)	(BR_INPUT_SKB_CB(__skb)->mrouters_only)
#else
# define BR_INPUT_SKB_CB_MROUTERS_ONLY(__skb)	(0)
#endif

#define br_printk(level, br, format, args...)	\
	printk(level "%s: " format, (br)->dev->name, ##args)

#define br_err(__br, format, args...)			\
	br_printk(KERN_ERR, __br, format, ##args)
#define br_warn(__br, format, args...)			\
	br_printk(KERN_WARNING, __br, format, ##args)
#define br_notice(__br, format, args...)		\
	br_printk(KERN_NOTICE, __br, format, ##args)
#define br_info(__br, format, args...)			\
	br_printk(KERN_INFO, __br, format, ##args)

#define br_debug(br, format, args...)			\
	pr_debug("%s: " format,  (br)->dev->name, ##args)

extern struct notifier_block br_device_notifier;

/* called under bridge lock 
 * 判断该网桥桥是否为根桥，是根桥则返回1,不是根桥则返回0
 * */
static inline int br_is_root_bridge(const struct net_bridge *br)
{
	return !memcmp(&br->bridge_id, &br->designated_root, 8);
}

/* br_device.c */
void br_dev_setup(struct net_device *dev);
void br_dev_delete(struct net_device *dev, struct list_head *list);
netdev_tx_t br_dev_xmit(struct sk_buff *skb, struct net_device *dev);
#ifdef CONFIG_NET_POLL_CONTROLLER
static inline void br_netpoll_send_skb(const struct net_bridge_port *p,
				       struct sk_buff *skb)
{
	struct netpoll *np = p->np;

	if (np)
		netpoll_send_skb(np, skb);
}

int br_netpoll_enable(struct net_bridge_port *p, gfp_t gfp);
void br_netpoll_disable(struct net_bridge_port *p);
#else
static inline void br_netpoll_send_skb(const struct net_bridge_port *p,
				       struct sk_buff *skb)
{
}

static inline int br_netpoll_enable(struct net_bridge_port *p, gfp_t gfp)
{
	return 0;
}

static inline void br_netpoll_disable(struct net_bridge_port *p)
{
}
#endif

/* br_fdb.c */
int br_fdb_init(void);
void br_fdb_fini(void);
void br_fdb_flush(struct net_bridge *br);
void br_fdb_find_delete_local(struct net_bridge *br,
			      const struct net_bridge_port *p,
			      const unsigned char *addr, u16 vid);
void br_fdb_changeaddr(struct net_bridge_port *p, const unsigned char *newaddr);
void br_fdb_change_mac_address(struct net_bridge *br, const u8 *newaddr);
void br_fdb_cleanup(unsigned long arg);
void br_fdb_delete_by_port(struct net_bridge *br,
			   const struct net_bridge_port *p, int do_all);
struct net_bridge_fdb_entry *__br_fdb_get(struct net_bridge *br,
					  const unsigned char *addr, __u16 vid);
int br_fdb_test_addr(struct net_device *dev, unsigned char *addr);
int br_fdb_fillbuf(struct net_bridge *br, void *buf, unsigned long count,
		   unsigned long off);
int br_fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		  const unsigned char *addr, u16 vid);
void br_fdb_update(struct net_bridge *br, struct net_bridge_port *source,
		   const unsigned char *addr, u16 vid, bool added_by_user);

int br_fdb_delete(struct ndmsg *ndm, struct nlattr *tb[],
		  struct net_device *dev, const unsigned char *addr);
int br_fdb_add(struct ndmsg *nlh, struct nlattr *tb[], struct net_device *dev,
	       const unsigned char *addr, u16 nlh_flags);
int br_fdb_dump(struct sk_buff *skb, struct netlink_callback *cb,
		struct net_device *dev, int idx);

/* br_forward.c */
void br_deliver(const struct net_bridge_port *to, struct sk_buff *skb);
int br_dev_queue_push_xmit(struct sk_buff *skb);
void br_forward(const struct net_bridge_port *to,
		struct sk_buff *skb, struct sk_buff *skb0);
int br_forward_finish(struct sk_buff *skb);
void br_flood_deliver(struct net_bridge *br, struct sk_buff *skb, bool unicast);
void br_flood_forward(struct net_bridge *br, struct sk_buff *skb,
		      struct sk_buff *skb2, bool unicast);

/* br_if.c */
void br_port_carrier_check(struct net_bridge_port *p);
int br_add_bridge(struct net *net, const char *name);
int br_del_bridge(struct net *net, const char *name);
int br_add_if(struct net_bridge *br, struct net_device *dev);
int br_del_if(struct net_bridge *br, struct net_device *dev);
int br_min_mtu(const struct net_bridge *br);
netdev_features_t br_features_recompute(struct net_bridge *br,
					netdev_features_t features);

/* br_input.c */
int br_handle_frame_finish(struct sk_buff *skb);
rx_handler_result_t br_handle_frame(struct sk_buff **pskb);

// 检查指定设备是否是一个桥端口设备
static inline bool br_rx_handler_check_rcu(const struct net_device *dev)
{
	return rcu_dereference(dev->rx_handler) == br_handle_frame;
}

// 如果指定设备是桥端口设备则返回对应的桥端口结构，否则返回NULL
static inline struct net_bridge_port *br_port_get_check_rcu(const struct net_device *dev)
{
	return br_rx_handler_check_rcu(dev) ? br_port_get_rcu(dev) : NULL;
}

/* br_ioctl.c */
int br_dev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
int br_ioctl_deviceless_stub(struct net *net, unsigned int cmd,
			     void __user *arg);

/* br_multicast.c */
#ifdef CONFIG_BRIDGE_IGMP_SNOOPING      // 以下是配置了CONFIG_BRIDGE_IGMP_SNOOPING的相关内容
extern unsigned int br_mdb_rehash_seq;
int br_multicast_rcv(struct net_bridge *br, struct net_bridge_port *port,
		     struct sk_buff *skb, u16 vid);
struct net_bridge_mdb_entry *br_mdb_get(struct net_bridge *br,
					struct sk_buff *skb, u16 vid);
void br_multicast_add_port(struct net_bridge_port *port);
void br_multicast_del_port(struct net_bridge_port *port);
void br_multicast_enable_port(struct net_bridge_port *port);
void br_multicast_disable_port(struct net_bridge_port *port);
void br_multicast_init(struct net_bridge *br);
void br_multicast_open(struct net_bridge *br);
void br_multicast_stop(struct net_bridge *br);
void br_multicast_deliver(struct net_bridge_mdb_entry *mdst,
			  struct sk_buff *skb);
void br_multicast_forward(struct net_bridge_mdb_entry *mdst,
			  struct sk_buff *skb, struct sk_buff *skb2);
int br_multicast_set_router(struct net_bridge *br, unsigned long val);
int br_multicast_set_port_router(struct net_bridge_port *p, unsigned long val);
int br_multicast_toggle(struct net_bridge *br, unsigned long val);
int br_multicast_set_querier(struct net_bridge *br, unsigned long val);
int br_multicast_set_hash_max(struct net_bridge *br, unsigned long val);
struct net_bridge_mdb_entry *
br_mdb_ip_get(struct net_bridge_mdb_htable *mdb, struct br_ip *dst);
struct net_bridge_mdb_entry *
br_multicast_new_group(struct net_bridge *br, struct net_bridge_port *port,
		       struct br_ip *group);
void br_multicast_free_pg(struct rcu_head *head);
struct net_bridge_port_group *
br_multicast_new_port_group(struct net_bridge_port *port, struct br_ip *group,
			    struct net_bridge_port_group __rcu *next,
			    unsigned char state);
void br_mdb_init(void);
void br_mdb_uninit(void);
void br_mdb_notify(struct net_device *dev, struct net_bridge_port *port,
		   struct br_ip *group, int type);

#define mlock_dereference(X, br) \
	rcu_dereference_protected(X, lockdep_is_held(&br->multicast_lock))

static inline bool br_multicast_is_router(struct net_bridge *br)
{
	return br->multicast_router == 2 ||
	       (br->multicast_router == 1 &&
		timer_pending(&br->multicast_router_timer));
}

static inline bool
__br_multicast_querier_exists(struct net_bridge *br,
			      struct bridge_mcast_querier *querier)
{
	return time_is_before_jiffies(querier->delay_time) &&
	       (br->multicast_querier || timer_pending(&querier->timer));
}

/* 检查对应的IGMP查询器是否存在
 *
 * 备注：显然本函数运行前提是配置了CONFIG_BRIDGE_IGMP_SNOOPING
 */
static inline bool br_multicast_querier_exists(struct net_bridge *br,
					       struct ethhdr *eth)
{
	switch (eth->h_proto) {
	case (htons(ETH_P_IP)):
		return __br_multicast_querier_exists(br, &br->ip4_querier);
#if IS_ENABLED(CONFIG_IPV6)
	case (htons(ETH_P_IPV6)):
		return __br_multicast_querier_exists(br, &br->ip6_querier);
#endif
	default:
		return false;
	}
}
#else       // 以下是没有配置CONFIG_BRIDGE_IGMP_SNOOPING的相关内容
static inline int br_multicast_rcv(struct net_bridge *br,
				   struct net_bridge_port *port,
				   struct sk_buff *skb,
				   u16 vid)
{
	return 0;
}

static inline struct net_bridge_mdb_entry *br_mdb_get(struct net_bridge *br,
						      struct sk_buff *skb, u16 vid)
{
	return NULL;
}

static inline void br_multicast_add_port(struct net_bridge_port *port)
{
}

static inline void br_multicast_del_port(struct net_bridge_port *port)
{
}

static inline void br_multicast_enable_port(struct net_bridge_port *port)
{
}

static inline void br_multicast_disable_port(struct net_bridge_port *port)
{
}

static inline void br_multicast_init(struct net_bridge *br)
{
}

static inline void br_multicast_open(struct net_bridge *br)
{
}

static inline void br_multicast_stop(struct net_bridge *br)
{
}

static inline void br_multicast_deliver(struct net_bridge_mdb_entry *mdst,
					struct sk_buff *skb)
{
}

static inline void br_multicast_forward(struct net_bridge_mdb_entry *mdst,
					struct sk_buff *skb,
					struct sk_buff *skb2)
{
}
static inline bool br_multicast_is_router(struct net_bridge *br)
{
	return 0;
}
static inline bool br_multicast_querier_exists(struct net_bridge *br,
					       struct ethhdr *eth)
{
	return false;
}
static inline void br_mdb_init(void)
{
}
static inline void br_mdb_uninit(void)
{
}
#endif

/* br_vlan.c */
#ifdef CONFIG_BRIDGE_VLAN_FILTERING     // 以下是配置了CONFIG_BRIDGE_VLAN_FILTERING的相关内容
bool br_allowed_ingress(struct net_bridge *br, struct net_port_vlans *v,
			struct sk_buff *skb, u16 *vid);
bool br_allowed_egress(struct net_bridge *br, const struct net_port_vlans *v,
		       const struct sk_buff *skb);
bool br_should_learn(struct net_bridge_port *p, struct sk_buff *skb, u16 *vid);
struct sk_buff *br_handle_vlan(struct net_bridge *br,
			       const struct net_port_vlans *v,
			       struct sk_buff *skb);
int br_vlan_add(struct net_bridge *br, u16 vid, u16 flags);
int br_vlan_delete(struct net_bridge *br, u16 vid);
void br_vlan_flush(struct net_bridge *br);
bool br_vlan_find(struct net_bridge *br, u16 vid);
int br_vlan_filter_toggle(struct net_bridge *br, unsigned long val);
int nbp_vlan_add(struct net_bridge_port *port, u16 vid, u16 flags);
int nbp_vlan_delete(struct net_bridge_port *port, u16 vid);
void nbp_vlan_flush(struct net_bridge_port *port);
bool nbp_vlan_find(struct net_bridge_port *port, u16 vid);

// 获取指定网桥的vlan信息(网桥其实被看做一个特殊的桥端口)
static inline struct net_port_vlans *br_get_vlan_info(
						const struct net_bridge *br)
{
	return rcu_dereference_rtnl(br->vlan_info);
}

// 获取指定桥端口的vlan信息
static inline struct net_port_vlans *nbp_get_vlan_info(
						const struct net_bridge_port *p)
{
	return rcu_dereference_rtnl(p->vlan_info);
}

/* Since bridge now depends on 8021Q module, but the time bridge sees the
 * skb, the vlan tag will always be present if the frame was tagged.
 * 从skb中获取vlan id  
 */
static inline int br_vlan_get_tag(const struct sk_buff *skb, u16 *vid)
{
	int err = 0;

    // 如果存在vlan id则返回相应值，如果不存在则返回vlan id = 0
	if (vlan_tx_tag_present(skb))
		*vid = vlan_tx_tag_get(skb) & VLAN_VID_MASK;
	else {
		*vid = 0;
		err = -EINVAL;
	}

	return err;
}

// 获取指定桥端口的pvid，存在则返回相应值，不存在则返回VLAN_N_VID
static inline u16 br_get_pvid(const struct net_port_vlans *v)
{
	/* Return just the VID if it is set, or VLAN_N_VID (invalid vid) if
	 * vid wasn't set
	 */
	smp_rmb();
	return v->pvid ?: VLAN_N_VID;
}

#else           // 以下是没有配置CONFIG_BRIDGE_VLAN_FILTERING时，显然相关函数都为空
static inline bool br_allowed_ingress(struct net_bridge *br,
				      struct net_port_vlans *v,
				      struct sk_buff *skb,
				      u16 *vid)
{
	return true;
}

static inline bool br_allowed_egress(struct net_bridge *br,
				     const struct net_port_vlans *v,
				     const struct sk_buff *skb)
{
	return true;
}

static inline bool br_should_learn(struct net_bridge_port *p,
				   struct sk_buff *skb, u16 *vid)
{
	return true;
}

static inline struct sk_buff *br_handle_vlan(struct net_bridge *br,
					     const struct net_port_vlans *v,
					     struct sk_buff *skb)
{
	return skb;
}

static inline int br_vlan_add(struct net_bridge *br, u16 vid, u16 flags)
{
	return -EOPNOTSUPP;
}

static inline int br_vlan_delete(struct net_bridge *br, u16 vid)
{
	return -EOPNOTSUPP;
}

static inline void br_vlan_flush(struct net_bridge *br)
{
}

static inline bool br_vlan_find(struct net_bridge *br, u16 vid)
{
	return false;
}

static inline int nbp_vlan_add(struct net_bridge_port *port, u16 vid, u16 flags)
{
	return -EOPNOTSUPP;
}

static inline int nbp_vlan_delete(struct net_bridge_port *port, u16 vid)
{
	return -EOPNOTSUPP;
}

static inline void nbp_vlan_flush(struct net_bridge_port *port)
{
}

static inline struct net_port_vlans *br_get_vlan_info(
						const struct net_bridge *br)
{
	return NULL;
}
static inline struct net_port_vlans *nbp_get_vlan_info(
						const struct net_bridge_port *p)
{
	return NULL;
}

static inline bool nbp_vlan_find(struct net_bridge_port *port, u16 vid)
{
	return false;
}

static inline u16 br_vlan_get_tag(const struct sk_buff *skb, u16 *tag)
{
	return 0;
}
static inline u16 br_get_pvid(const struct net_port_vlans *v)
{
	return VLAN_N_VID;	/* Returns invalid vid */
}
#endif

/* br_netfilter.c */
#ifdef CONFIG_BRIDGE_NETFILTER
int br_netfilter_init(void);
void br_netfilter_fini(void);
void br_netfilter_rtable_init(struct net_bridge *);
#else
#define br_netfilter_init()	(0)
#define br_netfilter_fini()	do { } while (0)
#define br_netfilter_rtable_init(x)
#endif

/* br_stp.c */
void br_log_state(const struct net_bridge_port *p);
struct net_bridge_port *br_get_port(struct net_bridge *br, u16 port_no);
void br_init_port(struct net_bridge_port *p);
void br_become_designated_port(struct net_bridge_port *p);

void __br_set_forward_delay(struct net_bridge *br, unsigned long t);
int br_set_forward_delay(struct net_bridge *br, unsigned long x);
int br_set_hello_time(struct net_bridge *br, unsigned long x);
int br_set_max_age(struct net_bridge *br, unsigned long x);


/* br_stp_if.c */
void br_stp_enable_bridge(struct net_bridge *br);
void br_stp_disable_bridge(struct net_bridge *br);
void br_stp_set_enabled(struct net_bridge *br, unsigned long val);
void br_stp_enable_port(struct net_bridge_port *p);
void br_stp_disable_port(struct net_bridge_port *p);
bool br_stp_recalculate_bridge_id(struct net_bridge *br);
void br_stp_change_bridge_id(struct net_bridge *br, const unsigned char *a);
void br_stp_set_bridge_priority(struct net_bridge *br, u16 newprio);
int br_stp_set_port_priority(struct net_bridge_port *p, unsigned long newprio);
int br_stp_set_path_cost(struct net_bridge_port *p, unsigned long path_cost);
ssize_t br_show_bridge_id(char *buf, const struct bridge_id *id);

/* br_stp_bpdu.c */
struct stp_proto;
void br_stp_rcv(const struct stp_proto *proto, struct sk_buff *skb,
		struct net_device *dev);

/* br_stp_timer.c */
void br_stp_timer_init(struct net_bridge *br);
void br_stp_port_timer_init(struct net_bridge_port *p);
unsigned long br_timer_value(const struct timer_list *timer);

/* br.c */
#if IS_ENABLED(CONFIG_ATM_LANE)
extern int (*br_fdb_test_addr_hook)(struct net_device *dev, unsigned char *addr);
#endif

/* br_netlink.c */
extern struct rtnl_link_ops br_link_ops;
int br_netlink_init(void);
void br_netlink_fini(void);
void br_ifinfo_notify(int event, struct net_bridge_port *port);
int br_setlink(struct net_device *dev, struct nlmsghdr *nlmsg);
int br_dellink(struct net_device *dev, struct nlmsghdr *nlmsg);
int br_getlink(struct sk_buff *skb, u32 pid, u32 seq, struct net_device *dev,
	       u32 filter_mask);

#ifdef CONFIG_SYSFS
/* br_sysfs_if.c */
extern const struct sysfs_ops brport_sysfs_ops;
int br_sysfs_addif(struct net_bridge_port *p);
int br_sysfs_renameif(struct net_bridge_port *p);

/* br_sysfs_br.c */
int br_sysfs_addbr(struct net_device *dev);
void br_sysfs_delbr(struct net_device *dev);

#else

static inline int br_sysfs_addif(struct net_bridge_port *p) { return 0; }
static inline int br_sysfs_renameif(struct net_bridge_port *p) { return 0; }
static inline int br_sysfs_addbr(struct net_device *dev) { return 0; }
static inline void br_sysfs_delbr(struct net_device *dev) { return; }
#endif /* CONFIG_SYSFS */

#endif
