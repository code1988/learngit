#ifndef _UAPI__LINUX_GENERIC_NETLINK_H
#define _UAPI__LINUX_GENERIC_NETLINK_H

#include <linux/types.h>
#include <linux/netlink.h>

#define GENL_NAMSIZ	16	/* length of family name  族名的长度上线 */

#define GENL_MIN_ID	NLMSG_MIN_TYPE  // 族ID的最小值
#define GENL_MAX_ID	1023            // 族ID的最大值

// genetlink协议的消息头结构(替换了其他netlink协议中的族头位置)
struct genlmsghdr {
	__u8	cmd;        // 自定义的消息命令
	__u8	version;    // 用于版本控制(可选)
	__u16	reserved;   // 保留字段
};

#define GENL_HDRLEN	NLMSG_ALIGN(sizeof(struct genlmsghdr))      // genlmsghdr + pad

#define GENL_ADMIN_PERM		0x01
#define GENL_CMD_CAP_DO		0x02
#define GENL_CMD_CAP_DUMP	0x04
#define GENL_CMD_CAP_HASPOL	0x08

/*
 * List of reserved static generic netlink identifiers:
 * 以下是genetlink协议定义的几个保留族ID
 */
#define GENL_ID_GENERATE	0                       // 使用本ID注册族时，内核会自动分配一个可用的族ID
#define GENL_ID_CTRL		NLMSG_MIN_TYPE          // 控制器族ID
#define GENL_ID_VFS_DQUOT	(NLMSG_MIN_TYPE + 1)
#define GENL_ID_PMCRAID		(NLMSG_MIN_TYPE + 2)

/**************************************************************************
 * Controller
 **************************************************************************/
// 定义了genetlink控制器族支持的命令
enum {
	CTRL_CMD_UNSPEC,
	CTRL_CMD_NEWFAMILY,
	CTRL_CMD_DELFAMILY,
	CTRL_CMD_GETFAMILY,     // 该命令用于用户空间从内核中获取指定族名对应的族ID
	CTRL_CMD_NEWOPS,
	CTRL_CMD_DELOPS,
	CTRL_CMD_GETOPS,
	CTRL_CMD_NEWMCAST_GRP,
	CTRL_CMD_DELMCAST_GRP,
	CTRL_CMD_GETMCAST_GRP, /* unused */
	__CTRL_CMD_MAX,
};

#define CTRL_CMD_MAX (__CTRL_CMD_MAX - 1)   // 控制器族支持的命令数量

// 定义了genetlink控制器族支持的属性
enum {
	CTRL_ATTR_UNSPEC,
	CTRL_ATTR_FAMILY_ID,
	CTRL_ATTR_FAMILY_NAME,
	CTRL_ATTR_VERSION,
	CTRL_ATTR_HDRSIZE,
	CTRL_ATTR_MAXATTR,
	CTRL_ATTR_OPS,
	CTRL_ATTR_MCAST_GROUPS,
	__CTRL_ATTR_MAX,
};

#define CTRL_ATTR_MAX (__CTRL_ATTR_MAX - 1) // 控制器族支持的属性数量

enum {
	CTRL_ATTR_OP_UNSPEC,
	CTRL_ATTR_OP_ID,
	CTRL_ATTR_OP_FLAGS,
	__CTRL_ATTR_OP_MAX,
};

#define CTRL_ATTR_OP_MAX (__CTRL_ATTR_OP_MAX - 1)

enum {
	CTRL_ATTR_MCAST_GRP_UNSPEC,
	CTRL_ATTR_MCAST_GRP_NAME,
	CTRL_ATTR_MCAST_GRP_ID,
	__CTRL_ATTR_MCAST_GRP_MAX,
};

#define CTRL_ATTR_MCAST_GRP_MAX (__CTRL_ATTR_MCAST_GRP_MAX - 1)


#endif /* _UAPI__LINUX_GENERIC_NETLINK_H */
