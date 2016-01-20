/*Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __ARCH_SYS_ARCH_H__
#define __ARCH_SYS_ARCH_H__

#include "arch/cc.h"
#include "ucos_ii.h"

#include "app_cfg.h" // define LWIP TASK Prio

#ifdef SYS_ARCH_GLOBALS
#define SYS_ARCH_EXT
#else
#define SYS_ARCH_EXT extern
#endif

/*-----------------macros-----------------------------------------------------*/
#define LWIP_STK_SIZE	256//300

//#define LWIP_TASK_MAX	  3		//max number of lwip tasks (TCPIP)
//#define LWIP_START_PRIO	10		        //first prio of lwip tasks in uC/OS-II

#define LWIP_TASK_MAX	  (LWIP_TASK_END_PRIO - LWIP_TASK_START_PRIO + 1)		// 8个		
//max number of lwip tasks (TCPIP) note LWIP TASK start with 1

#define LWIP_START_PRIO	  LWIP_TASK_START_PRIO		//first prio of lwip tasks in uC/OS-II

#if 0 /*hxj amend,date 2015-11-13 7:46*/
#define MAX_QUEUES        10	// 邮箱(消息队列)的最大数量
#else
#define MAX_QUEUES        20	// 邮箱(消息队列)的最大数量
#endif
#define MAX_QUEUE_ENTRIES 20	// 每个邮箱(消息队列)的最大消息数

#define SYS_MBOX_NULL (void *)0
#define SYS_SEM_NULL  (void *)0

#define sys_arch_mbox_tryfetch(mbox,msg) \
      sys_arch_mbox_fetch(mbox,msg,1)

/*-----------------type define------------------------------------------------*/

/** struct of LwIP mailbox */
typedef struct {
    OS_EVENT*   pQ;
    void*       pvQEntries[MAX_QUEUE_ENTRIES];
} TQ_DESCR, *PQ_DESCR;

typedef OS_EVENT *sys_sem_t; // type of semiphores
typedef PQ_DESCR sys_mbox_t; // type of mailboxes 实际其实用的而是ucos的消息队列
typedef INT8U sys_thread_t; // type of id of the new thread

typedef INT8U sys_prot_t;

/*-----------------global variables-------------------------------------------*/

SYS_ARCH_EXT OS_STK LWIP_TASK_STK[LWIP_TASK_MAX][LWIP_STK_SIZE];



#endif /* __ARCH_SYS_ARCH_H__ */

