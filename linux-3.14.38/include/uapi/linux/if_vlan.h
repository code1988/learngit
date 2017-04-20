/*
 * VLAN		An implementation of 802.1Q VLAN tagging.
 *
 * Authors:	Ben Greear <greearb@candelatech.com>
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 */

#ifndef _UAPI_LINUX_IF_VLAN_H_
#define _UAPI_LINUX_IF_VLAN_H_


/* VLAN IOCTLs are found in sockios.h */

/* Passed in vlan_ioctl_args structure to determine behaviour. 
 * ioctl中已经定义了的vlan命令枚举
 * */
enum vlan_ioctl_cmds {
	ADD_VLAN_CMD,                   // 添加一个vlan
	DEL_VLAN_CMD,                   // 删除一个vlan
	SET_VLAN_INGRESS_PRIORITY_CMD,
	SET_VLAN_EGRESS_PRIORITY_CMD,
	GET_VLAN_INGRESS_PRIORITY_CMD,
	GET_VLAN_EGRESS_PRIORITY_CMD,
	SET_VLAN_NAME_TYPE_CMD,         // 设置vlan名字的显示风格，设置前的已有的vlan显示风格不变，设置时不需要指定device1
	SET_VLAN_FLAG_CMD,
	GET_VLAN_REALDEV_NAME_CMD, /* If this works, you know it's a VLAN device, btw */
	GET_VLAN_VID_CMD /* Get the VID of this VLAN (specified by name) */
};

// vlan私有空间vlan_dev_priv->flags标志枚举
enum vlan_flags {
	VLAN_FLAG_REORDER_HDR	= 0x1,      // 决定了报文发送时是否需要打上vlan tag / 报文接收时是否需要脱掉vlan tag(创建vlan设备时缺省设置了该标志)
	VLAN_FLAG_GVRP		= 0x2,
	VLAN_FLAG_LOOSE_BINDING	= 0x4,
	VLAN_FLAG_MVRP		= 0x8,
};

// vlan名字的显示风格
enum vlan_name_types {
	VLAN_NAME_TYPE_PLUS_VID, /* Name will look like:  vlan0005 */
	VLAN_NAME_TYPE_RAW_PLUS_VID, /* name will look like:  eth1.0005 */
	VLAN_NAME_TYPE_PLUS_VID_NO_PAD, /* Name will look like:  vlan5 */
	VLAN_NAME_TYPE_RAW_PLUS_VID_NO_PAD, /* Name will look like:  eth0.5 */
	VLAN_NAME_TYPE_HIGHEST
};

// ioctl中vlan传入的参数结构
struct vlan_ioctl_args {
	int cmd; /* Should be one of the vlan_ioctl_cmds enum above. */
	char device1[24];       // 用户传入的设备名，用于内核查找实际对应的设备
                            // 调用ADD_VLAN_CMD/SET_VLAN_FLAG_CMD时，该参数传入的是vlan的宿主设备名
                            // 调用其他命令时，该参数传入的是vlan设备名
        union {
		char device2[24];   // 用于返回vlan设备名
		int VID;            // 用于传入vlan ID
		unsigned int skb_priority;
		unsigned int name_type;
		unsigned int bind_type;
		unsigned int flag; /* Matches vlan_dev_priv flags */
        } u;

	short vlan_qos;   
};

#endif /* _UAPI_LINUX_IF_VLAN_H_ */
