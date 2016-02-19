/*
*  \file lwiplib.c
*
*  \brief lwip related initializations
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

/*
** Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
** ALL RIGHTS RESERVED
*/

/*
** lwIP Compile Time Options for StarterWare.
*/
#include "sysconfig.h"
#include "lwiplib.h"
#include "cpswif.h"
#include "dhcp.h"
#include "hw_types.h"
#include "Llmnr.h"
#include "Ssdp.h"
/*
** lwIP high-level API/Stack/IPV4/SNMP/Network Interface/PPP codes
*/
//#include "src/api/api_lib.c"
//#include "src/api/api_msg.c"
//#include "src/api/err.c"
//#include "src/api/netbuf.c"
//#include "src/api/netdb.c"
//#include "src/api/netifapi.c"
//#include "src/api/tcpip.c"
//#include "src/api/sockets.c"
//
//#include "src/core/def.c"
//#include "src/core/timers.c"
//#include "src/core/dhcp.c"
//#include "src/core/dns.c"
//#include "src/core/init.c"
//#include "src/core/mem.c"
//#include "src/core/memp.c"
//#include "src/core/netif.c"
//#include "src/core/pbuf.c"
//#include "src/core/raw.c"
//#include "src/core/stats.c"
//#include "src/core/sys.c"
//#include "src/core/tcp.c"
//#include "src/core/tcp_in.c"
//#include "src/core/tcp_out.c"
//#include "src/core/udp.c"
//
//#include "src/core/ipv4/autoip.c"
//#include "src/core/ipv4/icmp.c"
//#include "src/core/ipv4/igmp.c"
//#include "src/core/ipv4/inet.c"
//#include "src/core/ipv4/inet_chksum.c"
//#include "src/core/ipv4/ip.c"
//#include "src/core/ipv4/ip_addr.c"
//#include "src/core/ipv4/ip_frag.c"
//
//#include "src/core/snmp/asn1_dec.c"
//#include "src/core/snmp/asn1_enc.c"
//#include "src/core/snmp/mib2.c"
//#include "src/core/snmp/mib_structs.c"
//#include "src/core/snmp/msg_in.c"
//#include "src/core/snmp/msg_out.c"
//
//#include "src/netif/etharp.c"
//
//#include "src/netif/ppp/auth.c"
//#include "src/netif/ppp/chap.c"
//#include "src/netif/ppp/chpms.c"
//#include "src/netif/ppp/fsm.c"
//#include "src/netif/ppp/ipcp.c"
//#include "src/netif/ppp/lcp.c"
//#include "src/netif/ppp/magic.c"
//#include "src/netif/ppp/md5.c"
//#include "src/netif/ppp/pap.c"
//#include "src/netif/ppp/ppp.c"
//#include "src/netif/ppp/ppp_oe.c"
//#include "src/netif/ppp/randm.c"
//#include "src/netif/ppp/vj.c"
//
///*
//** CPSW-specific lwIP interface/porting layer code.
//*/
//#include "ports/cpsw/perf.c"
//#include "ports/cpsw/sys_arch.c"
#include "ports/cpsw/netif/cpswif.c"
//#include "locator.c"

/******************************************************************************
**                       INTERNAL FUNCTION PROTOTYPES
******************************************************************************/

#if LWIP_DHCP
static void lwIPDHCPComplete(unsigned int ifNum);
#endif

/******************************************************************************
**                       INTERNAL VARIABLE DEFINITIONS
******************************************************************************/
/*
** The lwIP network interface structure for CPSW ports.
*/
#ifdef CPSW_DUAL_MAC_MODE
static struct netif cpswNetIF[MAX_CPSW_INST * MAX_SLAVEPORT_PER_INST];
#else
static struct netif cpswNetIF[MAX_CPSW_INST];
#endif

/*
** Helper to identify ports
*/
 struct cpswportif cpswPortIf[MAX_CPSW_INST * MAX_SLAVEPORT_PER_INST];

/******************************************************************************
**                          FUNCTION DEFINITIONS
******************************************************************************/
/**
 * \brief   This function waits for DHCP completion with a timeout
 *
 * \param   ifNum  The netif number for the interface
 *
 * \return  None
*/


#if LWIP_DHCP
static void lwIPDHCPComplete(unsigned int ifNum)
{
    unsigned int dhcpTries = NUM_DHCP_TRIES;
    int cnt;
    volatile unsigned char *state=NULL;


    while(dhcpTries--)
    {
//        LWIP_PRINTF("\n\rDHCP Trial %d...", (NUM_DHCP_TRIES - dhcpTries));
        dhcp_start(&cpswNetIF[ifNum]);

        cnt = LWIP_DHCP_TIMEOUT;

        /* Check for DHCP completion for 'cnt' number of times, each 10ms */
        while(cnt--)
        {
            //delay(10);
            OSTimeDlyHMSM(0,0,0,10);
            state = &(cpswNetIF[ifNum].dhcp->state);
            if(DHCP_BOUND == *state)
            {
                return;
            }
        }
    }

//    LWIP_PRINTF("\n\rUnable to complete DHCP! \n\r");
}

