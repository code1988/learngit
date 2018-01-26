#ifndef __NET_GENERIC_NETLINK_H
#define __NET_GENERIC_NETLINK_H

#include <linux/genetlink.h>
#include <net/netlink.h>
#include <net/net_namespace.h>

#define GENLMSG_DEFAULT_SIZE (NLMSG_DEFAULT_SIZE - GENL_HDRLEN)

/**
 * struct genl_multicast_group - generic netlink multicast group
 * 定义了genetlink协议的内核组播组结构
 * @name: name of the multicast group, names are per-family
 */
struct genl_multicast_group {
	char			name[GENL_NAMSIZ];  // 组播组名
};

struct genl_ops;
struct genl_info;

/**
 * struct genl_family - generic netlink family
 * 定义了genetlink协议的族管理块
 *
 * @id: protocol family idenfitier
 * @hdrsize: length of user specific header in bytes
 * @name: name of family
 * @version: protocol version
 * @maxattr: maximum number of attributes supported
 * @netnsok: set to true if the family can handle network
 *	namespaces and should be presented in all of them
 * @pre_doit: called before an operation's doit callback, it may
 *	do additional, common, filtering and return an error
 * @post_doit: called after an operation's doit callback, it may
 *	undo operations done by pre_doit, for example release locks
 * @attrbuf: buffer to store parsed attributes
 * @family_list: family list
 * @mcgrps: multicast groups used by this family (private)
 * @n_mcgrps: number of multicast groups (private)
 * @mcgrp_offset: starting number of multicast group IDs in this family
 * @ops: the operations supported by this family (private)
 * @n_ops: number of operations supported by this family (private)
 */
struct genl_family {
	unsigned int		id;             // 族ID，在内核中用于唯一标识该族
	unsigned int		hdrsize;        // 用户自定义头的长度，0表示不使用
	char			name[GENL_NAMSIZ];  // 族名，主要是提供给用户空间使用，用户可以根据族名索引对应的族ID
	unsigned int		version;
	unsigned int		maxattr;        // 该族支持的最大属性数量
	bool			netnsok;            // 标识是否支持多net命名空间
	bool			parallel_ops;
	int			(*pre_doit)(const struct genl_ops *ops,
					    struct sk_buff *skb,
					    struct genl_info *info);
	void			(*post_doit)(const struct genl_ops *ops,
					     struct sk_buff *skb,
					     struct genl_info *info);
	struct nlattr **	attrbuf;	/* private  指向该族的属性缓存列表 */
	const struct genl_ops *	ops;		/* private  指向该族支持的用户命令表 */
	const struct genl_multicast_group *mcgrps; /* private 指向该族支持的内核组播组表 */
	unsigned int		n_ops;		/* private  该族支持的用户命令数量 */
	unsigned int		n_mcgrps;	/* private  该族支持的内核组播组数量 */
	unsigned int		mcgrp_offset;	/* private  该族的一段连续的组播组ID的起始号 */
	struct list_head	family_list;	/* private */
	struct module		*module;
};

/**
 * struct genl_info - receiving information
 * 定义了内核收到的genetlink信息
 *
 * @snd_seq: sending sequence number
 * @snd_portid: netlink portid of sender
 * @nlhdr: netlink message header
 * @genlhdr: generic netlink message header
 * @userhdr: user specific header
 * @attrs: netlink attributes
 * @_net: network namespace
 * @user_ptr: user pointers
 * @dst_sk: destination socket
 */
struct genl_info {
	u32			snd_seq;
	u32			snd_portid;
	struct nlmsghdr *	nlhdr;
	struct genlmsghdr *	genlhdr;
	void *			userhdr;
	struct nlattr **	attrs;
#ifdef CONFIG_NET_NS
	struct net *		_net;
#endif
	void *			user_ptr[2];
	struct sock *		dst_sk;
};

static inline struct net *genl_info_net(struct genl_info *info)
{
	return read_pnet(&info->_net);
}

static inline void genl_info_net_set(struct genl_info *info, struct net *net)
{
	write_pnet(&info->_net, net);
}

/**
 * struct genl_ops - generic netlink operations
 * 定义了genetlink协议的用户命令处理单元(显然，本结构只用于用户空间往内核空间发消息的情况)
 *
 * @cmd: command identifier
 * @internal_flags: flags used by the family
 * @flags: flags
 * @policy: attribute validation policy
 * @doit: standard command callback
 * @dumpit: callback for dumpers
 * @done: completion callback for dumps
 * @ops_list: operations list
 */
struct genl_ops {
	const struct nla_policy	*policy;        // 判断该命令处理单元属性是否有效的策略集合，NULL意味着对收到的属性不做检查
	int		       (*doit)(struct sk_buff *skb,
				       struct genl_info *info);     // 标准的命令回调函数
	int		       (*dumpit)(struct sk_buff *skb,
					 struct netlink_callback *cb);  // 需要转储时执行的回调函数
	int		       (*done)(struct netlink_callback *cb);    // 转储结束后执行的回调函数
	u8			cmd;                // 命令ID
	u8			internal_flags;     // 族内部使用的私有标识集合
	u8			flags;              // GENL_*
};

