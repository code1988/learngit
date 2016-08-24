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
 * 提供了快速生成树中所有状态机的一个抽象
 **********************************************************************/

/* Generic (abstract) state machine : 17.13, 17.14 */
 
#include "base.h"
#include "statmch.h"

#if STP_DBG
#  include "stpm.h"
#endif

// 创建状态机
STATE_MACH_T *STP_state_mach_create (void (*concreteEnterState) (STATE_MACH_T*),
                       Bool (*concreteCheckCondition) (STATE_MACH_T*),
                       char *(*concreteGetStatName) (int),
                       void *owner, char *name)
{
  STATE_MACH_T *this;

  // 分配状态机内存
  STP_MALLOC(this, STATE_MACH_T, "state machine");
 
  this->State = BEGIN;
  this->name = (char*) strdup (name);
  this->changeState = False;
#if STP_DBG
  this->debug = False;
  this->ignoreHop2State = BEGIN;
#endif
  this->concreteEnterState = concreteEnterState;
  this->concreteCheckCondition = concreteCheckCondition;
  this->concreteGetStatName = concreteGetStatName;
  this->owner.owner = owner;

  return this;
}
                              
// 删除状态机
void STP_state_mach_delete (STATE_MACH_T *this)
{
  free (this->name);
  STP_FREE(this, "state machine");
}

// 检查指定状态机的切换条件,条件通过则标志位置1
Bool STP_check_condition (STATE_MACH_T* this)
{
  Bool bret;

  // 调用每个状态机的检测状态函数
  bret = (*(this->concreteCheckCondition)) (this);  
  if (bret) {
    this->changeState = True;
  }
  
  return bret;
}
        
// 通过执行回调函数状态机真正进入新的状态
Bool STP_change_state (STATE_MACH_T* this)
{
  register int number_of_loops;

  // 遍历状态机链表，通过判断标志位changeState,确定是否更新状态机
  for (number_of_loops = 0; ; number_of_loops++) 
  {
    if (! this->changeState) return number_of_loops;
    
    (*(this->concreteEnterState)) (this);   // 更新状态机
    this->changeState = False;              // 状态变化标志清零
    STP_check_condition (this);             // 检查下一个状态,更新状态变化标志
  }

  return number_of_loops;
}

// 状态机状态更新，并置位状态切换标记
Bool STP_hop_2_state (STATE_MACH_T* this, unsigned int new_state)
{
#ifdef STP_DBG
  switch (this->debug) {
    case 0: break;
    case 1:
      if (new_state == this->State || new_state == this->ignoreHop2State) break;
      stp_trace ("%-8s(%s-%s): %s=>%s",
        this->name,
        *this->owner.port->owner->name ? this->owner.port->owner->name : "Glbl",
        this->owner.port->port_name,
        (*(this->concreteGetStatName)) (this->State),
        (*(this->concreteGetStatName)) (new_state));
      break;
    case 2:
      if (new_state == this->State) break;
      stp_trace ("%s(%s): %s=>%s", 
        this->name,
        *this->owner.stpm->name ? this->owner.stpm->name : "Glbl",
        (*(this->concreteGetStatName)) (this->State),
        (*(this->concreteGetStatName)) (new_state));
      break;
  }
#endif

  this->State = new_state;  // 更新当前状态
  this->changeState = True; // 置位状态切换标志
  return True;
}

