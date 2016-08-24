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

 /* STP machine instance : bridge per VLAN: 17.17 */
 
#include "base.h"
#include "stpm.h"
#include "stp_to.h" /* for STP_OUT_flush_lt */

static STPM_T *bridges = NULL;

static int
_stp_stpm_init_machine (STATE_MACH_T* this)
{
  this->State = BEGIN;
  (*(this->concreteEnterState)) (this);
  return 0;
}

// 根据需求执行状态机的回调函数，返回成功执行的状态机数量
// exit_on_non_zero_ret用于传入需求
static int _stp_stpm_iterate_machines (STPM_T* this,
                           int (*iter_callb) (STATE_MACH_T*),
                           Bool exit_on_non_zero_ret)
{
  register STATE_MACH_T* stater;
  register PORT_T*       port;
  int                    iret, mret = 0;

  /* state machines per bridge */
  // 遍历隶属于桥的状态机链表
  for (stater = this->machines; stater; stater = stater->next) 
  {
      // 检查每个状态机是否状态改变
    iret = (*iter_callb) (stater);

    // 根据需求决定是否继续遍历
    if (exit_on_non_zero_ret && iret)
      return iret;
    else
      mret += iret;
  }

  /* state machines per port */
  // 遍历每个端口
  for (port = this->ports; port; port = port->next) 
  {
    // 遍历隶属于端口的状态机
    for (stater = port->machines; stater; stater = stater->next) 
    {
      // 检查每个状态机是否状态改变
      iret = (*iter_callb) (stater);

      // 根据需求决定是否继续遍历
      if (exit_on_non_zero_ret && iret)
        return iret;
      else
        mret += iret;
    }
  }
  
  // 返回状态变化的状态机数量 
  return mret;
}

void
_stp_stpm_init_data (STPM_T* this)
{
  STP_VECT_create (&this->rootPrio,
                   &this->BrId,
                   0,
                   &this->BrId,
                   0, 0);

  this->BrTimes.MessageAge = 0;

  STP_copy_times (&this->rootTimes, &this->BrTimes);
}

static unsigned char
_check_topoch (STPM_T* this)
{
  register PORT_T*  port;
  
  for (port = this->ports; port; port = port->next) {
    if (port->tcWhile) {
      return 1;
    }
  }
  return 0;
}

void STP_stpm_one_second (STPM_T* param)
{
  STPM_T*           this = (STPM_T*) param;
  register PORT_T*  port;
  register int      iii;

  if (STP_ENABLED != this->admin_state) return;

  for (port = this->ports; port; port = port->next) {
    for (iii = 0; iii < TIMERS_NUMBER; iii++) {
      if (*(port->timers[iii]) > 0) {
        (*port->timers[iii])--;
      }
    }    
    port->uptime++;
  }

  STP_stpm_update (this);
  this->Topo_Change = _check_topoch (this);
  if (this->Topo_Change) {
    this->Topo_Change_Count++;
    this->timeSince_Topo_Change = 0;
  } else {
    this->Topo_Change_Count = 0;
    this->timeSince_Topo_Change++;
  }
}

// 创建一个网桥控制块，并初始化，特别是创建了内部一个端口角色选择状态机
STPM_T* STP_stpm_create (int vlan_id, char* name)
{
  STPM_T* this;

  // 创建网桥控制块节点并插入表头
  STP_NEW_IN_LIST(this, STPM_T, bridges, "stp instance");

  this->admin_state = STP_DISABLED;
  
  this->vlan_id = vlan_id;
  if (name) {
    STP_STRDUP(this->name, name, "stp bridge name");
  }

  this->machines = NULL;
  this->ports = NULL;

  // 创建端口角色选择状态机,隶属于当前网桥
  STP_STATE_MACH_IN_LIST(rolesel);

#ifdef STP_DBG
  /* this->rolesel->debug = 2;  */
#endif

  return this;
}

// 使能/禁止生成树,并完成生成树状态检测和切换
int STP_stpm_enable (STPM_T* this, UID_STP_MODE_T admin_state)
{
  int rc = 0;

  if (admin_state == this->admin_state) {
    /* nothing to do :) */
    return 0;
  }

  if (STP_ENABLED == admin_state) {
    // 开启生成树,并完成生成树状态检测和切换
    rc = STP_stpm_start (this);
    this->admin_state = admin_state;
  } 
  else {
    this->admin_state = admin_state;
    // 关闭生成树
    STP_stpm_stop (this);
  }
  
  return rc;
}

void
STP_stpm_delete (STPM_T* this)
{
  register STPM_T*       tmp;
  register STPM_T*       prev;
  register STATE_MACH_T* stater;
  register PORT_T*       port;
  register void*         pv;

  STP_stpm_enable (this, STP_DISABLED);
  
  for (stater = this->machines; stater; ) {
    pv = (void*) stater->next;
    STP_state_mach_delete (stater);
    this->machines = stater = (STATE_MACH_T*) pv;
  }

  for (port = this->ports; port; ) {
    pv = (void*) port->next;
    STP_port_delete (port);
    this->ports = port = (PORT_T*) pv;
  }

  prev = NULL;
  for (tmp = bridges; tmp; tmp = tmp->next) {
    if (tmp->vlan_id == this->vlan_id) {
      if (prev) {
        prev->next = this->next;
      } else {
        bridges = this->next;
      }
      
      if (this->name)
        STP_FREE(this->name, "stp bridge name");
      STP_FREE(this, "stp instance");
      break;
    }
    prev = tmp;
  }
}