#endif

/**
  * @brief  LwIP periodic tasks
  * @param  localtime the current LocalTime value
  * @retval None
  */
#include "netconf.h"
static int static_ip_timeout=0;
static void LwIP_DHCP_task(void *arg)
{
  static struct ip_addr ip={0};
  bool static_ip = arg? TRUE:FALSE;
  if((!ip_addr_isany(&cpswNetIF[0].ip_addr)&&
     !ip_addr_cmp(&cpswNetIF[0].ip_addr,&ip)) || (++static_ip_timeout > 60 && static_ip)){
       if(!static_ip){
         ip_addr_set(&ip,&cpswNetIF[0].ip_addr);
         SetLocalhost(&cpswNetIF[0].ip_addr);
         SetNetMask(&cpswNetIF[0].netmask);
         SetGateway(&cpswNetIF[0].gw);
       }else{
         ip_addr_set(&ip,Localhost());
         netif_set_addr(&cpswNetIF[0], Localhost(),GetNetMask(),GetGateway());
         #if LWIP_DHCP
         dhcp_stop(&cpswNetIF[0]);
          #endif
         return;
       }
  }
  tcpip_timeout(1000, LwIP_DHCP_task, arg);
}

/**
 *
 * \brief Initializes the lwIP TCP/IP stack.
 *
 * \param lwipIf  The interface structure for lwIP
 *
 * \return IP Address.
*/
/**
 * @brief TcpipInitDone wait for tcpip init being done
 *
 * @param arg the semaphore to be signaled
 */
//void  (*ETH_PostSem)(void) = NULL;
unsigned int lwIPInit(LWIP_IF *lwipIf,int *ret_code)
{

    DBG_NET("in lwIPInit 0:\r\n");


    struct ip_addr ipaddr;
    struct ip_addr netmask;
    struct ip_addr gwaddr;
    unsigned int *ipAddrPtr;
    static unsigned int lwipInitFlag = 0;
    unsigned int ifNum;
    unsigned int temp;

    /* Setup the network address values. */
    if(lwipIf->ipMode == IPADDR_USE_STATIC)
    {
        ipaddr.addr = htonl(lwipIf->ipAddr);
        netmask.addr = htonl(lwipIf->netMask);
        gwaddr.addr = htonl(lwipIf->gwAddr);
    }
    else
    {
        ipaddr.addr = 0;
        netmask.addr = 0;
        gwaddr.addr = 0;
    }

#ifdef CPSW_DUAL_MAC_MODE
    ifNum = (lwipIf->instNum * MAX_SLAVEPORT_PER_INST) + lwipIf->slvPortNum - 1;
#else
    ifNum = lwipIf->instNum;
#endif


    cpswPortIf[ifNum].inst_num = lwipIf->instNum;
    cpswPortIf[ifNum].port_num = lwipIf->slvPortNum;

    /* set MAC hardware address */
    for(temp = 0; temp < LEN_MAC_ADDRESS; temp++)
    {
        cpswPortIf[ifNum].eth_addr[temp] =
                         lwipIf->macArray[(LEN_MAC_ADDRESS - 1) - temp];
    }
     //add loop interface //set local loop-interface 127.0.0.1
    /*
    ** Create, configure and add the Ethernet controller interface with
    ** default settings.  ip_input should be used to send packets directly to
    ** the stack. The lwIP will internaly call the cpswif_init function.
    */
    //ip_input))//cpswif_init硬件初始化也放在这里
    netif_add(&cpswNetIF[ifNum], &ipaddr, &netmask, &gwaddr,&cpswPortIf[ifNum], cpswif_init, tcpip_input);
    
    if(0 == lwipInitFlag)
    {
    	// 设置系统的缺省网络接口
        netif_set_default(&cpswNetIF[ifNum]);
        lwipInitFlag = 1;
    }

    /* Start DHCP, if enabled. */
#if LWIP_DHCP
    if(lwipIf->ipMode == IPADDR_USE_DHCP)
    {
        lwIPDHCPComplete(ifNum);
    }
#endif

    /* Start AutoIP, if enabled and DHCP is not. */
#if LWIP_AUTOIP
    if(lwipIf->ipMode == IPADDR_USE_AUTOIP)
    {
        autoip_start(&cpswNetIF[ifNum]);
    }
#endif

    if((lwipIf->ipMode == IPADDR_USE_STATIC) ||(lwipIf->ipMode == IPADDR_USE_AUTOIP))
    {
        // 使能网络接口
        netif_set_up(&cpswNetIF[ifNum]);

    }
    ipAddrPtr = (unsigned int*)&(cpswNetIF[ifNum].ip_addr);

    /*  Creates a new DHCP client for this interface on the first call.
    Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
    the predefined regular intervals after starting the client.
    You can peek in the netif->dhcp struct for the actual DHCP status.*/
#if LWIP_DHCP
    dhcp_start(&cpswNetIF[ifNum]);
    tcpip_timeout(1000, LwIP_DHCP_task, (void*)0);
#endif

    char host_name[64]={0};
    sprintf(host_name,"AM335X_%02X_%02X_%02X_%02X_%02X_%02X"
                    ,lwipIf->macArray[5]
                    ,lwipIf->macArray[4]
                    ,lwipIf->macArray[3]
                    ,lwipIf->macArray[2]
                    ,lwipIf->macArray[1]
                    ,lwipIf->macArray[0]);

    SetHostName(host_name);
    DBG_NET("in lwIPInit 5_1:call SetHostName()   end \r\n");

/*
    DBG_NET("in lwIPInit 6_0:call SsdpInit() start \r\n");
    SsdpInit(&cpswNetIF[ifNum]);
    DBG_NET("in lwIPInit 6_1:call SsdpInit()   end \r\n");


    DBG_NET("in lwIPInit 7_0:call LlmnrInit() start \r\n");
    LlmnrInit(&cpswNetIF[ifNum]);
    DBG_NET("in lwIPInit 7_0:call LlmnrInit()   end \r\n");
*/


    DBG_NET("in lwIPInit 100: \r\n");

    return (*ipAddrPtr);
}