int __genl_register_family(struct genl_family *family);

static inline int genl_register_family(struct genl_family *family)
{
	family->module = THIS_MODULE;
	return __genl_register_family(family);
}

/**
 * genl_register_family_with_ops - register a generic netlink family with ops
 * 注册一个genetlink族
 *
 * @family: generic netlink family          指向要注册的族管理块
 * @ops: operations to be registered        指向同时要被注册的用户命令表
 * @n_ops: number of elements to register   表中的用户命令数量
 * @mcgrps      指向同时要被注册的内核组播组表，可以为NULL
 * @n_mcgrps    表中的组播组数量，可以为0
 *
 * Registers the specified family and operations from the specified table.
 * Only one family may be registered with the same family name or identifier.
 *
 * The family id may equal GENL_ID_GENERATE causing an unique id to
 * be automatically generated and assigned.
 * 为了避免族ID的唯一性，通常都是让内核自动分配族ID
 *
 * Either a doit or dumpit callback must be specified for every registered
 * operation or the function will fail. Only one operation structure per
 * command identifier may be registered.
 *
 * See include/net/genetlink.h for more documenation on the operations
 * structure.
 *
 * Return 0 on success or a negative error code.
 */
static inline int
_genl_register_family_with_ops_grps(struct genl_family *family,
				    const struct genl_ops *ops, size_t n_ops,
				    const struct genl_multicast_group *mcgrps,
				    size_t n_mcgrps)
{
	family->module = THIS_MODULE;
	family->ops = ops;
	family->n_ops = n_ops;
	family->mcgrps = mcgrps;
	family->n_mcgrps = n_mcgrps;
	return __genl_register_family(family);
}

// 注册一个不带组播组的genetlink族
#define genl_register_family_with_ops(family, ops)			\
	_genl_register_family_with_ops_grps((family),			\
					    (ops), ARRAY_SIZE(ops),	\
					    NULL, 0)
// 注册一个带组播组的genetlink族
#define genl_register_family_with_ops_groups(family, ops, grps)	\
	_genl_register_family_with_ops_grps((family),			\
					    (ops), ARRAY_SIZE(ops),	\
					    (grps), ARRAY_SIZE(grps))

int genl_unregister_family(struct genl_family *family);
void genl_notify(struct genl_family *family,
		 struct sk_buff *skb, struct net *net, u32 portid,
		 u32 group, struct nlmsghdr *nlh, gfp_t flags);

struct sk_buff *genlmsg_new_unicast(size_t payload, struct genl_info *info,
				    gfp_t flags);
void *genlmsg_put(struct sk_buff *skb, u32 portid, u32 seq,
		  struct genl_family *family, int flags, u8 cmd);

/**
 * genlmsg_nlhdr - Obtain netlink header from user specified header
 * @user_hdr: user header as returned from genlmsg_put()
 * @family: generic netlink family
 *
 * Returns pointer to netlink header.
 */
static inline struct nlmsghdr *genlmsg_nlhdr(void *user_hdr,
					     struct genl_family *family)
{
	return (struct nlmsghdr *)((char *)user_hdr -
				   family->hdrsize -
				   GENL_HDRLEN -
				   NLMSG_HDRLEN);
}

/**
 * genl_dump_check_consistent - check if sequence is consistent and advertise if not
 * @cb: netlink callback structure that stores the sequence number
 * @user_hdr: user header as returned from genlmsg_put()
 * @family: generic netlink family
 *
 * Cf. nl_dump_check_consistent(), this just provides a wrapper to make it
 * simpler to use with generic netlink.
 */
static inline void genl_dump_check_consistent(struct netlink_callback *cb,
					      void *user_hdr,
					      struct genl_family *family)
{
	nl_dump_check_consistent(cb, genlmsg_nlhdr(user_hdr, family));
}

/**
 * genlmsg_put_reply - Add generic netlink header to a reply message
 * @skb: socket buffer holding the message
 * @info: receiver info
 * @family: generic netlink family
 * @flags: netlink message flags
 * @cmd: generic netlink command
 *
 * Returns pointer to user specific header
 */
static inline void *genlmsg_put_reply(struct sk_buff *skb,
				      struct genl_info *info,
				      struct genl_family *family,
				      int flags, u8 cmd)
{
	return genlmsg_put(skb, info->snd_portid, info->snd_seq, family,
			   flags, cmd);
}

/**
 * genlmsg_end - Finalize a generic netlink message
 * @skb: socket buffer the message is stored in
 * @hdr: user specific header
 */
static inline int genlmsg_end(struct sk_buff *skb, void *hdr)
{
	return nlmsg_end(skb, hdr - GENL_HDRLEN - NLMSG_HDRLEN);
}

/**
 * genlmsg_cancel - Cancel construction of a generic netlink message
 * @skb: socket buffer the message is stored in
 * @hdr: generic netlink message header
 */
static inline void genlmsg_cancel(struct sk_buff *skb, void *hdr)
{
	if (hdr)
		nlmsg_cancel(skb, hdr - GENL_HDRLEN - NLMSG_HDRLEN);
}

