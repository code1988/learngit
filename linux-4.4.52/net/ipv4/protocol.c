/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		INET protocol dispatch tables.
 *		ipv4协议调度表的实现
 *
 * Authors:	Ross Biro
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *
 * Fixes:
 *		Alan Cox	: Ahah! udp icmp errors don't work because
 *				  udp_err is never called!
 *		Alan Cox	: Added new fields for init and ready for
 *				  proper fragmentation (_NO_ 4K limits!)
 *		Richard Colella	: Hang on hash collision
 *		Vince Laviano	: Modified inet_del_protocol() to correctly
 *				  maintain copy bit.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */
#include <linux/cache.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/spinlock.h>
#include <net/protocol.h>

// 这张表记录了所有已经注册的ipv4协议，索引号就是基于ipv4的协议ID(IPPROTO_*)
const struct net_protocol __rcu *inet_protos[MAX_INET_PROTOS] __read_mostly;
const struct net_offload __rcu *inet_offloads[MAX_INET_PROTOS] __read_mostly;
EXPORT_SYMBOL(inet_offloads);

/* 注册一个基于ipv4的协议，实际主要就是注册该协议报文从网络层->传输层的接口
 * @prot        指向一个基于ipv4的协议
 * @protocol    该ipv4协议ID，IPPROTO_*
 * @返回值      成功返回0
 *
 * 备注：调用本函数时需要确保指定ipv4协议尚未注册过，否则注册失败
 */
int inet_add_protocol(const struct net_protocol *prot, unsigned char protocol)
{
    // 所有待添加的ipv4协议必须都支持多网络命名空间
	if (!prot->netns_ok) {
		pr_err("Protocol %u is not namespace aware, cannot register.\n",
			protocol);
		return -EINVAL;
	}

    // 将传入的net_protocol结构地址添加到全局的inet_protos，显然如果该协议已经注册过，则返回-1
	return !cmpxchg((const struct net_protocol **)&inet_protos[protocol],
			NULL, prot) ? 0 : -1;
}
EXPORT_SYMBOL(inet_add_protocol);

int inet_add_offload(const struct net_offload *prot, unsigned char protocol)
{
	return !cmpxchg((const struct net_offload **)&inet_offloads[protocol],
			NULL, prot) ? 0 : -1;
}
EXPORT_SYMBOL(inet_add_offload);

int inet_del_protocol(const struct net_protocol *prot, unsigned char protocol)
{
	int ret;

	ret = (cmpxchg((const struct net_protocol **)&inet_protos[protocol],
		       prot, NULL) == prot) ? 0 : -1;

	synchronize_net();

	return ret;
}
EXPORT_SYMBOL(inet_del_protocol);

int inet_del_offload(const struct net_offload *prot, unsigned char protocol)
{
	int ret;

	ret = (cmpxchg((const struct net_offload **)&inet_offloads[protocol],
		       prot, NULL) == prot) ? 0 : -1;

	synchronize_net();

	return ret;
}
EXPORT_SYMBOL(inet_del_offload);