/****************************************************************************************************
**名称:unsigned int lwIPInit_re(void)
**功能:上电重新初始化
* 入口:无
* 出口:成功返回1
**auth:hxj, date: 2014-12-23 10:22
*****************************************************************************************************/
unsigned int lwIPInit_re(void)
{
    DBG_NET("in lwIPInit_re 0:\r\n");

    unsigned int ifNum=0;
    err_t ret_err;
    unsigned int ret=0;

    //ret_err=cpswif_init(&cpswNetIF[ifNum]);

    netif_set_down(&cpswNetIF[ifNum]);
    MY_DELAY_X_MS(500);
    ret_err=cpswif_port_init(&cpswNetIF[ifNum]);
    if(ERR_OK !=ret_err)
    {
        DBG_NET("in lwIPInit_re 1:call cpswif_port_init err\r\n");
        ret=0;
    }
    else
    {
        DBG_NET("in lwIPInit_re 2:call cpswif_port_init ok\r\n");
        netif_set_up(&cpswNetIF[ifNum]);
        ret=1;
    }

    DBG_NET("in lwIPInit_re 100:\r\n");

    return ret;

}








/*
 * \brief   Checks if the ethernet link is up
 *
 * \param   instNum     The instance number of CPSW module
 * \param   slvPortNum  The Slave Port Number
 *
 * \return  Interface status.
*/
unsigned int lwIPNetIfStatusGet(unsigned int instNum, unsigned int slvPortNum)
{
    unsigned int ifNum;

    ifNum = instNum * MAX_SLAVEPORT_PER_INST + slvPortNum - 1;

    return (cpswif_netif_status(&cpswNetIF[ifNum]));
}

/*
 * \brief   Checks if the ethernet link is up
 *
 * \param   instNum     The instance number of CPSW module
 * \param   slvPortNum  The Slave Port Number
 *
 * \return  The link status.
*/
unsigned int lwIPLinkStatusGet(unsigned int instNum, unsigned int slvPortNum)
{
    return (cpswif_link_status(instNum, slvPortNum));
}

/**
 * \brief   Interrupt handler for Receive Interrupt. Directly calls the
 *          cpsw interface receive interrupt handler.
 *
 * \param   instNum  The instance number of CPSW module for which receive
 *                   interrupt happened
 *
 * \return  None.
*/
void lwIPRxIntHandler(unsigned int instNum)
{
    cpswif_rx_inthandler(instNum, &cpswNetIF[0]);
}

/**
 * \brief   Interrupt handler for Transmit Interrupt. Directly calls the
 *          cpsw interface transmit interrupt handler.
 *
 * \param   instNum  The instance number of CPSW module for which transmit
 *                   interrupt happened
 *
 * \return  None.
*/
void lwIPTxIntHandler(unsigned int instNum)
{
    cpswif_tx_inthandler(instNum);
}

/**
 * \brief   Starts DHCP negotiation
 *
 * \param   instNum     The instance number of CPSW module
 * \param   slvPortNum  The Slave Port Number
 *
 * \return  IP address assigned. If IP acquisition failed, zero will be returned.
*/
unsigned int lwIPDHCPStart(unsigned int instNum, unsigned int slvPortNum)
{
    unsigned int *ipAddrPtr;
    unsigned int ifNum;

    ifNum = instNum * MAX_SLAVEPORT_PER_INST + slvPortNum - 1;

#if LWIP_DHCP
    lwIPDHCPComplete(ifNum);
#endif

    ipAddrPtr = (unsigned int*)&(cpswNetIF[ifNum].ip_addr);

    return (*ipAddrPtr);
}

/***************************** End Of File ***********************************/