/**
 * genlmsg_multicast_netns - multicast a netlink message to a specific netns
 * 在指定net命名空间从内核发送指定族的genetlink组播消息
 *
 * @family: the generic netlink family  目的genetlink族
 * @net: the net namespace              发组播消息的net命名空间
 * @skb: netlink message as socket buffer   承载了genetlink消息的skb
 * @portid: own netlink portid to avoid sending to yourself     发送方自己的地址，用于避免发送给自己
 * @group: offset of multicast group in groups array            组播组ID序号
 * @flags: allocation flags
 */
static inline int genlmsg_multicast_netns(struct genl_family *family,
					  struct net *net, struct sk_buff *skb,
					  u32 portid, unsigned int group, gfp_t flags)
{
	if (WARN_ON_ONCE(group >= family->n_mcgrps))
		return -EINVAL;
    // 计算得到真正的组播地址
	group = family->mcgrp_offset + group;
	return nlmsg_multicast(net->genl_sock, skb, portid, group, flags);
}

/**
 * genlmsg_multicast - multicast a netlink message to the default netns
 * @family: the generic netlink family
 * @skb: netlink message as socket buffer
 * @portid: own netlink portid to avoid sending to yourself
 * @group: offset of multicast group in groups array
 * @flags: allocation flags
 */
static inline int genlmsg_multicast(struct genl_family *family,
				    struct sk_buff *skb, u32 portid,
				    unsigned int group, gfp_t flags)
{
	return genlmsg_multicast_netns(family, &init_net, skb,
				       portid, group, flags);
}

/**
 * genlmsg_multicast_allns - multicast a netlink message to all net namespaces
 * @family: the generic netlink family
 * @skb: netlink message as socket buffer
 * @portid: own netlink portid to avoid sending to yourself
 * @group: offset of multicast group in groups array
 * @flags: allocation flags
 *
 * This function must hold the RTNL or rcu_read_lock().
 */
int genlmsg_multicast_allns(struct genl_family *family,
			    struct sk_buff *skb, u32 portid,
			    unsigned int group, gfp_t flags);

/**
 * genlmsg_unicast - unicast a netlink message
 * @skb: netlink message as socket buffer
 * @portid: netlink portid of the destination socket
 */
static inline int genlmsg_unicast(struct net *net, struct sk_buff *skb, u32 portid)
{
	return nlmsg_unicast(net->genl_sock, skb, portid);
}

/**
 * genlmsg_reply - reply to a request
 * @skb: netlink message to be sent back
 * @info: receiver information
 */
static inline int genlmsg_reply(struct sk_buff *skb, struct genl_info *info)
{
	return genlmsg_unicast(genl_info_net(info), skb, info->snd_portid);
}

/**
 * gennlmsg_data - head of message payload
 * @gnlh: genetlink message header
 */
static inline void *genlmsg_data(const struct genlmsghdr *gnlh)
{
	return ((unsigned char *) gnlh + GENL_HDRLEN);
}

/**
 * genlmsg_len - length of message payload
 * @gnlh: genetlink message header
 */
static inline int genlmsg_len(const struct genlmsghdr *gnlh)
{
	struct nlmsghdr *nlh = (struct nlmsghdr *)((unsigned char *)gnlh -
							NLMSG_HDRLEN);
	return (nlh->nlmsg_len - GENL_HDRLEN - NLMSG_HDRLEN);
}

/**
 * genlmsg_msg_size - length of genetlink message not including padding
 * @payload: length of message payload
 */
static inline int genlmsg_msg_size(int payload)
{
	return GENL_HDRLEN + payload;
}

/**
 * genlmsg_total_size - length of genetlink message including padding
 * @payload: length of message payload
 */
static inline int genlmsg_total_size(int payload)
{
	return NLMSG_ALIGN(genlmsg_msg_size(payload));
}

/**
 * genlmsg_new - Allocate a new generic netlink message
 * @payload: size of the message payload
 * @flags: the type of memory to allocate.
 */
static inline struct sk_buff *genlmsg_new(size_t payload, gfp_t flags)
{
	return nlmsg_new(genlmsg_total_size(payload), flags);
}

/**
 * genl_set_err - report error to genetlink broadcast listeners
 * @family: the generic netlink family
 * @net: the network namespace to report the error to
 * @portid: the PORTID of a process that we want to skip (if any)
 * @group: the broadcast group that will notice the error
 * 	(this is the offset of the multicast group in the groups array)
 * @code: error code, must be negative (as usual in kernelspace)
 *
 * This function returns the number of broadcast listeners that have set the
 * NETLINK_RECV_NO_ENOBUFS socket option.
 */
static inline int genl_set_err(struct genl_family *family, struct net *net,
			       u32 portid, u32 group, int code)
{
	if (WARN_ON_ONCE(group >= family->n_mcgrps))
		return -EINVAL;
	group = family->mcgrp_offset + group;
	return netlink_set_err(net->genl_sock, portid, group, code);
}

#endif	/* __NET_GENERIC_NETLINK_H */
