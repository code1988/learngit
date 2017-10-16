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

#ifndef _BR_PRIVATE_STP_H
#define _BR_PRIVATE_STP_H

// BPDU报文类型
#define BPDU_TYPE_CONFIG 0      // 普通的配置报文，只能从"指定端口"发出去，并且只能从"根端口"被接收到
#define BPDU_TYPE_TCN 0x80      // 拓扑更改通知报文，只能从"根端口"发出去，并且只能从"指定端口"被接收到

/* IEEE 802.1D-1998 timer values */
// hello定时器的有效范围1s~10s
#define BR_MIN_HELLO_TIME	(1*HZ)
#define BR_MAX_HELLO_TIME	(10*HZ)

// forward delay定时器的有效范围2s~30s
#define BR_MIN_FORWARD_DELAY	(2*HZ)
#define BR_MAX_FORWARD_DELAY	(30*HZ)

// max age定时器的有效范围6s~40s
#define BR_MIN_MAX_AGE		(6*HZ)
#define BR_MAX_MAX_AGE		(40*HZ)

// 路径开销的有效范围1~65535
#define BR_MIN_PATH_COST	1
#define BR_MAX_PATH_COST	65535

// 定义了普通的配置BPDU包含的主要内容集合
struct br_config_bpdu {
	unsigned int	topology_change:1;      // 拓扑发生变化标志位，由"根桥"置1，然后传播到整个生成树网络
	unsigned int	topology_change_ack:1;  // TCN-BPDU应答标志位，"指定端口"收到TCN-BPDU时，置1作为回复
	bridge_id	root;       // "根桥ID"
	int		root_path_cost; // 到根桥的路径开销
	bridge_id	bridge_id;  // "指定桥ID"
	port_id		port_id;    // "指定端口ID"
	int		message_age;    // 消息年龄，每经过一个网桥+1
	int		max_age;        // 最大消息生存时间(只由"根桥"设置)，当message_age > max_age时，该BPDU被丢弃
	int		hello_time;     // 保活时间(只由"根桥"设置)
	int		forward_delay;  // 转发延迟(只由"根桥"设置)
};

/* called under bridge lock 
 * 判断当前端口是否是"指定端口"
 * */
static inline int br_is_designated_port(const struct net_bridge_port *p)
{
	return !memcmp(&p->designated_bridge, &p->br->bridge_id, 8) &&
		(p->designated_port == p->port_id);
}


/* br_stp.c */
void br_become_root_bridge(struct net_bridge *br);
void br_config_bpdu_generation(struct net_bridge *);
void br_configuration_update(struct net_bridge *);
void br_port_state_selection(struct net_bridge *);
void br_received_config_bpdu(struct net_bridge_port *p,
			     const struct br_config_bpdu *bpdu);
void br_received_tcn_bpdu(struct net_bridge_port *p);
void br_transmit_config(struct net_bridge_port *p);
void br_transmit_tcn(struct net_bridge *br);
void br_topology_change_detection(struct net_bridge *br);

/* br_stp_bpdu.c */
void br_send_config_bpdu(struct net_bridge_port *, struct br_config_bpdu *);
void br_send_tcn_bpdu(struct net_bridge_port *);

#endif
