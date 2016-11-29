#ifndef _UAPI__LINUX_NETLINK_H
#define _UAPI__LINUX_NETLINK_H

#include <linux/kernel.h>
#include <linux/socket.h> /* for __kernel_sa_family_t */
#include <linux/types.h>

// netlink协议，除了以下预定义的之外，还可以自定义，目前上限MAX_LINKS个
#define NETLINK_ROUTE		0	/* Routing/device hook				*/
#define NETLINK_UNUSED		1	/* Unused number				*/
#define NETLINK_USERSOCK	2	/* Reserved for user mode socket protocols 	*/
#define NETLINK_FIREWALL	3	/* Unused number, formerly ip_queue		*/
#define NETLINK_SOCK_DIAG	4	/* socket monitoring				*/
#define NETLINK_NFLOG		5	/* netfilter/iptables ULOG */
#define NETLINK_XFRM		6	/* ipsec */
#define NETLINK_SELINUX		7	/* SELinux event notifications */
#define NETLINK_ISCSI		8	/* Open-iSCSI */
#define NETLINK_AUDIT		9	/* auditing */
#define NETLINK_FIB_LOOKUP	10	
#define NETLINK_CONNECTOR	11
#define NETLINK_NETFILTER	12	/* netfilter subsystem */
#define NETLINK_IP6_FW		13
#define NETLINK_DNRTMSG		14	/* DECnet routing messages */
#define NETLINK_KOBJECT_UEVENT	15	/* Kernel messages to userspace */
#define NETLINK_GENERIC		16
/* leave room for NETLINK_DM (DM Events) */
#define NETLINK_SCSITRANSPORT	18	/* SCSI Transports */
#define NETLINK_ECRYPTFS	19
#define NETLINK_RDMA		20
#define NETLINK_CRYPTO		21	/* Crypto layer */

#define NETLINK_INET_DIAG	NETLINK_SOCK_DIAG

#define MAX_LINKS 32		

// netlink地址
struct sockaddr_nl {
	__kernel_sa_family_t	nl_family;	/* AF_NETLINK	*/
	unsigned short	nl_pad;		/* zero		*/
	__u32		nl_pid;		/* port ID	*/
       	__u32		nl_groups;	/* multicast groups mask */
};

// netlink消息头
struct nlmsghdr {
	__u32		nlmsg_len;	// netlink消息实际长（header + payload）
	__u16		nlmsg_type;	// netlink消息类型
	__u16		nlmsg_flags;// 附加的标志位,定义见下面的 NLM_F_*
	__u32		nlmsg_seq;	// 序号（用于追踪）
	__u32		nlmsg_pid;	// 进程ID（用于追踪）
};

/* Flags values */
#define NLM_F_REQUEST		1	// 表示消息是一个请求。所有用户首先发起的消息都要设置该标志，可以和GET request和NEW request系列标志组合
#define NLM_F_MULTI		2	/* Multipart message, terminated by NLMSG_DONE */
#define NLM_F_ACK		4	/* Reply with ack, with zero or error code */
#define NLM_F_ECHO		8	/* Echo this request 		*/
#define NLM_F_DUMP_INTR		16	/* Dump was inconsistent due to sequence change */

/* Modifiers to GET request */
#define NLM_F_ROOT	0x100	    // 表示被请求的数据应当整体返回用户应用，而不是一条一条返回，有该标志的request通常导致响应的消息设置NLM_F_MULTI标志
#define NLM_F_MATCH	0x200	    // 表示被请求的数据将会通过指定的过滤器来匹配（通常是用户设置了过滤器时使用该标记）
#define NLM_F_ATOMIC	0x400	/* atomic GET		*/
#define NLM_F_DUMP	(NLM_F_ROOT|NLM_F_MATCH)            // NLM_F_ROOT和NLM_F_MATCH的合集

/* Modifiers to NEW request */
#define NLM_F_REPLACE	0x100	/* Override existing		*/
#define NLM_F_EXCL	0x200	/* Do not touch, if it exists	*/
#define NLM_F_CREATE	0x400	/* Create, if it does not exist	*/
#define NLM_F_APPEND	0x800	/* Add to end of list		*/

/*
   4.4BSD ADD		NLM_F_CREATE|NLM_F_EXCL
   4.4BSD CHANGE	NLM_F_REPLACE

   True CHANGE		NLM_F_CREATE|NLM_F_REPLACE
   Append		NLM_F_CREATE
   Check		NLM_F_EXCL
 */

