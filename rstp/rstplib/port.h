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

/* STP PORT instance : 17.18, 17.15 */
 
#ifndef _STP_PORT_H__
#define _STP_PORT_H__

#include "statmch.h"

#define TIMERS_NUMBER   9
typedef unsigned int    PORT_TIMER_T;   // 定时器计数器

typedef enum {
  Mine,
  Aged,
  Received,
  Disabled
} INFO_IS_T;

// 接收到bpdu的消息类型
typedef enum {
  SuperiorDesignateMsg, // 更优的bpdu
  RepeatedDesignateMsg, // 重复的bpdu
  ConfirmedRootMsg,
  OtherMsg              // 其他消息(包括拓扑变化消息)
} RCVD_MSG_T;

// 端口角色
typedef enum {
  DisabledPort = 0,
  AlternatePort,    // 可选端口，提供了到达根网桥的另一条可选路径，是根端口的备份
  BackupPort,       // 备份端口，备份端口是指定端口的备份
  RootPort,         // 根端口，提供了到达根网桥的一条最优路径
  DesignatedPort,   // 指定端口，将LAN连到指定网桥的端口都是该LAN的指定端口
  NonStpPort
} PORT_ROLE_T;

// 端口控制块
typedef struct port_t {
  struct port_t*     next;

  /* per Port state machines */
  // 属于每个端口的状态机
  STATE_MACH_T*     info;      /* 17.21 */
  STATE_MACH_T*     roletrns;  /* 17.23 */
  STATE_MACH_T*     sttrans;   /* 17.24 */
  STATE_MACH_T*     topoch;    /* 17.25 */
  STATE_MACH_T*     migrate;   /* 17.26 */
  STATE_MACH_T*     transmit;  /* 17.26 */
  STATE_MACH_T*     p2p;       /* 6.4.3, 6.5.1 */
  STATE_MACH_T*     edge;      /*  */
  STATE_MACH_T*     pcost;     /*  */

  STATE_MACH_T*     machines;   // 属于端口的状态机链表 /* list of machines */

  struct stpm_t*    owner; /* Bridge, that this port belongs to */
  
  /* per port Timers */
  PORT_TIMER_T      fdWhile;      /* 17.15.1 */
  PORT_TIMER_T      helloWhen;    /* 17.15.2 */
  PORT_TIMER_T      mdelayWhile;  /* 17.15.3 */
  PORT_TIMER_T      rbWhile;      /* 17.15.4 */
  PORT_TIMER_T      rcvdInfoWhile;/* 17.15.5 */ // bpdu报文的剩余生存时间,用于避免旧config bpdu报文在冗余链路上无休止传播
  PORT_TIMER_T      rrWhile;      /* 17.15.6 */
  PORT_TIMER_T      tcWhile;      /* 17.15.7 */
  PORT_TIMER_T      txCount;      /* 17.18.40 */
  PORT_TIMER_T      lnkWhile;

  PORT_TIMER_T*     timers[TIMERS_NUMBER]; /*list of timers */  // 定时器数组

  Bool              agreed;        /* 17.18.1 */
  PRIO_VECTOR_T     designPrio;    /* 17.18.2 */
  TIMEVALUES_T      designTimes;   /* 17.18.3 */
  Bool              forward;       /* 17.18.4 */
  Bool              forwarding;    /* 17.18.5 */
  INFO_IS_T         infoIs;        /* 17.18.6 */
  Bool              initPm;        /* 17.18.7  */
  Bool              learn;         /* 17.18.8 */
  Bool              learning;      /* 17.18.9 */
  Bool              mcheck;        /* 17.18.10 */
  PRIO_VECTOR_T     msgPrio;       /* 17.18.11 */   // 消息优先级向量
  TIMEVALUES_T      msgTimes;      /* 17.18.12 */   // 时间控制块
  Bool              newInfo;       /* 17.18.13 */
  Bool              operEdge;      /* 17.18.14 */
  Bool              adminEdge;     /* 17.18.14 */
  Bool              portEnabled;   /* 17.18.15 */
  PORT_ID           port_id;       /* 17.18.16 */   // 端口ID
  PRIO_VECTOR_T     portPrio;      /* 17.18.17 */   // 端口优先级向量    
  TIMEVALUES_T      portTimes;     /* 17.18.18 */
  Bool              proposed;      /* 17.18.19 */
  Bool              proposing;     /* 17.18.20 */
  Bool              rcvdBpdu;      /* 17.18.21 */   // 标记是否收到bpdu报文
  RCVD_MSG_T        rcvdMsg;       /* 17.18.22 */   
  Bool              rcvdRSTP;      /* 17/18.23 */   // 标记是否收到rstp bpdu
  Bool              rcvdSTP;       /* 17.18.24 */   // 标记是否收到stp bpdu
  Bool              rcvdTc;        /* 17.18.25 */   // 标记拓扑变化
  Bool              rcvdTcAck;     /* 17.18.26 */   // 标记拓扑变化ack
  Bool              rcvdTcn;       /* 17.18.27 */   // 标记是否收到TCN（拓扑变化通知）bpdu
  Bool              reRoot;        /* 17.18.28 */
  Bool              reselect;      /* 17.18.29 */   // 标记本端口是否重新发起端口选择
  PORT_ROLE_T       role;          /* 17.18.30 */
  Bool              selected;      /* 17.18.31 */   // 角色选择完成标记
  PORT_ROLE_T       selectedRole;  /* 17.18.32 */   // 端口角色
  Bool              sendRSTP;      /* 17.18.33 */
  Bool              sync;          /* 17.18.34 */
  Bool              synced;        /* 17.18.35 */
  Bool              tc;            /* 17.18.36 */
  Bool              tcAck;         /* 17.18.37 */
  Bool              tcProp;        /* 17.18.38 */

  Bool              updtInfo;      /* 17.18.41 */

  /* message information */
  unsigned char     msgBpduVersion; // bpdu版本
  unsigned char     msgBpduType;    // bpdu报文类型
  unsigned char     msgPortRole;    // 端口角色
  unsigned char     msgFlags;       // bpdu携带的标记，用于记录拓扑标志

  unsigned long     adminPCost; /* may be ADMIN_PORT_PATH_COST_AUTO */
  unsigned long     operPCost;
  unsigned long     operSpeed;
  unsigned long     usedSpeed;
  int               LinkDelay;   /* TBD: LinkDelay may be managed ? */
  Bool              adminEnable; /* 'has LINK' */
  Bool              wasInitBpdu;  
  Bool              admin_non_stp;  // 生成树使能/禁止标记

  Bool              p2p_recompute;
  Bool              operPointToPointMac;
  ADMIN_P2P_T       adminPointToPointMac;

  /* statistics */
  // 统计收到的三种bpdu报文的数量
  unsigned long     rx_cfg_bpdu_cnt;    // 记录config bpdu数量
  unsigned long     rx_rstp_bpdu_cnt;   // 记录rstp bpdu数量
  unsigned long     rx_tcn_bpdu_cnt;    // 记录CNT bpdu数量

  unsigned long     uptime;       /* 14.8.2.1.3.a */    // 运行时间，单位秒

  int               port_index;
  char*             port_name;

#ifdef STP_DBG
  unsigned int	    skip_rx;
  unsigned int	    skip_tx;
#endif
} PORT_T;

PORT_T*
STP_port_create (struct stpm_t* stpm, int port_index);

void
STP_port_delete (PORT_T* this);

int
STP_port_rx_bpdu (PORT_T* this, BPDU_T* bpdu, size_t len);

void
STP_port_init (PORT_T* this, struct stpm_t* stpm, Bool check_link);

#ifdef STP_DBG
int
STP_port_trace_state_machine (PORT_T* this, char* mach_name, int enadis, int vlan_id);

void
STP_port_trace_flags (char* title, PORT_T* this);
#endif

#endif /*  _STP_PORT_H__ */

