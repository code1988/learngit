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

/* STP priority vectors API : 17.4.2 */
 
#include "base.h"
#include "stp_bpdu.h"
#include "vector.h"

// 比较桥ID大小(返回值 -1:后大 1:前大 0:相等)
int STP_VECT_compare_bridge_id (BRIDGE_ID* b1, BRIDGE_ID* b2)
{
    // 先比较桥优先级
    if (b1->prio < b2->prio)
        return -1;

    if (b1->prio > b2->prio)
        return 1;
    // 桥优先级相同再比较桥地址
    return memcmp (b1->addr, b2->addr, 6);
}

void
STP_VECT_copy (OUT PRIO_VECTOR_T* t, IN PRIO_VECTOR_T* f)
{
  memcpy (t, f, sizeof (PRIO_VECTOR_T));
}

void
STP_VECT_create (OUT PRIO_VECTOR_T* t,
                 IN BRIDGE_ID* root_br,
                 IN unsigned long root_path_cost,
                 IN BRIDGE_ID* design_bridge,
                 IN PORT_ID design_port,
                 IN PORT_ID bridge_port)
{
  memcpy (&t->root_bridge, root_br, sizeof (BRIDGE_ID));
  t->root_path_cost = root_path_cost;
  memcpy (&t->design_bridge, design_bridge, sizeof (BRIDGE_ID));
  t->design_port = design_port;
  t->bridge_port = bridge_port;
}

// 比较优先级向量(返回值 <0:前优 >0:后优)
int STP_VECT_compare_vector (PRIO_VECTOR_T* v1, PRIO_VECTOR_T* v2)
{
    int bridcmp;

    // 比较根桥ID，小则优
    bridcmp = STP_VECT_compare_bridge_id (&v1->root_bridge, &v2->root_bridge);

    // 如果根桥ID前优则直接返回
    if (bridcmp < 0) return bridcmp;

    // 对于根桥ID相等的情况，需要进一步判断
    if (! bridcmp) 
    {
        // 比较路径成本，小则优
        bridcmp = v1->root_path_cost - v2->root_path_cost;
        // 如果路径成本前优则直接返回
        if (bridcmp < 0) return bridcmp;

        // 对于路径成本也相等的情况，需要进一步判断
        if (! bridcmp) 
        {
            // 比较指定桥ID,小则优
            bridcmp = STP_VECT_compare_bridge_id (&v1->design_bridge, &v2->design_bridge);

            // 如果指定桥ID前优则直接返回
            if (bridcmp < 0) return bridcmp;

            // 对于指定桥ID也相等的情况,需要进一步判断
            if (! bridcmp) 
            {
                // 比较指定端口大小，小则优
                bridcmp = v1->design_port - v2->design_port;

                // 如果指定端口前优则直接返回
                if (bridcmp < 0) return bridcmp;

                // 对于指定端口大小相等的情况，最后比较根端口大小，小则优
                if (! bridcmp)
                    return v1->bridge_port - v2->bridge_port;
            }
        }
    }

    // 运行到这里必然意味着后优
    return bridcmp;
}

static unsigned short
stp_vect_get_short (IN unsigned char* f)
{
  return ntohs (*(unsigned short *)f);
}

static void
stp_vect_set_short (IN unsigned short f, OUT unsigned char* t)
{
  *(unsigned short *)t = htons (f);
}

static void
stp_vect_get_bridge_id (IN unsigned char* c_br, OUT BRIDGE_ID* bridge_id)
{
  bridge_id->prio = stp_vect_get_short (c_br);
  memcpy (bridge_id->addr, c_br + 2, 6);
}

static void
stp_vect_set_bridge_id (IN BRIDGE_ID* bridge_id, OUT unsigned char* c_br)
{
  stp_vect_set_short (bridge_id->prio, c_br);
  memcpy (c_br + 2, bridge_id->addr, 6);
}

// 将bpdu报文主体转换为优先级向量
void STP_VECT_get_vector (IN BPDU_BODY_T* b, OUT PRIO_VECTOR_T* v)
{
  stp_vect_get_bridge_id (b->root_id, &v->root_bridge);

  v->root_path_cost = ntohl (*((long*) b->root_path_cost));

  stp_vect_get_bridge_id (b->bridge_id, &v->design_bridge);

  v->design_port = stp_vect_get_short (b->port_id);
}

// 将优先级向量转换为bpdu报文主体
void STP_VECT_set_vector (IN PRIO_VECTOR_T* v, OUT BPDU_BODY_T* b)
{
  unsigned long root_path_cost;

  stp_vect_set_bridge_id (&v->root_bridge, b->root_id);

  root_path_cost = htonl (v->root_path_cost);
  memcpy (b->root_path_cost, &root_path_cost, 4);

  stp_vect_set_bridge_id (&v->design_bridge, b->bridge_id);

  stp_vect_set_short (v->design_port, b->port_id);
}

#ifdef STP_DBG

void
STP_VECT_br_id_print (IN char *title, IN BRIDGE_ID* br_id, IN Bool cr)
{
  Print ("%s=%04lX-%02x%02x%02x%02x%02x%02x",
            title,
          (unsigned long) br_id->prio,
          (unsigned char) br_id->addr[0],
          (unsigned char) br_id->addr[1],
          (unsigned char) br_id->addr[2],
          (unsigned char) br_id->addr[3],
          (unsigned char) br_id->addr[4],
          (unsigned char) br_id->addr[5]);
  Print (cr ? "\n" : " ");
}

void
STP_VECT_print (IN char *title, IN PRIO_VECTOR_T *v)
{
  Print ("%s:", title);
  STP_VECT_br_id_print ("rootBr", &v->root_bridge, False);
    
/****
  Print (" rpc=%ld ", (long) v->root_path_cost);
****/

  STP_VECT_br_id_print ("designBr", &v->design_bridge, False);

/****/
  Print (" dp=%lx bp=%lx ",
          (unsigned long) v->design_port,
          (unsigned long) v->bridge_port);
/***********/
  Print ("\n");
}
#endif

