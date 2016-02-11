/**
 * \file lwipopts.h - Configuration options for lwIP
 *
 * Copyright (c) 2010 Texas Instruments Incorporated
 */
/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
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

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__


//#define LWIP_DEBUG              1



//#define sys_bios_ind_sdk
/*****************************************************************************
**                           CONFIGURATIONS
*****************************************************************************/
/*
** The macro CPSW_DUAL_MAC_MODE shall be defined for using CPSW ports in
** Dual MAC mode.
*/
#define CPSW_DUAL_MAC_MODE

/*
** The macro CPSW_SWITCH_CONFIG shall be defined to use for configuring
** CPSW Switch for ALE & PORT Configurations in Switch Mode
** Dual MAC mode.
*/
#ifndef CPSW_DUAL_MAC_MODE
#define CPSW_SWITCH_CONFIG
#endif

/*
** If Static IP address to be used, give it here. This value shall be 0 if
** dynamic IP address is to be used.
** For Example, for IP Address 192.168.247.1, use the corresponding hex
** value 0xC0A8F701.
*/
#ifdef CPSW_DUAL_MAC_MODE
#define STATIC_IP_ADDRESS_PORT1         0xC0A86464    //ip:192.168.100.100
#define STATIC_IP_ADDRESS_PORT2         0             /* Port 2 static IP */

#else
#define STATIC_IP_ADDRESS               0             /* Static IP in
                                                         Switch Mode */
#endif

/*
** The below macro should be defined for using lwIP with cache. For cache
** enabling, pbuf pool shall be cache line aligned. This is done by using
** separate pool for each memory. The alignment of pbuf pool to cache line
** size is done in /ports/cpsw/include/arch/cc.h.
*/
#define LWIP_CACHE_ENABLED

#define SOC_CACHELINE_SIZE_BYTES        64            /* Number of bytes in
                                                         a cache line */
/*
** The timeout for DHCP completion. lwIP library will wait for DHCP
** completion for (LWIP_DHCP_TIMEOUT / 100) seconds.
*/
#define LWIP_DHCP_TIMEOUT               500

/*
** The number of times DHCP is attempted. Each time, the library will wait
** for (LWIP_DHCP_TIMEOUT / 100) seconds for DHCP completion.
*/
#define NUM_DHCP_TRIES                  5

/*****************************************************************************
**            lwIP SPECIFIC DEFINITIONS - To be used by lwIP stack
*****************************************************************************/
#define HOST_TMR_INTERVAL               0 // 主机定时器间隔，注意不是lwip协议栈本身的定时器间隔，可用来检查IP地址的获取情况或者周期性地调用一些函数。
#define DYNAMIC_HTTP_HEADERS

/*****************************************************************************
**                    Platform specific locking
*****************************************************************************/
//SYS_LIGHTWEIGHT_PROT实际已经是1，使能了保护操作
#define SYS_LIGHTWEIGHT_PROT            0    // default is 0 针对Stellaris必须1，主要是因为在分配内存的时候，要确保总中断关闭。防止内存分配失败。
#define NO_SYS                          0    //use ucos_II
#define NO_SYS_NO_TIMERS                1

/*****************************************************************************
**                          Memory Options
*****************************************************************************/
#define MEM_ALIGNMENT                   4
#define MEM_SIZE                        (128 * 1024) /* 128K */// default is 1600 该值在ZI中占了很大的份额。
                                        //这就是堆内存的大小，如果应用程序有大量数据在发送是要被复制，那么该值就应该尽量大一点。由此可见，发送缓冲区从这里边分配。

#define MEMP_NUM_PBUF                   96
#define MEMP_NUM_TCP_PCB                32
#define PBUF_POOL_SIZE                  210

#ifdef LWIP_CACHE_ENABLED
#define MEMP_SEPARATE_POOLS             1            /* We want the pbuf
                                                        pool cache line
                                                        aligned*/
#endif

/*****************************************************************************
**                           IP Options
*****************************************************************************/
#define IP_REASSEMBLY                   1		// IP包分片重组功能使能
#define IP_FRAG                         0

/*****************************************************************************
**                           DHCP Options
*****************************************************************************/
#define LWIP_DHCP                       0   //使能DHCP
#define DHCP_DOES_ARP_CHECK             0

/*****************************************************************************
**                           Auto IP  Options
*****************************************************************************/
#define LWIP_AUTOIP                     0
#define LWIP_DHCP_AUTOIP_COOP           ((LWIP_DHCP) && (LWIP_AUTOIP))

/*****************************************************************************
**                           TCP  Options
*****************************************************************************/
#define TCP_MSS                         1500
#define TCP_WND                         (8 * TCP_MSS)
#define TCP_SND_BUF                     (8 * TCP_MSS)
#define TCP_OVERSIZE                    TCP_MSS

/*****************************************************************************
**                           PBUF  Options
*****************************************************************************/
#define PBUF_LINK_HLEN                  14				// 链路层首部长度
#define PBUF_POOL_BUFSIZE               1520//1520         /* + size of struct pbuf
                                                       // shall be cache line
                                                       // aligned be enabled */
#define ETH_PAD_SIZE                    0
#define LWIP_NETCONN                    1   //使用netconn

/*****************************************************************************
**                           Socket  Options
*****************************************************************************/
#define LWIP_SOCKET                     1
// LWIP  MUTEX 使用信号量实现 sys.h里用到此宏
#define LWIP_COMPAT_MUTEX               1
#define LWIP_PROVIDE_ERRNO              1


#endif /* __LWIPOPTS_H__ */
