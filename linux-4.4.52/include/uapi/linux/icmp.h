/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		Definitions for the ICMP protocol.
 *
 * Version:	@(#)icmp.h	1.0.3	04/28/93
 *
 * Author:	Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */
#ifndef _UAPI_LINUX_ICMP_H
#define _UAPI_LINUX_ICMP_H

#include <linux/types.h>

// 以下是icmpv4报文中的类型字段定义
#define ICMP_ECHOREPLY		0	/* Echo Reply	 ping回复icmp查询消息		*/
#define ICMP_DEST_UNREACH	3	/* Destination Unreachable  目的不可达类icmp错误消息	*/
#define ICMP_SOURCE_QUENCH	4	/* Source Quench 信源抑制icmp错误消息(已作废)		*/
#define ICMP_REDIRECT		5	/* Redirect (change route)	重定向类icmp错误消息(这类消息只有网关允许发送) */
#define ICMP_ECHO		8	/* Echo Request		 ping请求icmp查询消息	*/
#define ICMP_TIME_EXCEEDED	11	/* Time Exceeded  超时类icmp错误消息		*/
#define ICMP_PARAMETERPROB	12	/* Parameter Problem		*/
#define ICMP_TIMESTAMP		13	/* Timestamp Request 时间戳请求icmp查询消息		*/
#define ICMP_TIMESTAMPREPLY	14	/* Timestamp Reply	 时间戳回复icmp查询消息	*/
#define ICMP_INFO_REQUEST	15	/* Information Request		*/
#define ICMP_INFO_REPLY		16	/* Information Reply		*/
#define ICMP_ADDRESS		17	/* Address Mask Request		*/
#define ICMP_ADDRESSREPLY	18	/* Address Mask Reply		*/
#define NR_ICMP_TYPES		18  // icmpv4消息总共支持18种类型


/* Codes for UNREACH. 目的不可达类消息的细分类型 */
#define ICMP_NET_UNREACH	0	/* Network Unreachable		*/
#define ICMP_HOST_UNREACH	1	/* Host Unreachable		*/
#define ICMP_PROT_UNREACH	2	/* Protocol Unreachable	
                                   协议不可达(ipv4头中协议字段的协议号不支持)	*/
#define ICMP_PORT_UNREACH	3	/* Port Unreachable		
                                   端口不可达(收到的udp报文找不到匹配的套接字，但报文校验和正确) */
#define ICMP_FRAG_NEEDED	4	/* Fragmentation Needed/DF set	
                                   需要分片(转发ipv4报文时，如果其长度超过了出口网络设备的mtu,
                                   且该ipv4头中没有设置DF位,且系统没有设置忽略DF位的标志) */
#define ICMP_SR_FAILED		5	/* Source Route failed		*/
#define ICMP_NET_UNKNOWN	6
#define ICMP_HOST_UNKNOWN	7
#define ICMP_HOST_ISOLATED	8
#define ICMP_NET_ANO		9
#define ICMP_HOST_ANO		10  /* 目标主机被管理员禁止访问(比如目标主机被设置了一条规则：
                                   iptables -A INPUT -j REJECT --reject-with icmp-host-prohibited ) */
#define ICMP_NET_UNR_TOS	11
#define ICMP_HOST_UNR_TOS	12
#define ICMP_PKT_FILTERED	13	/* Packet filtered */
#define ICMP_PREC_VIOLATION	14	/* Precedence violation */
#define ICMP_PREC_CUTOFF	15	/* Precedence cut off */
#define NR_ICMP_UNREACH		15	/* instead of hardcoding immediate value */

/* Codes for REDIRECT. 重定向类消息的细分类型 */
#define ICMP_REDIR_NET		0	/* Redirect Net			*/
#define ICMP_REDIR_HOST		1	/* Redirect Host		*/
#define ICMP_REDIR_NETTOS	2	/* Redirect Net for TOS		*/
#define ICMP_REDIR_HOSTTOS	3	/* Redirect Host for TOS	*/

/* Codes for TIME_EXCEEDED. 超时类消息的细分类型 */
#define ICMP_EXC_TTL		0	/* TTL count exceeded  TTL耗尽		*/
#define ICMP_EXC_FRAGTIME	1	/* Fragment Reass time exceeded 分片回收(重组)超时	*/

// icmpv4报文头结构
struct icmphdr {
  __u8		type;
  __u8		code;
  __sum16	checksum;
  // 可变部分内容取决域type和code
  union {
	struct {
		__be16	id;
		__be16	sequence;
	} echo;
	__be32	gateway;
	struct {
		__be16	__unused;
		__be16	mtu;
	} frag;
  } un;
};


/*
 *	constants for (set|get)sockopt
 */

#define ICMP_FILTER			1

struct icmp_filter {
	__u32		data;
};


#endif /* _UAPI_LINUX_ICMP_H */
