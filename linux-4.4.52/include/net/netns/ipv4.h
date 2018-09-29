/*
 * ipv4 in net namespaces
 */

#ifndef __NETNS_IPV4_H__
#define __NETNS_IPV4_H__

#include <linux/uidgid.h>
#include <net/inet_frag.h>
#include <linux/rcupdate.h>

struct tcpm_hash_bucket;
struct ctl_table_header;
struct ipv4_devconf;
struct fib_rules_ops;
struct hlist_head;
struct fib_table;
struct sock;
struct local_ports {
	seqlock_t	lock;
	int		range[2];
	bool		warned;
};

// ping套接字权限配置集结构
struct ping_group_range {
	seqlock_t	lock;
	kgid_t		range[2];
};

/* 网络命名空间层面的ipv4配置信息集合结构
 * 备注：这个结构中大部分变量都对应了procfs条目
 */
struct netns_ipv4 {
#ifdef CONFIG_SYSCTL
	struct ctl_table_header	*forw_hdr;
	struct ctl_table_header	*frags_hdr;
	struct ctl_table_header	*ipv4_hdr;
	struct ctl_table_header *route_hdr;
	struct ctl_table_header *xfrm4_hdr;
#endif
	struct ipv4_devconf	*devconf_all;       // 网络命名空间层面的ipv4相关配置参数
	struct ipv4_devconf	*devconf_dflt;
#ifdef CONFIG_IP_MULTIPLE_TABLES
	struct fib_rules_ops	*rules_ops;
	bool			fib_has_custom_rules;
	struct fib_table __rcu	*fib_local;
	struct fib_table __rcu	*fib_main;
	struct fib_table __rcu	*fib_default;
#endif
#ifdef CONFIG_IP_ROUTE_CLASSID
	int			fib_num_tclassid_users;
#endif
	struct hlist_head	*fib_table_hash;
	bool			fib_offload_disabled;
	struct sock		*fibnl;

	struct sock  * __percpu	*icmp_sk;   // icmp内核套接字(在smp架构中每个cpu都对应了一个icmp内核套接字)
	struct sock		*mc_autojoin_sk;

	struct inet_peer_base	*peers;
	struct sock  * __percpu	*tcp_sk;
	struct netns_frags	frags;
#ifdef CONFIG_NETFILTER
	struct xt_table		*iptable_filter;
	struct xt_table		*iptable_mangle;
	struct xt_table		*iptable_raw;
	struct xt_table		*arptable_filter;
#ifdef CONFIG_SECURITY
	struct xt_table		*iptable_security;
#endif
	struct xt_table		*nat_table;
#endif

	int sysctl_icmp_echo_ignore_all;        // 标识是否关闭对ping请求报文的应答功能(缺省为0)
	int sysctl_icmp_echo_ignore_broadcasts; // 标识是否关闭对ip广播/组播ping/时间戳请求报文的应答功能(缺省为1)
	int sysctl_icmp_ignore_bogus_error_responses;   // 标识是否关闭告警信息"<IPv4Addr>sent an invalid ICMP type. . ."
	int sysctl_icmp_ratelimit;              /* 对于类型与icmp速率掩码匹配的的icmp报文，将其最大速率限制为该值
                                               该值为0表示禁用速率限制(缺省为1*HZ) */
	int sysctl_icmp_ratemask;               /* 指定要进行速率限制的icmp消息类型集合，掩码格式，每位代表一种icmp消息类型
                                               (缺省为0x1818,即对应了4种icmp消息类型：
                                               ICMP_PARAMETERPROB、ICMP_TIME_EXCEEDED、ICMP_SOURCE_QUENCH、ICMP_DEST_UNREACH) */
	int sysctl_icmp_errors_use_inbound_ifaddr;  /* 0表示发送icmp错误消息时将使用出口网络设备的主地址
                                                   非0表示发送icmp错误消息时将使用收到的引起该icmp错误的数据包的入口网络设备的主地址
                                                   (缺省为0) */

	struct local_ports ip_local_ports;

	int sysctl_tcp_ecn;
	int sysctl_tcp_ecn_fallback;

	int sysctl_ip_no_pmtu_disc;     // 标识是否关闭pmtu发现功能(缺省为0)
	int sysctl_ip_fwd_use_pmtu;
	int sysctl_ip_nonlocal_bind;

	int sysctl_fwmark_reflect;
	int sysctl_tcp_fwmark_accept;
	int sysctl_tcp_mtu_probing;
	int sysctl_tcp_base_mss;
	int sysctl_tcp_probe_threshold;
	u32 sysctl_tcp_probe_interval;

	struct ping_group_range ping_group_range;   /* ping套接字的权限配置集
                                                   (缺省为"1 0"，意味着任何用户（包括root用户）都不允许创建ping套接字) */

	atomic_t dev_addr_genid;

#ifdef CONFIG_SYSCTL
	unsigned long *sysctl_local_reserved_ports;
#endif

#ifdef CONFIG_IP_MROUTE
#ifndef CONFIG_IP_MROUTE_MULTIPLE_TABLES
	struct mr_table		*mrt;
#else
	struct list_head	mr_tables;
	struct fib_rules_ops	*mr_rules_ops;
#endif
#endif
	atomic_t	rt_genid;
};
#endif
