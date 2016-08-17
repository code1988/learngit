/************************************************************************ 
 * RSTP library - Rapid Spanning Tree (802.1t, 802.1w) 
 * Copyright (C) 2001-2003 Optical Access 
 * Author: Alex Rozin 
 * 
 * This file is part of RSTP library. 
 * 
 * RSTP library is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published by the 
 * Free Software Foundation; version 2.1 
 * 
 * RSTP library is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser 
 * General Public License for more details. 
 * 
 * You should have received a copy of the GNU Lesser General Public License 
 * along with RSTP library; see the file COPYING.  If not, write to the Free 
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
 * 02111-1307, USA. 
 **********************************************************************/

/* BPDU formats: 9.1 - 9.3, 17.28 */
 
#ifndef _STP_BPDU_H__
#define _STP_BPDU_H__

#define MIN_BPDU                7
#define BPDU_L_SAP              0x42
#define LLC_UI                  0x03
#define BPDU_PROTOCOL_ID        0x0000
#define BPDU_VERSION_ID         0x00
#define BPDU_VERSION_RAPID_ID   0x02

// bpdu类型
#define BPDU_TOPO_CHANGE_TYPE   0x80    // TCN bpdu (STP 专属)
#define BPDU_CONFIG_TYPE        0x00    // config bpdu (STP 专属)
#define BPDU_RSTP               0x02    // rstp bpdu (RSTP 专属)

#define TOLPLOGY_CHANGE_BIT     0x01    // 拓扑变化标志位
#define PROPOSAL_BIT            0x02
#define PORT_ROLE_OFFS          2   /* 0x04 & 0x08 */
#define PORT_ROLE_MASK          (0x03 << PORT_ROLE_OFFS)
#define LEARN_BIT               0x10
#define FORWARD_BIT             0x20
#define AGREEMENT_BIT           0x40
#define TOLPLOGY_CHANGE_ACK_BIT 0x80    // 拓扑变化ack标志位

// 端口角色
#define RSTP_PORT_ROLE_UNKN     0x00    // 未知
#define RSTP_PORT_ROLE_ALTBACK  0x01    // 备份端口
#define RSTP_PORT_ROLE_ROOT     0x02    // 根端口
#define RSTP_PORT_ROLE_DESGN    0x03    // 指定端口

typedef struct mac_header_t {
  unsigned char dst_mac[6];
  unsigned char src_mac[6];
} MAC_HEADER_T;

// 以太网头
typedef struct eth_header_t {
  unsigned char len8023[2];
  unsigned char dsap;
  unsigned char ssap;
  unsigned char llc;
} ETH_HEADER_T;

// bpdu头
typedef struct bpdu_header_t {
  unsigned char protocol[2];    // 固定值0x0000
  unsigned char version;        // bpdu版本（rstp/stp）
  unsigned char bpdu_type;      // bpdu类型
} BPDU_HEADER_T;

// bpdu主体
typedef struct bpdu_body_t {
    /* 标记字段
     * bit8: topology-change-ack 指定端口收到TCN bpdu时，该位置1
     * bit7: agreement           根端口收到proposal时，该位置1
     * bit6: forwarding          端口进入forwarding状态时，该位置1
     * bit5: learning            端口可以进行地址学习时，该位置1
     * bit4-bit3: port-role      封装发送此bpdu的端口角色
     * bit2: proposal            指定端口要快速切换到forwarding状态时，该位置1
     * bit1: topology-change     拓扑发生变化时，该位置1
     */
  unsigned char flags;              
  unsigned char root_id[8];         // 也就是桥ID，由2字节优先级和6字节mac组成
  unsigned char root_path_cost[4];  // 路径成本
  unsigned char bridge_id[8];       // 同root id
  unsigned char port_id[2];         // 端口ID，由优先级和端口编号组成
  unsigned char message_age[2];     // 消息年龄，每经过一个网桥+1，message_age大于max_age时消息被丢弃，所以网络上的网桥数量受到限制
  unsigned char max_age[2];         // 消息最大年龄，统一由根节点配置
  unsigned char hello_time[2];      // ，统一由根节点配置
  unsigned char forward_delay[2];   // 转发延迟，统一由根节点配置
} BPDU_BODY_T;

// bpdu报文
typedef struct stp_bpdu_t {
  ETH_HEADER_T  eth;        // 以太网帧头
  BPDU_HEADER_T hdr;        // bpdu头
  BPDU_BODY_T   body;       // bpdu主体
  unsigned char ver_1_len[2];   // stp没有此项，为rstp特有
} BPDU_T;

#endif /* _STP_BPDU_H__ */