// 开启生成树,并完成生成树状态检测和切换
int STP_stpm_start (STPM_T* this)
{
  register PORT_T* port;

#ifndef RSTP_JWS
  if (! this->ports) { /* there are not any ports :( */
    return STP_There_Are_No_Ports;
  }
#endif

  if (! STP_compute_bridge_id (this)) {/* can't compute bridge id ? :( */
    return STP_Cannot_Compute_Bridge_Prio;
  }

  /* check, that the stpm has unique bridge Id */
  if (0 != STP_stpm_check_bridge_priority (this)) {
    /* there is an enabled bridge with same ID :( */
    return STP_Invalid_Bridge_Priority;
  }

  _stp_stpm_init_data (this);

  for (port = this->ports; port; port = port->next) {
    STP_port_init (port, this, True);
  }

#ifndef STRONGLY_SPEC_802_1W
  /* A. see comment near STRONGLY_SPEC_802_1W in topoch.c */
  /* B. port=0 here means: delete for all ports */
#ifdef STP_DBG
  stp_trace("%s (all, start stpm)",
        "clearFDB");
#endif

  STP_OUT_flush_lt (0, this->vlan_id, LT_FLASH_ONLY_THE_PORT, "start stpm");
#endif

  _stp_stpm_iterate_machines (this, _stp_stpm_init_machine, False);

  
    // 这里完成了当前网桥以及端口所属的所有状态机的状态检测和切换
  STP_stpm_update (this);

  return 0;
}

void
STP_stpm_stop (STPM_T* this)
{
}

// 这里完成了当前网桥以及端口所属的所有状态机的状态检测和切换
int STP_stpm_update (STPM_T* this) /* returns number of loops */
{
  register Bool     need_state_change;
  register int      number_of_loops = 0;

  need_state_change = False; 
  
  for (;;) 
  {/* loop until not need changes */
    // 检查状态变化的状态机,一次找到一个就行
    need_state_change = _stp_stpm_iterate_machines (this,
                                                   STP_check_condition,
                                                   True);
    // 当再也找不到时返回执行了检查和切换的状态机总数
    if (! need_state_change) return number_of_loops;

    number_of_loops++;
    /* here we know, that at least one stater must be
       updated (it has changed state) */
    // 切换所有状态变化的状态机状态
    number_of_loops += _stp_stpm_iterate_machines (this,
                                                  STP_change_state,
                                                  False);

  }

  // 返回执行了检查和切换的状态机总数
  return number_of_loops;
}

// 生成桥ID
BRIDGE_ID *STP_compute_bridge_id (STPM_T* this)
{
#ifdef RSTP_JWS
  register PORT_T* port;
  unsigned char old[6], new[6];
  memset(&old, 0xff, sizeof(old));

  for (port = this->ports; port; port = port->next) {
    STP_OUT_get_port_mac (port->port_index, new);
    if (memcmp(new, old, sizeof(old)) < 0)
      memcpy(old, new, sizeof(old));
  }

  memcpy(this->BrId.addr, old, sizeof(old));

  return &this->BrId;
#else
  register PORT_T* port;
  register PORT_T* min_num_port=NULL;
  int              port_index = 0;

  for (port = this->ports; port; port = port->next) {
    if (! port_index || port->port_index < port_index) {
      min_num_port = port;
      port_index = port->port_index;
    }
  }

  if (! min_num_port) return NULL; /* IMHO, it may not be */

  STP_OUT_get_port_mac (min_num_port->port_index, this->BrId.addr);

  return &this->BrId;
#endif 
}

// 获取网桥链表头
STPM_T *STP_stpm_get_the_list (void)
{
  return bridges;
}

// 置位每个端口的reselect、复位每个端口的selected
void STP_stpm_update_after_bridge_management (STPM_T* this)
{
  register PORT_T* port;

  for (port = this->ports; port; port = port->next) {
    port->reselect = True;
    port->selected = False;
  }
}

int
STP_stpm_check_bridge_priority (STPM_T* this)
{
  register STPM_T* oth;

  for (oth = bridges; oth; oth = oth->next) {
    if (STP_ENABLED == oth->admin_state && oth != this &&
        ! STP_VECT_compare_bridge_id (&this->BrId, &oth->BrId)) {
      return STP_Invalid_Bridge_Priority;
    }
  }

  return 0;
}

const char*
STP_stpm_get_port_name_by_id (STPM_T* this, PORT_ID port_id)
{
  register PORT_T* port;

  for (port = this->ports; port; port = port->next) {
    if (port_id == port->port_id) {
        return port->port_name;
    }
  }

  return "Undef?";
}