// netlink消息长度需要4字节对齐
#define NLMSG_ALIGNTO	4U
#define NLMSG_ALIGN(len) ( ((len)+NLMSG_ALIGNTO-1) & ~(NLMSG_ALIGNTO-1) )
#define NLMSG_HDRLEN	 ((int) NLMSG_ALIGN(sizeof(struct nlmsghdr)))       // netlink消息头对齐后长度
#define NLMSG_LENGTH(len) ((len) + NLMSG_HDRLEN)                            // netlink消息实际长（不含payload部分的填充）
#define NLMSG_SPACE(len) NLMSG_ALIGN(NLMSG_LENGTH(len))                     // netlink消息头 + 填充 + len + 填充
#define NLMSG_DATA(nlh)  ((void*)(((char*)nlh) + NLMSG_LENGTH(0)))          // netlink消息payload首地址
// 下一条netlink消息
#define NLMSG_NEXT(nlh,len)	 ((len) -= NLMSG_ALIGN((nlh)->nlmsg_len), \
				  (struct nlmsghdr*)(((char*)(nlh)) + NLMSG_ALIGN((nlh)->nlmsg_len)))
// netlink消息合法性检测
#define NLMSG_OK(nlh,len) ((len) >= (int)sizeof(struct nlmsghdr) && \
			   (nlh)->nlmsg_len >= sizeof(struct nlmsghdr) && \
			   (nlh)->nlmsg_len <= (len))
#define NLMSG_PAYLOAD(nlh,len) ((nlh)->nlmsg_len - NLMSG_SPACE((len)))

#define NLMSG_NOOP		0x1	/* Nothing.		*/
#define NLMSG_ERROR		0x2	/* Error		*/
#define NLMSG_DONE		0x3	/* End of a dump	*/
#define NLMSG_OVERRUN		0x4	/* Data lost		*/

#define NLMSG_MIN_TYPE		0x10	/* < 0x10: reserved control messages */

struct nlmsgerr {
	int		error;
	struct nlmsghdr msg;
};

#define NETLINK_ADD_MEMBERSHIP	1
#define NETLINK_DROP_MEMBERSHIP	2
#define NETLINK_PKTINFO		3
#define NETLINK_BROADCAST_ERROR	4
#define NETLINK_NO_ENOBUFS	5
#define NETLINK_RX_RING		6
#define NETLINK_TX_RING		7

struct nl_pktinfo {
	__u32	group;
};

struct nl_mmap_req {
	unsigned int	nm_block_size;
	unsigned int	nm_block_nr;
	unsigned int	nm_frame_size;
	unsigned int	nm_frame_nr;
};

struct nl_mmap_hdr {
	unsigned int	nm_status;
	unsigned int	nm_len;
	__u32		nm_group;
	/* credentials */
	__u32		nm_pid;
	__u32		nm_uid;
	__u32		nm_gid;
};

enum nl_mmap_status {
	NL_MMAP_STATUS_UNUSED,
	NL_MMAP_STATUS_RESERVED,
	NL_MMAP_STATUS_VALID,
	NL_MMAP_STATUS_COPY,
	NL_MMAP_STATUS_SKIP,
};

#define NL_MMAP_MSG_ALIGNMENT		NLMSG_ALIGNTO
#define NL_MMAP_MSG_ALIGN(sz)		__ALIGN_KERNEL(sz, NL_MMAP_MSG_ALIGNMENT)
#define NL_MMAP_HDRLEN			NL_MMAP_MSG_ALIGN(sizeof(struct nl_mmap_hdr))

#define NET_MAJOR 36		/* Major 36 is reserved for networking 						*/

enum {
	NETLINK_UNCONNECTED = 0,
	NETLINK_CONNECTED,
};

/*
 *  <------- NLA_HDRLEN ------> <-- NLA_ALIGN(payload)-->
 * +---------------------+- - -+- - - - - - - - - -+- - -+
 * |        Header       | Pad |     Payload       | Pad |
 * |   (struct nlattr)   | ing |                   | ing |
 * +---------------------+- - -+- - - - - - - - - -+- - -+
 *  <-------------- nlattr->nla_len -------------->
 */
// 属性头
struct nlattr {
	__u16           nla_len;    // 属性实际长度，不包含为尾部padding
	__u16           nla_type;   // 属性类型
};

/*
 * nla_type (16 bits)
 * +---+---+-------------------------------+
 * | N | O | Attribute Type                |
 * +---+---+-------------------------------+
 * N := Carries nested attributes
 * O := Payload stored in network byte order
 *
 * Note: The N and O flag are mutually exclusive.
 */
#define NLA_F_NESTED		(1 << 15)
#define NLA_F_NET_BYTEORDER	(1 << 14)
#define NLA_TYPE_MASK		~(NLA_F_NESTED | NLA_F_NET_BYTEORDER)

// netlink属性长度也是4字节对齐
#define NLA_ALIGNTO		4
#define NLA_ALIGN(len)		(((len) + NLA_ALIGNTO - 1) & ~(NLA_ALIGNTO - 1))
#define NLA_HDRLEN		((int) NLA_ALIGN(sizeof(struct nlattr)))


#endif /* _UAPI__LINUX_NETLINK_H */
