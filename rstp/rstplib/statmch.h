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
 *
 * 通用状态机
 **********************************************************************/

/* Generic (abstract state machine) state machine : 17.13, 17.14 */
 
#ifndef _STP_STATER_H__
#define _STP_STATER_H__

// 用于表示非有效状态
#define BEGIN  9999 /* distinct from any valid state */

// 状态机控制块
typedef struct state_mach_t {
  struct state_mach_t* next;    // 指向下一个状态机控制块

  char*         name; /* for debugging */   // 状态机名
#ifdef STP_DBG
  char          debug; /* 0- no dbg, 1 - port, 2 - stpm */
  unsigned int  ignoreHop2State;
#endif

  Bool          changeState;    // 状态变化标志,根据状态切换检测函数的返回值而定
  unsigned int  State;          // 当前状态

  void          (* concreteEnterState) (struct state_mach_t * );    // 执行进入某状态后的固定动作
  Bool          (* concreteCheckCondition) (struct state_mach_t * );// 检查状态切换条件并完成状态切换
  char*         (* concreteGetStatName) (int);                      // 获取状态名
  union {
    struct stpm_t* stpm;    // 指向本状态机所属网桥
    struct port_t* port;    // 指向本状态机所属端口控制块
    void         * owner;   // 指向本状态机的创建者，可能是某端口，也可能是某网桥
  } owner;              // 状态机属主

} STATE_MACH_T;

// 将具体某个状态机(由WHAT填入状态机名)添加到this下属的状态机链表中
#define STP_STATE_MACH_IN_LIST(WHAT)                              \
  {                                                               \
    STATE_MACH_T* abstr;                                          \
                                                                  \
      /*创建状态机，关联执行函数和检测函数，等级状态机属主*/\
    abstr = STP_state_mach_create (STP_##WHAT##_enter_state,      \
                                  STP_##WHAT##_check_conditions,  \
                                  STP_##WHAT##_get_state_name,    \
                                  this,                           \
                                  #WHAT);                         \
    abstr->next = this->machines;                                 \
    this->machines = abstr;                                       \
    this->WHAT = abstr;                       \
  }


STATE_MACH_T *
STP_state_mach_create (void (* concreteEnterState) (STATE_MACH_T*),
                       Bool (* concreteCheckCondition) (STATE_MACH_T*),
                       char * (* concreteGetStatName) (int),
                       void* owner, char* name);
                     
void
STP_state_mach_delete (STATE_MACH_T* this);

Bool
STP_check_condition (STATE_MACH_T* this);

Bool
STP_change_state (STATE_MACH_T* this);

Bool
STP_hop_2_state (STATE_MACH_T* this, unsigned int new_state);

#endif /* _STP_STATER_H__ */

