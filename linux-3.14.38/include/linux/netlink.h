#ifndef __LINUX_NETLINK_H
#define __LINUX_NETLINK_H


#include <linux/capability.h>
#include <linux/skbuff.h>
#include <linux/export.h>
#include <net/scm.h>
#include <uapi/linux/netlink.h>

struct net;

// 获取netlink消息头
static inline struct nlmsghdr *nlmsg_hdr(const struct sk_buff *skb)
{
	return (struct nlmsghdr *)skb->data;
}

// skb中netlink消息属性枚举
enum netlink_skb_flags {
	NETLINK_SKB_MMAPED	= 0x1,	/* Packet data is mmaped */
	NETLINK_SKB_TX		= 0x2,	/* Packet was sent by userspace */
	NETLINK_SKB_DELIVERED	= 0x4,	/* Packet was delivered */
	NETLINK_SKB_DST		= 0x8,	/* Dst set in sendto or sendmsg 用于标示该netlink消息被设置了自定义的目的地址*/
};

// skb中针对netlink消息的附加信息
struct netlink_skb_parms {
	struct scm_creds	creds;		/* Skb credentials	*/
	__u32			portid;         // 记录了发送该netlink消息的源netlink套接字绑定的单播地址(注意，不是目的单播地址！)
	__u32			dst_group;      // 记录了发送该netlink消息时指定的目的组播地址
	__u32			flags;          // 就是netlink_skb_flags类型的属性集合
	struct sock		*sk;            // 记录了该netlink消息的源sock结构
};

// 将skb结构中的cb字段自定义用于保存netlink参数控制块
#define NETLINK_CB(skb)		(*(struct netlink_skb_parms*)&((skb)->cb))
#define NETLINK_CREDS(skb)	(&NETLINK_CB((skb)).creds)


extern void netlink_table_grab(void);
extern void netlink_table_ungrab(void);

#define NL_CFG_F_NONROOT_RECV	(1 << 0)    // 用来限定非超级用户是否可以绑定到多播组
#define NL_CFG_F_NONROOT_SEND	(1 << 1)    // 用来限定非超级用户是否可以发送组播(似乎也限定了非超级用户是否可以发送目的地址不是kernel的单播)

/* optional Netlink kernel configuration parameters */
// 内核创建具体协议类型的netlink套接字时传入的配置参数控制块
struct netlink_kernel_cfg {
	unsigned int	groups; // 该协议类型支持的最大多播组数量
	unsigned int	flags;  // 用来设置NL_CFG_F_NONROOT_SEND/NL_CFG_F_NONROOT_RECV这两个标志
	void		(*input)(struct sk_buff *skb);  // 用来配置协议类型私有的消息接收函数，用户空间发送该协议类型的netlink消息给内核后，就会调用本函数
	struct mutex	*cb_mutex;      // 用来配置协议私有的互斥锁
	void		(*bind)(int group); // 用来配置协议类型私有的bind回调函数
	bool		(*compare)(struct net *net, struct sock *sk);   // 用来配置协议类型私有的compare回调函数
};

extern struct sock *__netlink_kernel_create(struct net *net, int unit,
					    struct module *module,
					    struct netlink_kernel_cfg *cfg);
/* 创建属于内核的具体协议(如NETLINK_ROUTE)的netlink套接字,成功返回创建的sock结构
 *
 * 备注：只要是跟用户态交互的netlink协议，就需要在初始化时调用本函数，以创建一个属于内核的netlink套接字
 *      之后，只要用户空间发送了一个该协议的netlink消息到内核，通过本函数注册的相应协议的input函数就会被调用
 */
static inline struct sock *
netlink_kernel_create(struct net *net, int unit, struct netlink_kernel_cfg *cfg)
{
	return __netlink_kernel_create(net, unit, THIS_MODULE, cfg);
}

extern void netlink_kernel_release(struct sock *sk);
extern int __netlink_change_ngroups(struct sock *sk, unsigned int groups);
extern int netlink_change_ngroups(struct sock *sk, unsigned int groups);
extern void __netlink_clear_multicast_users(struct sock *sk, unsigned int group);
extern void netlink_ack(struct sk_buff *in_skb, struct nlmsghdr *nlh, int err);
extern int netlink_has_listeners(struct sock *sk, unsigned int group);
extern struct sk_buff *netlink_alloc_skb(struct sock *ssk, unsigned int size,
					 u32 dst_portid, gfp_t gfp_mask);
extern int netlink_unicast(struct sock *ssk, struct sk_buff *skb, __u32 portid, int nonblock);
extern int netlink_broadcast(struct sock *ssk, struct sk_buff *skb, __u32 portid,
			     __u32 group, gfp_t allocation);
extern int netlink_broadcast_filtered(struct sock *ssk, struct sk_buff *skb,
	__u32 portid, __u32 group, gfp_t allocation,
	int (*filter)(struct sock *dsk, struct sk_buff *skb, void *data),
	void *filter_data);
extern int netlink_set_err(struct sock *ssk, __u32 portid, __u32 group, int code);
extern int netlink_register_notifier(struct notifier_block *nb);
extern int netlink_unregister_notifier(struct notifier_block *nb);

/* finegrained unicast helpers: */
struct sock *netlink_getsockbyfilp(struct file *filp);
int netlink_attachskb(struct sock *sk, struct sk_buff *skb,
		      long *timeo, struct sock *ssk);
void netlink_detachskb(struct sock *sk, struct sk_buff *skb);
int netlink_sendskb(struct sock *sk, struct sk_buff *skb);

static inline struct sk_buff *
netlink_skb_clone(struct sk_buff *skb, gfp_t gfp_mask)
{
	struct sk_buff *nskb;

	nskb = skb_clone(skb, gfp_mask);
	if (!nskb)
		return NULL;

	/* This is a large skb, set destructor callback to release head */
	if (is_vmalloc_addr(skb->head))
		nskb->destructor = skb->destructor;

	return nskb;
}

/*
 *	skb should fit one page. This choice is good for headerless malloc.
 *	But we should limit to 8K so that userspace does not have to
 *	use enormous buffer sizes on recvmsg() calls just to avoid
 *	MSG_TRUNC when PAGE_SIZE is very large.
 *	skb的数据缓冲区长度上限的合理值跟page大小有关
 */
#if PAGE_SIZE < 8192UL
#define NLMSG_GOODSIZE	SKB_WITH_OVERHEAD(PAGE_SIZE)
#else
#define NLMSG_GOODSIZE	SKB_WITH_OVERHEAD(8192UL)
#endif

#define NLMSG_DEFAULT_SIZE (NLMSG_GOODSIZE - NLMSG_HDRLEN)

// 定义了当前某条netlink消息期间有效的操作集合
struct netlink_callback {
	struct sk_buff		*skb;                           // 指向当前收到的正在处理的skb
	const struct nlmsghdr	*nlh;                       // 指向一条当前正在处理的netlink消息
	int			(*dump)(struct sk_buff * skb,
					struct netlink_callback *cb);       // 指向dump回调
	int			(*done)(struct netlink_callback *cb);
	void			*data;
	/* the module that dump function belong to */
	struct module		*module;
	u16			family;
	u16			min_dump_alloc;                         // 记录了需要dump的数据空间大小
	unsigned int		prev_seq, seq;
	long			args[6];
};

struct netlink_notify {
	struct net *net;
	int portid;
	int protocol;
};

struct nlmsghdr *
__nlmsg_put(struct sk_buff *skb, u32 portid, u32 seq, int type, int len, int flags);

// netlink用于dump操作的控制块
struct netlink_dump_control {
	int (*dump)(struct sk_buff *skb, struct netlink_callback *);    // 一般就是指向dumpit回调函数
	int (*done)(struct netlink_callback *);
	void *data;
	struct module *module;
	u16 min_dump_alloc;         // 用于dump的数据空间大小
};

extern int __netlink_dump_start(struct sock *ssk, struct sk_buff *skb,
				const struct nlmsghdr *nlh,
				struct netlink_dump_control *control);
/* 执行dump操作(封装)
 *
 * @ssk     - 这个sock结构也就是对应了一个netlink套接字
 * @skb     - 存储了netlink消息的skb
 * @nlh     - 一条完整的netlink消息
 * @control - 用于dump操作的控制块，里面主要记录了dump回调函数和dump的数据空间大小
 */
static inline int netlink_dump_start(struct sock *ssk, struct sk_buff *skb,
				     const struct nlmsghdr *nlh,
				     struct netlink_dump_control *control)
{
	if (!control->module)
		control->module = THIS_MODULE;

	return __netlink_dump_start(ssk, skb, nlh, control);
}

struct netlink_tap {
	struct net_device *dev;
	struct module *module;
	struct list_head list;
};

extern int netlink_add_tap(struct netlink_tap *nt);
extern int netlink_remove_tap(struct netlink_tap *nt);

bool __netlink_ns_capable(const struct netlink_skb_parms *nsp,
			  struct user_namespace *ns, int cap);
bool netlink_ns_capable(const struct sk_buff *skb,
			struct user_namespace *ns, int cap);
bool netlink_capable(const struct sk_buff *skb, int cap);
bool netlink_net_capable(const struct sk_buff *skb, int cap);

#endif	/* __LINUX_NETLINK_H */
