/*
 * @file
 * Transmission Control Protocol, incoming traffic
 *
 * The input processing functions of the TCP layer.
 *
 * These functions are generally called in the order (ip_input() ->)
 * tcp_input() -> * tcp_process() -> tcp_receive() (-> application).
 * 
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

#include "lwip/opt.h"

#if LWIP_TCP /* don't build if not configured for use in lwipopts.h */

#include "lwip/tcp_impl.h"
#include "lwip/def.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/inet_chksum.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "arch/perf.h"

/* These variables are global to all functions involved in the input
   processing of TCP segments. They are set by the tcp_input()
   function. */
static struct tcp_seg inseg;	// 用来组织收到的报文段pbuf
static struct tcp_hdr *tcphdr;	// 指向报文段中的TCP首部
static struct ip_hdr *iphdr;	// 指向报文段中的IP首部
static u32_t seqno, ackno;		//  TCP首部中的序号和确认号
static u8_t flags;				// TCP首部中的标志字段
static u16_t tcplen;			// TCP报文段中数据的长度,对于SYN或FIN报文，该长度要加1

// 以下2个变量，recv_flags用于存储tcp_process函数的处理结果，recv_data用于存储向上层提交的数据，
static u8_t recv_flags;			// 目前只会保存3个处理结果:TF_GOT_FIN、TF_RESET、TF_CLOSED
static struct pbuf *recv_data;	

struct tcp_pcb *tcp_input_pcb;	// 处理当前报文段的控制块

/* Forward declarations. */
static err_t tcp_process(struct tcp_pcb *pcb);
static void tcp_receive(struct tcp_pcb *pcb);
static void tcp_parseopt(struct tcp_pcb *pcb);

static err_t tcp_listen_input(struct tcp_pcb_listen *pcb);
static err_t tcp_timewait_input(struct tcp_pcb *pcb);

/**
 * The initial input processing of TCP. It verifies the TCP header, demultiplexes
 * the segment between the PCBs and passes it on to tcp_process(), which implements
 * the TCP finite state machine. This function is called by the IP layer (in
 * ip_input()).
 	TCP层的总输入函数，由IP层的ip_input调用，根据数据包查找相应tcp控制块以及相应函数	
 	tcp_timewait_input、tcp_listen_input、tcp_process进行处理(即寻找到一个合适的接口)
 	如果调用的是前2个，tcp_input在这两个函数返回后就结束了
 	但如果调用的是第3个，则tcp_input还要进行许多相应处理
 *
 * @param p received TCP segment to process (p->payload pointing to the IP header)
 * @param inp network interface on which this segment was received
 */
void
tcp_input(struct pbuf *p, struct netif *inp)
{
  struct tcp_pcb *pcb, *prev;
  struct tcp_pcb_listen *lpcb;
#if SO_REUSE
  struct tcp_pcb *lpcb_prev = NULL;
  struct tcp_pcb_listen *lpcb_any = NULL;
#endif /* SO_REUSE */
	u8_t hdrlen;
	err_t err;

	PERF_START;

	TCP_STATS_INC(tcp.recv);
	snmp_inc_tcpinsegs();

	// 略过IP包头，提取TCP头
	iphdr = (struct ip_hdr *)p->payload;
	tcphdr = (struct tcp_hdr *)((u8_t *)p->payload + IPH_HL(iphdr) * 4);

#if TCP_INPUT_DEBUG
  tcp_debug_print(tcphdr);
#endif

	// 移动pbuf结构中的数据包指针，使指向TCP头
	if (pbuf_header(p, -((s16_t)(IPH_HL(iphdr) * 4))) || (p->tot_len < sizeof(struct tcp_hdr))) 
	{
		/* drop short packets */
		LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_input: short packet (%"U16_F" bytes) discarded\n", p->tot_len));
		TCP_STATS_INC(tcp.lenerr);
		TCP_STATS_INC(tcp.drop);
		snmp_inc_tcpinerrs();
		pbuf_free(p);
		return;
	}

	// 不处理输入的广播包
	if (ip_addr_isbroadcast(&current_iphdr_dest, inp) || ip_addr_ismulticast(&current_iphdr_dest)) 
    {
    	TCP_STATS_INC(tcp.proterr);
    	TCP_STATS_INC(tcp.drop);
    	snmp_inc_tcpinerrs();
    	pbuf_free(p);
    	return;
	}

#if CHECKSUM_CHECK_TCP
	// 验证TCP校验和
	if (inet_chksum_pseudo(p, ip_current_src_addr(), ip_current_dest_addr(),
	  IP_PROTO_TCP, p->tot_len) != 0) {
	  LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_input: packet discarded due to failing checksum 0x%04"X16_F"\n",
	    inet_chksum_pseudo(p, ip_current_src_addr(), ip_current_dest_addr(),
	  IP_PROTO_TCP, p->tot_len)));
#if TCP_DEBUG
    tcp_debug_print(tcphdr);
#endif /* TCP_DEBUG */
    TCP_STATS_INC(tcp.chkerr);
    TCP_STATS_INC(tcp.drop);
    snmp_inc_tcpinerrs();
    pbuf_free(p);
    return;
  	}
#endif

	// 继续移动pbuf结构中的数据包指针，使指向TCP数据
	hdrlen = TCPH_HDRLEN(tcphdr);
	if(pbuf_header(p, -(hdrlen * 4))){
	/* drop short packets */
	LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_input: short packet\n"));
	TCP_STATS_INC(tcp.lenerr);
	TCP_STATS_INC(tcp.drop);
	snmp_inc_tcpinerrs();
	pbuf_free(p);
	return;
  	}

	// 网络字节序转主机字节序
	tcphdr->src = ntohs(tcphdr->src);				// 源端口
	tcphdr->dest = ntohs(tcphdr->dest);				// 目的端口
	seqno = tcphdr->seqno = ntohl(tcphdr->seqno);	// 序号
	ackno = tcphdr->ackno = ntohl(tcphdr->ackno);	// 确认序号
	tcphdr->wnd = ntohs(tcphdr->wnd);				// 窗口大小

	flags = TCPH_FLAGS(tcphdr);						// 6位标志位
	tcplen = p->tot_len + ((flags & (TCP_FIN | TCP_SYN)) ? 1 : 0);	// 数据包中数据的长度，对于FIN或SYN标志置1的数据包，该长度要加1

	// 以下就是对接收到的数据包进行分类处理，也就是寻找合适的接口，根据addr，port
	// 首先在tcp_active_pcbs 中找，有没有匹配的tcp_pcb，
	prev = NULL;
  	for(pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next) 
  	{
	    LWIP_ASSERT("tcp_input: active pcb->state != CLOSED", pcb->state != CLOSED);
	    LWIP_ASSERT("tcp_input: active pcb->state != TIME-WAIT", pcb->state != TIME_WAIT);
	    LWIP_ASSERT("tcp_input: active pcb->state != LISTEN", pcb->state != LISTEN);
    	if (pcb->remote_port == tcphdr->src &&
		pcb->local_port == tcphdr->dest &&
		ip_addr_cmp(&(pcb->remote_ip), &current_iphdr_src) &&
		ip_addr_cmp(&(pcb->local_ip), &current_iphdr_dest)) 
		{

			/* Move this PCB to the front of the list so that subsequent
			 lookups will be faster (we exploit locality in TCP segment
			 arrivals). */
			 // 找到匹配的接口之后,将该tcp_pcb从tcp_active_pcbs链表池中取出，退出循环往下运行，这时pcb != NULL
			LWIP_ASSERT("tcp_input: pcb->next != pcb (before cache)", pcb->next != pcb);
			if (prev != NULL) 
			{
				prev->next = pcb->next;
				pcb->next = tcp_active_pcbs;
				tcp_active_pcbs = pcb;
			}
			LWIP_ASSERT("tcp_input: pcb->next != pcb (after cache)", pcb->next != pcb);
			break;
		}
	    prev = pcb;
  	}

	// 如果在tcp_active_pcbs中没有找到，继续在tcp_tw_pcbs 和tcp_listen_pcbs中找
  	if (pcb == NULL) 
	{
	    // 在tcp_tw_pcbs中找
	    for(pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next) 
		{
			LWIP_ASSERT("tcp_input: TIME-WAIT pcb->state == TIME-WAIT", pcb->state == TIME_WAIT);
			if (pcb->remote_port == tcphdr->src &&
			pcb->local_port == tcphdr->dest &&
			ip_addr_cmp(&(pcb->remote_ip), &current_iphdr_src) &&
			ip_addr_cmp(&(pcb->local_ip), &current_iphdr_dest)) 
			{
				/* We don't really care enough to move this PCB to the front
				   of the list since we are not very likely to receive that
				   many segments for connections in TIME-WAIT. */
				LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_input: packed for TIME_WAITing connection.\n"));
				// 进入TIME_WAIT状态处理，处理完直接这里返回不再往下运行
				tcp_timewait_input(pcb);
				pbuf_free(p);
				return;
			}
	    }

	    // 在tcp_listen_pcbs中找
	    prev = NULL;
	    for(lpcb = tcp_listen_pcbs.listen_pcbs; lpcb != NULL; lpcb = lpcb->next) 
		{
			// 判断端口是否匹配
	      	if (lpcb->local_port == tcphdr->dest) 
			{
#if SO_REUSE
		        if (ip_addr_cmp(&(lpcb->local_ip), &current_iphdr_dest)) 
				{
		          /* found an exact match */
		          break;
		        } 
				else if(ip_addr_isany(&(lpcb->local_ip))) 
				{
		          /* found an ANY-match */
		          lpcb_any = lpcb;
		          lpcb_prev = prev;
		        }
#else /* SO_REUSE */
				// 然后判断IP是否匹配，或者是IPADDR_ANY接收任何IP
		        if (ip_addr_cmp(&(lpcb->local_ip), &current_iphdr_dest) ||
		            ip_addr_isany(&(lpcb->local_ip))) 
		        {
		        	 // 找到匹配的接口之后退出循环往下运行，这时lpcb != NULL
		          	break;
		        }
#endif /* SO_REUSE */
	      	}
	      	prev = (struct tcp_pcb *)lpcb;
	    }
#if SO_REUSE
    /* first try specific local IP */
    if (lpcb == NULL) {
      /* only pass to ANY if no specific local IP has been found */
      lpcb = lpcb_any;
      prev = lpcb_prev;
    }
#endif /* SO_REUSE */
		// 这里是判断在tcp_listen_pcbs中是否找到
	    if (lpcb != NULL) 
		{
			/* Move this PCB to the front of the list so that subsequent
			 lookups will be faster (we exploit locality in TCP segment
			 arrivals). */
			if (prev != NULL) {
			((struct tcp_pcb_listen *)prev)->next = lpcb->next;
			      /* our successor is the remainder of the listening list */
			lpcb->next = tcp_listen_pcbs.listen_pcbs;
			      /* put this listening pcb at the head of the listening list */
			tcp_listen_pcbs.listen_pcbs = lpcb;
			}

			LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_input: packed for LISTENing connection.\n"));
			// 进入LISTEN	状态处理，处理完直接这里返回不再往下运行
			tcp_listen_input(lpcb);
			pbuf_free(p);
			return;
	    }
  	}

#if TCP_INPUT_DEBUG
  LWIP_DEBUGF(TCP_INPUT_DEBUG, ("+-+-+-+-+-+-+-+-+-+-+-+-+-+- tcp_input: flags "));
  tcp_debug_print_flags(TCPH_FLAGS(tcphdr));
  LWIP_DEBUGF(TCP_INPUT_DEBUG, ("-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n"));
#endif /* TCP_INPUT_DEBUG */

	// 如果在tcp_active_pcbs中找到了，则经过处理后进入tcp_process
  	if (pcb != NULL) 
	{
    	/* The incoming segment belongs to a connection. */
#if TCP_INPUT_DEBUG
#if TCP_DEBUG
    tcp_debug_print_state(pcb->state);
#endif /* TCP_DEBUG */
#endif /* TCP_INPUT_DEBUG */

	    /* Set up a tcp_seg structure. */
	    inseg.next = NULL;		// 关闭报文段队列功能
	    inseg.len = p->tot_len;	// 设置报文段数据总长
	    inseg.p = p;			// 设置报文段数据链表头指针
	    inseg.tcphdr = tcphdr;	// 设置报文段的TCP头

	    recv_data = NULL;
	    recv_flags = 0;			// 这个记录当前控制块状态的标志首先在这里被清0

	    // tcp_pcb的refused_data指针上是否还记录有尚未往上层递交的数据
	    if (pcb->refused_data != NULL) 
		{
			// 有的话回调用户recv函数接收未递交的数据
			LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_input: notify kept packet\n"));
			TCP_EVENT_RECV(pcb, pcb->refused_data, ERR_OK, err);
			// 判断处理recv函数的处理结果，成功refused_data指针清空，继续往下执行tcp_process
			if (err == ERR_OK) 
			{
			    pcb->refused_data = NULL;
			} 
			// 失败意味着tcp_pcb都被占用满，丢弃接收包不再处理，直接返回
			else if ((err == ERR_ABRT) || (tcplen > 0)) 
			{
    			/* if err == ERR_ABRT, 'pcb' is already deallocated */
    			/* Drop incoming packets because pcb is "full" (only if the incoming
    			   segment contains data). */
    			LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_input: drop incoming packets, because pcb is \"full\"\n"));
    			TCP_STATS_INC(tcp.drop);
    			snmp_inc_tcpinerrs();
    			pbuf_free(p);
    			return;
			}
	    }
    	tcp_input_pcb = pcb;	// 记录处理当前报文的控制块

		// 这里就是进入tcp_process处理接收包环节了
	    err = tcp_process(pcb);
	    // 若返回值为ERR_ABRT，说明控制块已经被完全删除(tcp_abort()),什么也不需要做
	    if (err != ERR_ABRT) 
		{
			// 返回值不为ERR_ABRT时，判断报文处理的各种结果
			if (recv_flags & TF_RESET) 			// 接收到对方的复位报文
			{
				/* TF_RESET means that the connection was reset by the other
				   end. We then call the error callback to inform the
				   application that the connection is dead before we
				   deallocate the PCB. */
				// 回调用户的errf函数
				TCP_EVENT_ERR(pcb->errf, pcb->callback_arg, ERR_RST);
				// 删除控制块
				tcp_pcb_remove(&tcp_active_pcbs, pcb);
				// 释放控制块空间
				memp_free(MEMP_TCP_PCB, pcb);
			} 
			else if (recv_flags & TF_CLOSED) 	// 双方连接成功断开
			{
				/* The connection has been closed and we will deallocate the
				   PCB. */
				// 删除控制块
				tcp_pcb_remove(&tcp_active_pcbs, pcb);
				// 释放控制块空间
				memp_free(MEMP_TCP_PCB, pcb);
			} 
			else 								
			{
				err = ERR_OK;
				/* If the application has registered a "sent" function to be
				   called when new send buffer space is available, we call it
				   now. */
				if (pcb->acked > 0) 		// 如果有数据被确认		
				{
					// 回调用户的send函数
				  	TCP_EVENT_SENT(pcb, pcb->acked, err);
				  	if (err == ERR_ABRT) {
				    goto aborted;
				  	}
				}

				if (recv_data != NULL)			// 如果有数据被接收到 
				{
					LWIP_ASSERT("pcb->refused_data == NULL", pcb->refused_data == NULL);
					if (pcb->flags & TF_RXCLOSED) 
					{
						/* received data although already closed -> abort (send RST) to
						   notify the remote host that not all data has been processed */
						pbuf_free(recv_data);
						tcp_abort(pcb);
						goto aborted;
					}
					if (flags & TCP_PSH) 		// 如果TCP标志位中带有PSH
					{
						// 设置pbuf首部的flag字段
						recv_data->flags |= PBUF_FLAG_PUSH;
					}

					// 回调用户的recv函数，接收递交上去的TCP数据recv_data
					TCP_EVENT_RECV(pcb, recv_data, ERR_OK, err);
					// 判断返回值，如果是ERR_ABRT，则丢弃，返回
					if (err == ERR_ABRT) {
					goto aborted;
					}

					// 除此之外，如果返回值是失败，将这部分尚未往上递交的数据暂存到refused_data指针中
					if (err != ERR_OK) {
					pcb->refused_data = recv_data;
					LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_input: keep incoming packet, because pcb is \"full\"\n"));
					}
				}

				/* If a FIN segment was received, we call the callback
				   function with a NULL buffer to indicate EOF. */
				if (recv_flags & TF_GOT_FIN)	// 如果收到对方的FIN请求 
				{
					/* correct rcv_wnd as the application won't call tcp_recved()
					 for the FIN's seqno */
					// 纠正接收窗口 
					if (pcb->rcv_wnd != TCP_WND) {
					pcb->rcv_wnd++;
					}
					// 用一个NULL指针回调用户的recv函数，通过这种方式用户程序可以知道对方的关闭请求
					TCP_EVENT_CLOSED(pcb, err);
					if (err == ERR_ABRT) {
					goto aborted;
					}
				}

				tcp_input_pcb = NULL;		// 当前报文到此处理完毕，清空当前报文的控制块
				/* Try to send something out. */
				tcp_output(pcb);			// 输出报文
#if TCP_INPUT_DEBUG
#if TCP_DEBUG
			tcp_debug_print_state(pcb->state);
#endif /* TCP_DEBUG */
#endif /* TCP_INPUT_DEBUG */
			}
    	}
	    /* Jump target if pcb has been aborted in a callback (by calling tcp_abort()).
	       Below this line, 'pcb' may not be dereferenced! */
aborted:
	    tcp_input_pcb = NULL;
	    recv_data = NULL;

	    /* give up our reference to inseg.p */
	    if (inseg.p != NULL)
	    {
	      pbuf_free(inseg.p);
	      inseg.p = NULL;
	    }
  	} 
	else 
	{
	    // 如果在3张链表里都未找到匹配的pcb，则调用tcp_rst向源主机发送一个TCP复位数据包
	    LWIP_DEBUGF(TCP_RST_DEBUG, ("tcp_input: no PCB match found, resetting.\n"));
	    if (!(TCPH_FLAGS(tcphdr) & TCP_RST)) {
	      TCP_STATS_INC(tcp.proterr);
	      TCP_STATS_INC(tcp.drop);
	      tcp_rst(ackno, seqno + tcplen,
	        ip_current_dest_addr(), ip_current_src_addr(),
	        tcphdr->dest, tcphdr->src);
	    }
	    pbuf_free(p);
  	}

  LWIP_ASSERT("tcp_input: tcp_pcbs_sane()", tcp_pcbs_sane());
  PERF_STOP("tcp_input");
}

/**
 * Called by tcp_input() when a segment arrives for a listening
 * connection (from tcp_input()).
 *	tcp_pcb等到的是SYN数据包时进入本函数
 	本函数是处于LISTEN状态的控制块处理输入报文函数
 * @param pcb the tcp_pcb_listen for which a segment arrived
 * @return ERR_OK if the segment was processed
 *         another err_t on error
 *
 * @note the return value is not (yet?) used in tcp_input()
 * @note the segment which arrived is saved in global variables, therefore only the pcb
 *       involved is passed as a parameter to this function
 */
static err_t
tcp_listen_input(struct tcp_pcb_listen *pcb)
{
	struct tcp_pcb *npcb;
	err_t rc;

	/* In the LISTEN state, we check for incoming SYN segments,
	 creates a new PCB, and responds with a SYN|ACK. */
  	if (flags & TCP_ACK) 		// TCP_ACK - 处于listen状态的pcb只能响应SYN握手包，所以对非握手包返回一个tcp_rst
  	{
	    /* For incoming segments with the ACK flag set, respond with a
	       RST. */
	    LWIP_DEBUGF(TCP_RST_DEBUG, ("tcp_listen_input: ACK in LISTEN, sending reset\n"));
	    tcp_rst(ackno + 1, seqno + tcplen,
	      ip_current_dest_addr(), ip_current_src_addr(),
	      tcphdr->dest, tcphdr->src);
  	} 
	else if (flags & TCP_SYN) 	// TCP_SYN - 处于listen状态的服务器端等到了SYN握手包
	{
    	LWIP_DEBUGF(TCP_DEBUG, ("TCP connection request %"U16_F" -> %"U16_F".\n", tcphdr->src, tcphdr->dest));
#if TCP_LISTEN_BACKLOG
    if (pcb->accepts_pending >= pcb->backlog) {
      LWIP_DEBUGF(TCP_DEBUG, ("tcp_listen_input: listen backlog exceeded for port %"U16_F"\n", tcphdr->dest));
      return ERR_ABRT;
    }
#endif /* TCP_LISTEN_BACKLOG */
		// 建立一个新的tcp_pcb，因为处于tcp_listen_pcbs链表上的pcb是tcp_pcb_listen结构的，而其他链表上的pcb是tcp_pcb结构
	    npcb = tcp_alloc(pcb->prio);
	    /* If a new PCB could not be created (probably due to lack of memory),
	       we don't do anything, but rely on the sender will retransmit the
	       SYN at a time when we have more memory available. */
	    if (npcb == NULL) 
		{
	      LWIP_DEBUGF(TCP_DEBUG, ("tcp_listen_input: could not allocate PCB\n"));
	      TCP_STATS_INC(tcp.memerr);
	      return ERR_MEM;
	    }
#if TCP_LISTEN_BACKLOG
    pcb->accepts_pending++;
#endif /* TCP_LISTEN_BACKLOG */
	    // 为这个新建的tcp_pcb填充成员
	    ip_addr_copy(npcb->local_ip, current_iphdr_dest);
	    npcb->local_port = pcb->local_port;
	    ip_addr_copy(npcb->remote_ip, current_iphdr_src);
	    npcb->remote_port = tcphdr->src;
	    npcb->state = SYN_RCVD;			// SYN_RCVD - 表示进入了收到SYN状态
	    npcb->rcv_nxt = seqno + 1;		// 期望接收到的下一个序号，注意加1
	    npcb->rcv_ann_right_edge = npcb->rcv_nxt;
	    npcb->snd_wnd = tcphdr->wnd;	// 设置发送窗口大小
	    npcb->ssthresh = npcb->snd_wnd;
	    npcb->snd_wl1 = seqno - 1;/* initialise to seqno-1 to force window update */
	    npcb->callback_arg = pcb->callback_arg;
#if LWIP_CALLBACK_API
	#ifdef sys_bios_ind_sdk
	 	npcb->acceptdata = pcb->acceptdata;
	#else
     	npcb->accept = pcb->accept;
    #endif
#endif /* LWIP_CALLBACK_API */
	    /* inherit socket options */
	    npcb->so_options = pcb->so_options & SOF_INHERITED;
	    //将这个设置好的tcp_pcb注册到tcp_active_pcbs链表中去
	    TCP_REG(&tcp_active_pcbs, npcb);

	    // 从收到的SYN握手包中提取TCP头中选项字段的值，并设置到自己的tcp_pcb
	    tcp_parseopt(npcb);
#if TCP_CALCULATE_EFF_SEND_MSS
    	npcb->mss = tcp_eff_send_mss(npcb->mss, &(npcb->remote_ip));
#endif /* TCP_CALCULATE_EFF_SEND_MSS */

	    snmp_inc_tcppassiveopens();

	    // 回复带有SYN和ACK标志的握手数据包
	    rc = tcp_enqueue_flags(npcb, TCP_SYN | TCP_ACK);
	    if (rc != ERR_OK) {
	      tcp_abandon(npcb, 0);
	      return rc;
	    }

		// 真正发送回复包在这里
	    return tcp_output(npcb);
  	}
  	return ERR_OK;
}

/**
 * Called by tcp_input() when a segment arrives for a connection in
 * TIME_WAIT.
 *	TIME_WAIT状态是指tcp关闭一个连接后会进入的状态
 	本函数是处于TIME_WAIT状态的控制块处理输入报文的函数
 * @param pcb the tcp_pcb for which a segment arrived
 *
 * @note the segment which arrived is saved in global variables, therefore only the pcb
 *       involved is passed as a parameter to this function
 */
static err_t
tcp_timewait_input(struct tcp_pcb *pcb)
{
  /* RFC 1337: in TIME_WAIT, ignore RST and ACK FINs + any 'acceptable' segments */
  /* RFC 793 3.9 Event Processing - Segment Arrives:
   * - first check sequence number - we skip that one in TIME_WAIT (always
   *   acceptable since we only send ACKs)
   * - second check the RST bit (... return) */
  	if (flags & TCP_RST)  	// TCP_RST - 复位连接，直接返回
  	{
    return ERR_OK;
  	}
  	/* - fourth, check the SYN bit, */
  	if (flags & TCP_SYN) 	// TCP_SYN - 同步序列号，返回一个ACK
	{
	    /* If an incoming segment is not acceptable, an acknowledgment
	       should be sent in reply */
	    if (TCP_SEQ_BETWEEN(seqno, pcb->rcv_nxt, pcb->rcv_nxt+pcb->rcv_wnd)) 
		{
	      /* If the SYN is in the window it is an error, send a reset */
	      tcp_rst(ackno, seqno + tcplen, ip_current_dest_addr(), ip_current_src_addr(),
	        tcphdr->dest, tcphdr->src);
	      return ERR_OK;
	    }
  	} 
	else if (flags & TCP_FIN)	// TCP_FIN - 重新启动定时器 	
	{
	    /* - eighth, check the FIN bit: Remain in the TIME-WAIT state.
	         Restart the 2 MSL time-wait timeout.*/
	    pcb->tmr = tcp_ticks;
  	}

  	if ((tcplen > 0))  
	{
    	/* Acknowledge data, FIN or out-of-window SYN */
    	pcb->flags |= TF_ACK_NOW;
    	return tcp_output(pcb);
  	}
	
  	return ERR_OK;
}

/**
 * Implements the TCP state machine. Called by tcp_input. In some
 * states tcp_receive() is called to receive data. The tcp_seg
 * argument will be freed by the caller (tcp_input()) unless the
 * recv_data pointer in the pcb is set.
 *	除了LISTEN、TIME_WAIT其他所有状态的控制块，其输入报文的处理都在这里
 	该函数主要实现了TCP状态转换功能
 * @param pcb the tcp_pcb for which a segment arrived
 *
 * @note the segment which arrived is saved in global variables, therefore only the pcb
 *       involved is passed as a parameter to this function
 */
static err_t
tcp_process(struct tcp_pcb *pcb)
{
	struct tcp_seg *rseg;
	u8_t acceptable = 0;
	err_t err;

	err = ERR_OK;

	// 首先判断该报文是不是一个RST报文
	if (flags & TCP_RST) 
	{
		// 判断该RST报文是否合法
		if (pcb->state == SYN_SENT) 	// 第一种情况，连接处于SYN_SENT状态
		{
			if (ackno == pcb->snd_nxt) 	// 且报文中确认号与snd_nxt相等
			{
				acceptable = 1;
			}
		} 
		else 							// 第二种情况，其他状态下报文中的序列号在接收窗口内
		{
			if (TCP_SEQ_BETWEEN(seqno, pcb->rcv_nxt,pcb->rcv_nxt+pcb->rcv_wnd)) 
			{
				acceptable = 1;
			}
		}

		// 如果RST报文合法，则需要复位当前连接的控制块，非法则直接返回不做处理
		if (acceptable) 
		{
		  LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_process: Connection RESET\n"));
		  LWIP_ASSERT("tcp_input: pcb->state != CLOSED", pcb->state != CLOSED);
		  recv_flags |= TF_RESET;
		  pcb->flags &= ~TF_ACK_DELAY;
		  return ERR_RST;
		} 
		else 
		{
		  LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_process: unacceptable reset seqno %"U32_F" rcv_nxt %"U32_F"\n",
		   seqno, pcb->rcv_nxt));
		  LWIP_DEBUGF(TCP_DEBUG, ("tcp_process: unacceptable reset seqno %"U32_F" rcv_nxt %"U32_F"\n",
		   seqno, pcb->rcv_nxt));
		  return ERR_OK;
		}
	}

	// 处理握手报文SYN，在连接已经建立情况下，但还是接收到对方的握手包
	// 说明这可能是一个超时重发的握手包，直接向对方返回一个ACK即可
	if ((flags & TCP_SYN) && (pcb->state != SYN_SENT && pcb->state != SYN_RCVD)) 
	{ 
		/* Cope with new connection attempt after remote end crashed */
		tcp_ack_now(pcb);
		return ERR_OK;
	}

	// 复位控制块的活动计数器
  	if ((pcb->flags & TF_RXCLOSED) == 0) 
  	{
    	/* Update the PCB (in)activity timer unless rx is closed (see tcp_shutdown) */
    	pcb->tmr = tcp_ticks;
  	}

	// 保活报文计数器清0
  	pcb->keep_cnt_sent = 0;

	// 处理报文首部中的选项字段
  	tcp_parseopt(pcb);

  	// 根据不同的TCP状态执行相应动作
  	switch (pcb->state) 
  	{
	  	case SYN_SENT:	// 客户端发出SYN后，就处于该状态等待服务器返回SYN+ACK
			LWIP_DEBUGF(TCP_INPUT_DEBUG, ("SYN-SENT: ackno %"U32_F" pcb->snd_nxt %"U32_F" unacked %"U32_F"\n", ackno,
			 pcb->snd_nxt, ntohl(pcb->unacked->tcphdr->seqno)));
			// 如果收到的是SYN+ACK，且序列号正确
			if ((flags & TCP_ACK) && (flags & TCP_SYN) && ackno == ntohl(pcb->unacked->tcphdr->seqno) + 1) 
	        {
				pcb->snd_buf++;		// SYN被返回的ACK确认，该报文占用1个字节，所以可用的发送空间加1字节
				pcb->rcv_nxt = seqno + 1;		// 期望接收的下一个序号，即接收端向发送端ACK报文中的确认号
				pcb->rcv_ann_right_edge = pcb->rcv_nxt;	// 初始化通告窗口的右边界值
				pcb->lastack = ackno;			// 更新接收到的最大确认序号
				pcb->snd_wnd = tcphdr->wnd;		// 发送窗口设置为接收窗口大小
				pcb->snd_wl1 = seqno - 1; 		// 上次窗口更新时收到的数据序号
				pcb->state = ESTABLISHED;		// 进入ESTABLISHED状态

#if TCP_CALCULATE_EFF_SEND_MSS
      			pcb->mss = tcp_eff_send_mss(pcb->mss, &(pcb->remote_ip));	// 设置最大报文段
#endif /* TCP_CALCULATE_EFF_SEND_MSS */

				/* Set ssthresh again after changing pcb->mss (already set in tcp_connect
				* but for the default value of pcb->mss) */
				pcb->ssthresh = pcb->mss * 10;	// 重设mss后，ssthresh值也要相应修改

				pcb->cwnd = ((pcb->cwnd == 1) ? (pcb->mss * 2) : pcb->mss);	// 初始化阻塞窗口
				LWIP_ASSERT("pcb->snd_queuelen > 0", (pcb->snd_queuelen > 0));
				--pcb->snd_queuelen;			// SYN被返回的ACK确认，所以占用的pbuf个数减1
				LWIP_DEBUGF(TCP_QLEN_DEBUG, ("tcp_process: SYN-SENT --queuelen %"U16_F"\n", (u16_t)pcb->snd_queuelen));
				rseg = pcb->unacked;			// 在发送了未收到确认的数据段队列中删除SYN报文
				pcb->unacked = rseg->next;
				
				if(pcb->unacked == NULL)		// 如果未确认的数据段队列为空，则停止重传定时器
				pcb->rtime = -1;
				else 							// 如果队列中还有报文，则复位重传定时器和重传次数
				{
				pcb->rtime = 0;
				pcb->nrtx = 0;
				}

				tcp_seg_free(rseg);				// 释放取下的SYN报文段空间

				/* Call the user specified function to call when sucessfully
				* connected. */
				TCP_EVENT_CONNECTED(pcb, ERR_OK, err);	// 回调用户的connect函数
				if (err == ERR_ABRT) {
				return ERR_ABRT;
				}
				tcp_ack_now(pcb);				// 向服务器返回ACK，三次握手结束
				}
				// 如果只收到对方的ACK却没有SYN，则向对方返回RST
				else if (flags & TCP_ACK) {
				/* send a RST to bring the other side in a non-synchronized state. */
				tcp_rst(ackno, seqno + tcplen, ip_current_dest_addr(), ip_current_src_addr(),
				tcphdr->dest, tcphdr->src);
				}
				break;
		case SYN_RCVD:	// 服务器发送SYN+ACK后，就处于该状态，等待客户端返回ACK
			// 如果收到ACK，也就是三次握手的最后一个报文
			if (flags & TCP_ACK) 
			{
				// 如果ACK合法
				if (TCP_SEQ_BETWEEN(ackno, pcb->lastack+1, pcb->snd_nxt)) 
				{
					u16_t old_cwnd;
					pcb->state = ESTABLISHED;	// 进入ESTABLISHED状态
					LWIP_DEBUGF(TCP_DEBUG, ("TCP connection established %"U16_F" -> %"U16_F".\n", inseg.tcphdr->src, inseg.tcphdr->dest));
#if LWIP_CALLBACK_API
		#ifdef sys_bios_ind_sdk
		 LWIP_ASSERT("pcb->accept != NULL", pcb->acceptdata != NULL);
		#else
         LWIP_ASSERT("pcb->accept != NULL", pcb->accept != NULL);
        #endif
#endif
					/* Call the accept function. */
					TCP_EVENT_ACCEPT(pcb, ERR_OK, err);		// 回调用户的accept函数
					if (err != ERR_OK) 						// 如果accept函数返回错误，则关闭当前连接
					{
					  /* If the accept function returns with an error, we abort
					   * the connection. */
					  /* Already aborted? */
					  if (err != ERR_ABRT) {
					    tcp_abort(pcb);
					  }
					  return ERR_ABRT;
					}
					old_cwnd = pcb->cwnd;		// 保存旧的阻塞窗口
					/* If there was any data contained within this ACK,
					 * we'd better pass it on to the application as well. */
					tcp_receive(pcb);			// 调用函数处理报文中的数据 

					// 如果本地有未确认数据被报文中的ACK确认
					if (pcb->acked != 0) 		
					{
					  pcb->acked--;				// 调整确认的字节数，因为SYN报文占用1个字节，所以减1
					}
			
			        pcb->cwnd = ((old_cwnd == 1) ? (pcb->mss * 2) : pcb->mss);	// 初始化阻塞窗口

					// 如果在上面的tcp_receive处理时设置了关闭连接标志
			        if (recv_flags & TF_GOT_FIN) 
					{
			          tcp_ack_now(pcb);			// 回复ACK，响应对方的FIN握手标志
			          pcb->state = CLOSE_WAIT;	// 进入CLOSE_WAIT状态
			        }
			  	} 
				else 
				{
			        // 对于不合法的ACK，则返回一个RST
			        tcp_rst(ackno, seqno + tcplen, ip_current_dest_addr(), ip_current_src_addr(),tcphdr->dest, tcphdr->src);
      			}
    		}
			// 如果收到客户端重复SYN握手包，说明SYN+ACK包丢失，需要重传
			else if ((flags & TCP_SYN) && (seqno == pcb->rcv_nxt - 1)) 
			{
      			/* Looks like another copy of the SYN - retransmit our SYN-ACK */
      			tcp_rexmit(pcb);
    		}
    		break;
  		case CLOSE_WAIT:	// 服务器处于半关闭状态，会一直等待上层应用执行关闭指令，并将状态变为LASK_ACK
    	/* FALLTHROUGH */
  		case ESTABLISHED:	// 连接双方都处于稳定状态
    		tcp_receive(pcb);		// 调用函数处理报文中的数据

			// 如果在上面的tcp_receive处理时设置了关闭连接标志
    		if (recv_flags & TF_GOT_FIN) 
			{ /* passive close */
      			tcp_ack_now(pcb);			// 回复ACK，响应对方的FIN握手标志
      			pcb->state = CLOSE_WAIT;	// 进入CLOSE_WAIT状态
    		}
    		break;
  		case FIN_WAIT_1:	// 上层应用主动执行关闭指令，发送FIN后处于该状态
    		tcp_receive(pcb);	// 调用函数处理报文中的数据

			// 如果在上面的tcp_receive处理时设置了关闭连接标志，即收到FIN握手
    		if (recv_flags & TF_GOT_FIN) 
			{
				// 如果该报文同时包含一个合法ACK
	      		if ((flags & TCP_ACK) && (ackno == pcb->snd_nxt)) 
				{
			        LWIP_DEBUGF(TCP_DEBUG,
			          ("TCP connection closed: FIN_WAIT_1 %"U16_F" -> %"U16_F".\n", inseg.tcphdr->src, inseg.tcphdr->dest));
			        tcp_ack_now(pcb);			// 回复ACK
			        tcp_pcb_purge(pcb);			// 清除该连接中的所有现存数据
			        TCP_RMV(&tcp_active_pcbs, pcb);	// 从tcp_active_pcbs链表中删除该控制块
			        pcb->state = TIME_WAIT;		// 跳过FIN_WAIT_2状态，直接进入TIME_WAIT状态
			        TCP_REG(&tcp_tw_pcbs, pcb);	// 将该控制块加入tcp_tw_pcbs链表
	      		} 
				// 如果该报文不含ACK，即表示双方同时执行了关闭连接操作
				else 
				{
	        		tcp_ack_now(pcb);		// 返回ACK
	        		pcb->state = CLOSING;	// 进入CLOSING状态
	      		}
	    	} 
			// 如果只收到有效的ACK
			else if ((flags & TCP_ACK) && (ackno == pcb->snd_nxt)) 
			{
      			pcb->state = FIN_WAIT_2;	// 进入FIN_WAIT_2状态
		    }
		    break;
		case FIN_WAIT_2:	// 主动关闭，发送FIN握手且收到ACK后处于该状态
			tcp_receive(pcb);	// 调用函数处理报文中的数据

			// 如果在上面的tcp_receive处理时设置了关闭连接标志，即收到FIN握手
			if (recv_flags & TF_GOT_FIN) 
			{
				LWIP_DEBUGF(TCP_DEBUG, ("TCP connection closed: FIN_WAIT_2 %"U16_F" -> %"U16_F".\n", inseg.tcphdr->src, inseg.tcphdr->dest));
				tcp_ack_now(pcb);		// 回复ACK
				tcp_pcb_purge(pcb);		// 清除该连接中的所有现存数据
				TCP_RMV(&tcp_active_pcbs, pcb);	// 从tcp_active_pcbs链表中删除该控制块
				pcb->state = TIME_WAIT;	// 进入TIME_WAIT状态
				TCP_REG(&tcp_tw_pcbs, pcb);		// 将该控制块加入tcp_tw_pcbs链表
			}
			break;
		case CLOSING:		// 双方同时执行主动关闭，处于该状态
			tcp_receive(pcb);	// 调用函数处理报文中的数据

			// 如果收到合法ACK
			if (flags & TCP_ACK && ackno == pcb->snd_nxt) 
			{
				LWIP_DEBUGF(TCP_DEBUG, ("TCP connection closed: CLOSING %"U16_F" -> %"U16_F".\n", inseg.tcphdr->src, inseg.tcphdr->dest));
				tcp_pcb_purge(pcb);		// 清除该连接中的所有现存数据
				TCP_RMV(&tcp_active_pcbs, pcb);	// 从tcp_active_pcbs链表中删除该控制块
				pcb->state = TIME_WAIT;	// 进入TIME_WAIT状态
				TCP_REG(&tcp_tw_pcbs, pcb);		// 将该控制块加入tcp_tw_pcbs链表
			}
			break;
		case LAST_ACK:		// 服务器在执行被动关闭时，发送完FIN，等待ACK时处于该状态
			tcp_receive(pcb);	// 调用函数处理报文中的数据

			// 如果收到合法ACK
			if (flags & TCP_ACK && ackno == pcb->snd_nxt) 
			{
				LWIP_DEBUGF(TCP_DEBUG, ("TCP connection closed: LAST_ACK %"U16_F" -> %"U16_F".\n", inseg.tcphdr->src, inseg.tcphdr->dest));
				/* bugfix #21699: don't set pcb->state to CLOSED here or we risk leaking segments */
				recv_flags |= TF_CLOSED;	// recv_flags设置为TF_CLOSED，由tcp_input函数对该控制块进行释放和清除
			}
			break;
		default:
			break;
  }
  return ERR_OK;
}

#if TCP_QUEUE_OOSEQ
/**
 * Insert segment into the list (segments covered with new one will be deleted)
 * 将输入报文段插入链表，并释放原有报文段的内存
 * Called from tcp_receive()
 */
static void
tcp_oos_insert_segment(struct tcp_seg *cseg, struct tcp_seg *next)
{
	struct tcp_seg *old_seg;

	// 如果插入报文段的TCP头中有FIN标志，意味着ooseq链表中后面所有报文段都作废
	if (TCPH_FLAGS(cseg->tcphdr) & TCP_FIN) 
	{
		/* received segment overlaps all following segments */
		tcp_segs_free(next);	// 直接释放原有报文段内存
		next = NULL;			// 丢弃后面所有报文段
	}
	else 
	{
		/* delete some following segments
		   oos queue may have segments with FIN flag */
		// 遍历ooseq链表中报文段，删除所有被插入报文段数据完全覆盖的报文段
		while (next && TCP_SEQ_GEQ((seqno + cseg->len),(next->tcphdr->seqno + next->len))) 
		{
			/* cseg with FIN already processed */
			// 如果被删除的报文段的TCP头中有FIN标志
			if (TCPH_FLAGS(next->tcphdr) & TCP_FIN) 
			{
				TCPH_SET_FLAG(cseg->tcphdr, TCP_FIN);	// 设置FIN标志到插入报文段的TCP头中
			}
			old_seg = next;
			next = next->next;
			tcp_seg_free(old_seg);						// 释放被完全覆盖的报文段
		}

		// 如果插入报文段和ooseq链表上原有报文段存在数据重叠
		// 注意，此处lwip源码可能又有误，正确应是if (next && TCP_SEQ_LT(seqno + cseg->len, next->tcphdr->seqno)) 
		if (next && TCP_SEQ_GT(seqno + cseg->len, next->tcphdr->seqno)) 
		{
			/* We need to trim the incoming segment. */
			cseg->len = (u16_t)(next->tcphdr->seqno - seqno);	// 对插入报文段尾部数据进行截断，去掉重叠部分
			pbuf_realloc(cseg->p, cseg->len);	//  因为数据被截断，pbuf中的参数需要相应调整
		}
	}
	cseg->next = next;	// 将插入报文段和ooseq链表后面的报文段衔接
}
#endif /* TCP_QUEUE_OOSEQ */

/**
 * Called by tcp_process. Checks if the given segment is an ACK for outstanding
 * data, and if so frees the memory of the buffered data. Next, is places the
 * segment on any of the receive queues (pcb->recved or pcb->ooseq). If the segment
 * is buffered, the pbuf is referenced by pbuf_ref so that it will not be freed until
 * i it has been removed from the buffer.
 *
 * If the incoming segment constitutes an ACK for a segment that was used for RTT
 * estimation, the RTT is estimated here as well.
 *
 * Called from tcp_process().
 */
static void
tcp_receive(struct tcp_pcb *pcb)
{
  	struct tcp_seg *next;
#if TCP_QUEUE_OOSEQ
  	struct tcp_seg *prev, *cseg;
#endif /* TCP_QUEUE_OOSEQ */
  	struct pbuf *p;
	s32_t off;
	s16_t m;
	u32_t right_wnd_edge;
	u16_t new_tot_len;
	int found_dupack = 0;	// 重复ack标志，置1表示是重复ack

	// 首先检测报文是否包含ACK标志
	if (flags & TCP_ACK) 
	{
		right_wnd_edge = pcb->snd_wnd + pcb->snd_wl2;

		// 有3种情况可以导致本地发送窗口更新
		if (TCP_SEQ_LT(pcb->snd_wl1, seqno)||								// snd_wl1小于新seqno
			(pcb->snd_wl1 == seqno && TCP_SEQ_LT(pcb->snd_wl2, ackno))||	// snd_wl1等于新seqno且snd_wl2小于新ackno，说明对方没有发送数据，只是在收到数据后发送一个确认
			(pcb->snd_wl2 == ackno && tcphdr->wnd > pcb->snd_wnd)) 			// snd_wl2等于新ackno且snd_wnd小于报文首部的窗口通告wnd
		{
		  	pcb->snd_wnd = tcphdr->wnd;		// 更新本地发送窗口大小
		  	pcb->snd_wl1 = seqno;			// 更新接收到的数据序号
		  	pcb->snd_wl2 = ackno;			// 更新接收到的应答序号
		  	if (pcb->snd_wnd > 0 && pcb->persist_backoff > 0) 	// 检测到非0窗口且探查开启
		  	{
		      pcb->persist_backoff = 0;		// 停止窗口探察
		  	}
		  	LWIP_DEBUGF(TCP_WND_DEBUG, ("tcp_receive: window update %"U16_F"\n", pcb->snd_wnd));
#if TCP_WND_DEBUG
    } else {
      if (pcb->snd_wnd != tcphdr->wnd) {
        LWIP_DEBUGF(TCP_WND_DEBUG, 
                    ("tcp_receive: no window update lastack %"U32_F" ackno %"
                     U32_F" wl1 %"U32_F" seqno %"U32_F" wl2 %"U32_F"\n",
                     pcb->lastack, ackno, pcb->snd_wl1, seqno, pcb->snd_wl2));
      }
#endif /* TCP_WND_DEBUG */
    	}

    /* (From Stevens TCP/IP Illustrated Vol II, p970.) Its only a
     * duplicate ack if:
     只有满足所有5个条件的ackno才会被看作一个重复的ack
     * 1) It doesn't ACK new data 		没有确认新数据
     * 2) length of received packet is zero (i.e. no payload) 	报文段中没有数据
     * 3) the advertised window hasn't changed 	本地发送窗口没有更新
     * 4) There is outstanding unacknowledged data (retransmission timer running)		本地有数据正等待被确认
     * 5) The ACK is == biggest ACK sequence number so far seen (snd_una)		ackno等于lastack
     * 
     * If it passes all five, should process as a dupack: 
     * a) dupacks < 3: do nothing 
     * b) dupacks == 3: fast retransmit 
     * c) dupacks > 3: increase cwnd 
     * 
     * If it only passes 1-3, should reset dupack counter (and add to
     * stats, which we don't do in lwIP)
     *
     * If it only passes 1, should reset dupack counter
     *
     */

    	/* Clause 1 */
		// 判断是否是一个重复的ACK
    	if (TCP_SEQ_LEQ(ackno, pcb->lastack)) 						// 如果ackno小于等于lastack，即没有确认新数据
		{
	      	pcb->acked = 0;		// 被当前报文确认的已发送长度清0
	      	/* Clause 2 */
	      	if (tcplen == 0) 										// 如果报文段中没有数据
		  	{
	        	/* Clause 3 */
	        	if (pcb->snd_wl2 + pcb->snd_wnd == right_wnd_edge)	// 本地发送窗口没有更新
				{
					/* Clause 4 */
					if (pcb->rtime >= 0) 							// 重传定时器正在运行，即本地有数据正等待被确认
					{
						/* Clause 5 */
						if (pcb->lastack == ackno) 					// 如果ackno等于lastack
						{
							// 此时可以确定这是一个重复的ack，说明报文发生了丢失
							found_dupack = 1;
							if (pcb->dupacks + 1 > pcb->dupacks)	// 该ack被重复收到的次数自增	
								++pcb->dupacks;
							if (pcb->dupacks > 3) 					// 如果该ack重复收到超过3次，说明发生了拥塞
							{
								/* Inflate the congestion window, but not if it means that
								the value overflows. */
								if ((u16_t)(pcb->cwnd + pcb->mss) > pcb->cwnd) 
								{
									pcb->cwnd += pcb->mss;
								}
							} 
							else if (pcb->dupacks == 3) 			// 如果该ack重复第3次收到，执行快速重传算法
							{
								/* Do fast retransmit */
								tcp_rexmit_fast(pcb);
							}
						}
					}
	        	}
	      	}
	      	// 如果没有确认新数据但又不属于重复ack
	      	if (!found_dupack) 
			{
	        	pcb->dupacks = 0;		// 将ack重复收到的次数清0
	      	}
	    } 

		// 如果是正常情况的ACK，lastack+1<=ackno<=snd_nxt
		else if (TCP_SEQ_BETWEEN(ackno, pcb->lastack+1, pcb->snd_nxt))
		{
			/* We come here when the ACK acknowledges new data. */

			/* Reset the "IN Fast Retransmit" flag, since we are no longer
			 in fast retransmit. Also reset the congestion window to the
			 slow start threshold. */
			// 如果控制块处于快速重传状态	，则关闭重传状态、拥塞功能		 
			if (pcb->flags & TF_INFR) 
			{
				pcb->flags &= ~TF_INFR;
				pcb->cwnd = pcb->ssthresh;
			}

			/* Reset the number of retransmissions. */
			pcb->nrtx = 0;								// 重传次数清0

			/* Reset the retransmission time-out. */
			pcb->rto = (pcb->sa >> 3) + pcb->sv;		// 复位重传超时时间

			/* Update the send buffer space. Diff between the two can never exceed 64K? */
			pcb->acked = (u16_t)(ackno - pcb->lastack);	// 设置acked字段为被确认的数据量

			pcb->snd_buf += pcb->acked;					// 释放可用的发送空间

			/* Reset the fast retransmit variables. */
			pcb->dupacks = 0;							// 将ack重复收到的次数清0
			pcb->lastack = ackno;						// 更新接收到的ackno

			/* Update the congestion control variables (cwnd and
			 ssthresh). */
			// 如果处于TCP连接已经建立状态，调整拥塞算法功能模块
			if (pcb->state >= ESTABLISHED) 
			{
				if (pcb->cwnd < pcb->ssthresh) 
				{
				  	if ((u16_t)(pcb->cwnd + pcb->mss) > pcb->cwnd) 
					{
				    	pcb->cwnd += pcb->mss;
				  	}
				  	LWIP_DEBUGF(TCP_CWND_DEBUG, ("tcp_receive: slow start cwnd %"U16_F"\n", pcb->cwnd));
				} 
				else 
				{
				  	u16_t new_cwnd = (pcb->cwnd + pcb->mss * pcb->mss / pcb->cwnd);
				  	if (new_cwnd > pcb->cwnd) 
					{
				    	pcb->cwnd = new_cwnd;
				  	}
				  	LWIP_DEBUGF(TCP_CWND_DEBUG, ("tcp_receive: congestion avoidance cwnd %"U16_F"\n", pcb->cwnd));
				}
			}
			LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_receive: ACK for %"U32_F", unacked->seqno %"U32_F":%"U32_F"\n",
			                            ackno,
			                            pcb->unacked != NULL?
			                            ntohl(pcb->unacked->tcphdr->seqno): 0,
			                            pcb->unacked != NULL?
			                            ntohl(pcb->unacked->tcphdr->seqno) + TCP_TCPLEN(pcb->unacked): 0));

			// 遍历unacked队列，将所有数据编号小于等于ackno的报文段移除
			while (pcb->unacked != NULL &&
			     TCP_SEQ_LEQ(ntohl(pcb->unacked->tcphdr->seqno) + TCP_TCPLEN(pcb->unacked), ackno)) 
			{
				LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_receive: removing %"U32_F":%"U32_F" from pcb->unacked\n",
				                              ntohl(pcb->unacked->tcphdr->seqno),
				                              ntohl(pcb->unacked->tcphdr->seqno) +
				                              TCP_TCPLEN(pcb->unacked)));
				// 将满足要求的报文从unacked链表取出
				next = pcb->unacked;
				pcb->unacked = pcb->unacked->next;

				LWIP_DEBUGF(TCP_QLEN_DEBUG, ("tcp_receive: queuelen %"U16_F" ... ", (u16_t)pcb->snd_queuelen));
				LWIP_ASSERT("pcb->snd_queuelen >= pbuf_clen(next->p)", (pcb->snd_queuelen >= pbuf_clen(next->p)));
				// 如果该ACK是针对本地发出的FIN而作出的，则acked字段减1，即不需要提交上层使知道FIN被对方成功接收
				if ((pcb->acked != 0) && ((TCPH_FLAGS(next->tcphdr) & TCP_FIN) != 0)) 
				{
				  	pcb->acked--;
				}

				pcb->snd_queuelen -= pbuf_clen(next->p);		// 释放被该报文占用的发送空间
				tcp_seg_free(next);								// 释放被该报文占用的tcp报文段

				LWIP_DEBUGF(TCP_QLEN_DEBUG, ("%"U16_F" (after freeing unacked)\n", (u16_t)pcb->snd_queuelen));
				if (pcb->snd_queuelen != 0) 
				{
				  LWIP_ASSERT("tcp_receive: valid queue length", pcb->unacked != NULL ||
				              pcb->unsent != NULL);
				}
			}

			// 当所有满足要求的报文段移除成功后，判断unacked队列是否为空
			if(pcb->unacked == NULL)
				pcb->rtime = -1;	// 若为空，关闭重传定时器
			else
				pcb->rtime = 0;		// 否则复位重传定时器

			pcb->polltmr = 0;		// 复位轮询定时器
    	}
		// 如果该ACK既不是重复ACK，又不是正常ACK，则acked字段清0
		else 
		{
      		/* Fix bug bug #21582: out of sequence ACK, didn't really ack anything */
      		pcb->acked = 0;
    	}

	    /* We go through the ->unsent list to see if any of the segments
	       on the list are acknowledged by the ACK. This may seem
	       strange since an "unsent" segment shouldn't be acked. The
	       rationale is that lwIP puts all outstanding segments on the
	       ->unsent list after a retransmission, so these segments may
	       in fact have been sent once. */
	    // 遍历unsent队列，将所有数据编号小于等于ackno的报文段移除
	    // 这是因为对于需要重传的报文段，lwip直接将它们挂在unsent队列上，所以收到的ACK可能是对已超时报文段的确认
	    while (pcb->unsent != NULL &&
	           TCP_SEQ_BETWEEN(ackno, ntohl(pcb->unsent->tcphdr->seqno) + TCP_TCPLEN(pcb->unsent), pcb->snd_nxt)) 
	    {
			LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_receive: removing %"U32_F":%"U32_F" from pcb->unsent\n",
			                            ntohl(pcb->unsent->tcphdr->seqno), ntohl(pcb->unsent->tcphdr->seqno) +
			                            TCP_TCPLEN(pcb->unsent)));
			// 将满足要求的报文从unsent链表取出
			next = pcb->unsent;
			pcb->unsent = pcb->unsent->next;
			LWIP_DEBUGF(TCP_QLEN_DEBUG, ("tcp_receive: queuelen %"U16_F" ... ", (u16_t)pcb->snd_queuelen));
			LWIP_ASSERT("pcb->snd_queuelen >= pbuf_clen(next->p)", (pcb->snd_queuelen >= pbuf_clen(next->p)));
			
			// 如果该ACK是针对本地发出的FIN而作出的，则acked字段减1，即不需要提交上层使知道FIN被对方成功接收
			if ((pcb->acked != 0) && ((TCPH_FLAGS(next->tcphdr) & TCP_FIN) != 0)) 
			{
				pcb->acked--;
			}
			pcb->snd_queuelen -= pbuf_clen(next->p);		// 释放被该报文占用的发送空间
			tcp_seg_free(next);								// 释放被该报文占用的tcp报文段
			LWIP_DEBUGF(TCP_QLEN_DEBUG, ("%"U16_F" (after freeing unsent)\n", (u16_t)pcb->snd_queuelen));
			if (pcb->snd_queuelen != 0) 
			{
			LWIP_ASSERT("tcp_receive: valid queue length",
			  pcb->unacked != NULL || pcb->unsent != NULL);
			}
	    }
	    /* End of ACK for new data processing. */

	    LWIP_DEBUGF(TCP_RTO_DEBUG, ("tcp_receive: pcb->rttest %"U32_F" rtseq %"U32_F" ackno %"U32_F"\n",
	                                pcb->rttest, pcb->rtseq, ackno));

	    /* RTT estimation calculations. This is done by checking if the
	       incoming segment acknowledges the segment we use to take a
	       round-trip time measurement. */
	    // RTT计算，暂略
	    if (pcb->rttest && TCP_SEQ_LT(pcb->rtseq, ackno)) 
		{
			/* diff between this shouldn't exceed 32K since this are tcp timer ticks
			 and a round-trip shouldn't be that long... */
			m = (s16_t)(tcp_ticks - pcb->rttest);

			LWIP_DEBUGF(TCP_RTO_DEBUG, ("tcp_receive: experienced rtt %"U16_F" ticks (%"U16_F" msec).\n",
			                          m, m * TCP_SLOW_INTERVAL));

			/* This is taken directly from VJs original code in his paper */
			m = m - (pcb->sa >> 3);
			pcb->sa += m;
			if (m < 0) {
			m = -m;
			}
			m = m - (pcb->sv >> 2);
			pcb->sv += m;
			pcb->rto = (pcb->sa >> 3) + pcb->sv;

			LWIP_DEBUGF(TCP_RTO_DEBUG, ("tcp_receive: RTO %"U16_F" (%"U16_F" milliseconds)\n",
			                          pcb->rto, pcb->rto * TCP_SLOW_INTERVAL));

			pcb->rttest = 0;
	    }
	}

	// 如果该输入报文还包含了数据，则要继续对数据进行处理
	if (tcplen > 0) 
	{
	    /* This code basically does three things:

	    +) If the incoming segment contains data that is the next
	    in-sequence data, this data is passed to the application. This
	    might involve trimming the first edge of the data. The rcv_nxt
	    variable and the advertised window are adjusted.

	    +) If the incoming segment has data that is above the next
	    sequence number expected (->rcv_nxt), the segment is placed on
	    the ->ooseq queue. This is done by finding the appropriate
	    place in the ->ooseq queue (which is ordered by sequence
	    number) and trim the segment in both ends if needed. An
	    immediate ACK is sent to indicate that we received an
	    out-of-sequence segment.

	    +) Finally, we check if the first segment on the ->ooseq queue
	    now is in sequence (i.e., if rcv_nxt >= ooseq->seqno). If
	    rcv_nxt > ooseq->seqno, we must trim the first edge of the
	    segment on ->ooseq before we adjust rcv_nxt. The data in the
	    segments that are now on sequence are chained onto the
	    incoming segment so that we only need to call the application
	    once.
	    */

	    /* First, we check if we must trim the first edge. We have to do
	       this if the sequence number of the incoming segment is less
	       than rcv_nxt, and the sequence number plus the length of the
	       segment is larger than rcv_nxt. */
	    /*    if (TCP_SEQ_LT(seqno, pcb->rcv_nxt)){
	          if (TCP_SEQ_LT(pcb->rcv_nxt, seqno + tcplen)) {*/
	    // 如果seqno+1 <= rcv_nxt <= seqno + tcplen-1，意味着收到的数据区域头部有无效数据，需要截断数据头
		if (TCP_SEQ_BETWEEN(pcb->rcv_nxt, seqno + 1, seqno + tcplen - 1))
		{
	  /* Trimming the first edge is done by pushing the payload
	     pointer in the pbuf downwards. This is somewhat tricky since
	     we do not want to discard the full contents of the pbuf up to
	     the new starting point of the data since we have to keep the
	     TCP header which is present in the first pbuf in the chain.

	     What is done is really quite a nasty hack: the first pbuf in
	     the pbuf chain is pointed to by inseg.p. Since we need to be
	     able to deallocate the whole pbuf, we cannot change this
	     inseg.p pointer to point to any of the later pbufs in the
	     chain. Instead, we point the ->payload pointer in the first
	     pbuf to data in one of the later pbufs. We also set the
	     inseg.data pointer to point to the right place. This way, the
	     ->p pointer will still point to the first pbuf, but the
	     ->p->payload pointer will point to data in another pbuf.

	     After we are done with adjusting the pbuf pointers we must
	     adjust the ->data pointer in the seg and the segment
	     length.*/

			off = pcb->rcv_nxt - seqno;		// 需要截掉的数据长度
			p = inseg.p;					// 获取收到的报文段的pbuf链表头
			LWIP_ASSERT("inseg.p != NULL", inseg.p);
			LWIP_ASSERT("insane offset!", (off < 0x7fff));

			// 判断需要截断的长度是否超出了第一个pbuf中存储的数据长度
			if (inseg.p->len < off) 
			{
				LWIP_ASSERT("pbuf too short!", (((s32_t)inseg.p->tot_len) >= off));
				new_tot_len = (u16_t)(inseg.p->tot_len - off);	// 截断重复数据后的有效数据长度

				// 如果超出，则需要遍历pbuf链表，依次摘除数据，直到最后一个包含摘除数据的pbuf
				while (p->len < off) 
				{
					off -= p->len;				// 剩余摘除长度
					/* KJM following line changed (with addition of new_tot_len var)
					 to fix bug #9076
					 inseg.p->tot_len -= p->len; */
					p->tot_len = new_tot_len;	// 更新当前pbuf中的数据总长，
					p->len = 0;					// 因为数据被摘除，所以当前pbuf中的数据分长清0
					p = p->next;				// 指向下一个pbuf
				}

				// 处理最后一个包含摘除数据的pbuf，就是调整数据指针略过摘除数据
				if(pbuf_header(p, (s16_t)-off)) 
				{
				  /* Do we need to cope with this failing?  Assert for now */
				  LWIP_ASSERT("pbuf_header failed", 0);
				}
			} 
			else 
			{
				// 如果未超出，则调整第一个pbuf中的数据指针略过摘除数据
				if(pbuf_header(inseg.p, (s16_t)-off)) {
				  /* Do we need to cope with this failing?  Assert for now */
				  LWIP_ASSERT("pbuf_header failed", 0);
				}
			}
			inseg.len -= (u16_t)(pcb->rcv_nxt - seqno);	// 更新TCP报文段数据总长
			inseg.tcphdr->seqno = seqno = pcb->rcv_nxt;	// 更新TCP头中的seqno，指向接收窗口头位置
    	}
    	else 
		{
			// 如果seqno < rcv_nxt，意味着seqno+tcplen-1 < rcv_nxt，说明这是个重复的报文段
			if (TCP_SEQ_LT(seqno, pcb->rcv_nxt))
			{
				/* the whole segment is < rcv_nxt */
				/* must be a duplicate of a packet that has already been correctly handled */

				LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_receive: duplicate seqno %"U32_F"\n", seqno));
				tcp_ack_now(pcb);		// 只回复一个ACK给对方(这里是否应该直接返回不再运行下去)
			}
    	}

	    /* The sequence number must be within the window (above rcv_nxt
	       and below rcv_nxt + rcv_wnd) in order to be further
	       processed. */
	    // 如果数据起始编号在接收范围内
	    if (TCP_SEQ_BETWEEN(seqno, pcb->rcv_nxt, pcb->rcv_nxt + pcb->rcv_wnd - 1))
        {
        	// 如果该报文数据处于接收起始位置，意味着该报文是连续到来的
			if (pcb->rcv_nxt == seqno) 
			{
				/* The incoming segment is the next in sequence. We check if
				   we have to trim the end of the segment and update rcv_nxt
				   and pass the data to the application. */
				tcplen = TCP_TCPLEN(&inseg);		// 更新该TCP报文的总数据长度

				// 如果总长大于接收窗口大小，就需要做截断处理，这里包含对FIN和SYN两种标志的不同处理结果，注意体会
				if (tcplen > pcb->rcv_wnd) 
				{
					LWIP_DEBUGF(TCP_INPUT_DEBUG, 
					("tcp_receive: other end overran receive window"
					"seqno %"U32_F" len %"U16_F" right edge %"U32_F"\n",
					seqno, tcplen, pcb->rcv_nxt + pcb->rcv_wnd));

					// 如果TCP头中带FIN标志，清除FIN标志
					if (TCPH_FLAGS(inseg.tcphdr) & TCP_FIN) 
					{
						/* Must remove the FIN from the header as we're trimming 
						* that byte of sequence-space from the packet */
						TCPH_FLAGS_SET(inseg.tcphdr, TCPH_FLAGS(inseg.tcphdr) &~ TCP_FIN);
					}
					
					/* Adjust length of segment to fit in the window. */
					inseg.len = pcb->rcv_wnd;		// 根据接收窗口调整数据长度

					// 如果TCP头中带SYN标志，报文段数据长度减1
					if (TCPH_FLAGS(inseg.tcphdr) & TCP_SYN) 
					{
						inseg.len -= 1;
					}
					
					pbuf_realloc(inseg.p, inseg.len);	//  因为数据被截断，pbuf中的参数需要相应调整
					tcplen = TCP_TCPLEN(&inseg);		// 再次更新该报文的总数据长度
					LWIP_ASSERT("tcp_receive: segment not trimmed correctly to rcv_wnd\n",
					(seqno + tcplen) == (pcb->rcv_nxt + pcb->rcv_wnd));
				}
#if TCP_QUEUE_OOSEQ
				/* Received in-sequence data, adjust ooseq data if:
				   - FIN has been received or
				   - inseq overlaps with ooseq */
				// 如果无序报文段队列ooseq上存在报文段
		        if (pcb->ooseq != NULL) 
				{
					// 判断当前有序报文段的TCP头中是否带FIN标志
					if (TCPH_FLAGS(inseg.tcphdr) & TCP_FIN) 
					{
						LWIP_DEBUGF(TCP_INPUT_DEBUG, 
						            ("tcp_receive: received in-order FIN, binning ooseq queue\n"));
						/* Received in-order FIN means anything that was received
						 * out of order must now have been received in-order, so
						 * bin the ooseq queue */
						// 如果该有序报文段带FIN标志，意味着单向TCP连接结束
						// 不可能再从对方收到新的报文段，ooseq队列中的报文段没有成为有序报文段可能，只能作废
						while (pcb->ooseq != NULL)
						{
						  struct tcp_seg *old_ooseq = pcb->ooseq;
						  pcb->ooseq = pcb->ooseq->next;
						  tcp_seg_free(old_ooseq);
						}
					}
					else 
					{
						next = pcb->ooseq;
						/* Remove all segments on ooseq that are covered by inseg already.
						 * FIN is copied from ooseq to inseg if present. */
						// 遍历ooseq链表，删除序号被当前有序报文段完全覆盖的报文段
						while (next && TCP_SEQ_GEQ(seqno + tcplen,next->tcphdr->seqno + next->len)) 
						{
							/* inseg cannot have FIN here (already processed above) */
							// 如果这些即将被删除的报文段带FIN标志且当前有序报文段不带SYN标志
							if (TCPH_FLAGS(next->tcphdr) & TCP_FIN &&(TCPH_FLAGS(inseg.tcphdr) & TCP_SYN) == 0) 
							{
								TCPH_SET_FLAG(inseg.tcphdr, TCP_FIN);	// 在当前有效报文段的TCP头中添加FIN标志
								tcplen = TCP_TCPLEN(&inseg);			// 再次更新该报文的总数据长度
							}
							prev = next;
							next = next->next;
							tcp_seg_free(prev);
						}
						/* Now trim right side of inseg if it overlaps with the first
						 * segment on ooseq */
						// 如果当前有序报文段尾部与ooseq中的报文段存在部分重叠
						if (next && TCP_SEQ_GT(seqno + tcplen,next->tcphdr->seqno)) 
						{
							/* inseg cannot have FIN here (already processed above) */
							inseg.len = (u16_t)(next->tcphdr->seqno - seqno);	// 截断当前有序报文段尾部的重叠部分

							// 如果当前有序报文段TCP头中带SYN标志，报文段数据长度减1
							if (TCPH_FLAGS(inseg.tcphdr) & TCP_SYN) 
							{
								inseg.len -= 1;
							}
							
							pbuf_realloc(inseg.p, inseg.len);			//  因为数据被截断，pbuf中的参数需要相应调整
							tcplen = TCP_TCPLEN(&inseg);				// 再次更新该报文的总数据长度
							LWIP_ASSERT("tcp_receive: segment not trimmed correctly to ooseq queue\n",
							(seqno + tcplen) == next->tcphdr->seqno);
						}
						pcb->ooseq = next;
					}
        		}
#endif /* TCP_QUEUE_OOSEQ */

		        pcb->rcv_nxt = seqno + tcplen;	// 更新下一个期望接收到的序号

		        /* Update the receiver's (our) window. */
		        LWIP_ASSERT("tcp_receive: tcplen > rcv_wnd\n", pcb->rcv_wnd >= tcplen);
		        pcb->rcv_wnd -= tcplen;			// 更新当前可用接收窗口

		        tcp_update_rcv_ann_wnd(pcb);

		        /* If there is data in the segment, we make preparations to
		           pass this up to the application. The ->recv_data variable
		           is used for holding the pbuf that goes to the
		           application. The code for reassembling out-of-sequence data
		           chains its data on this pbuf as well.

		           If the segment was a FIN, we set the TF_GOT_FIN flag that will
		           be used to indicate to the application that the remote side has
		           closed its end of the connection. */
		        // 如果该有序报文段中存在数据
		        if (inseg.p->tot_len > 0) 
				{
					recv_data = inseg.p;		// 将全局指针recv_data指向报文段中的数据pbuf
					/* Since this pbuf now is the responsibility of the
					 application, we delete our reference to it so that we won't
					 (mistakingly) deallocate it. */
					inseg.p = NULL;
		        }

				// 如果该有序报文段的TCP头中带FIN标志
		        if (TCPH_FLAGS(inseg.tcphdr) & TCP_FIN) 
				{
		          LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_receive: received FIN.\n"));
		          recv_flags |= TF_GOT_FIN;		// 则全局变量recv_flags添加TF_GOT_FIN标志
		        }

#if TCP_QUEUE_OOSEQ
		        /* We now check if we have segments on the ->ooseq queue that
		           are now in sequence. */
		        // 遍历ooseq队列，取出所有有序的报文段
		        // (通过比较ooseq队列中报文段的seqno和当前TCP控制块中保存的rcv_nxt来判定该报文段是否有序)
		        while (pcb->ooseq != NULL && pcb->ooseq->tcphdr->seqno == pcb->rcv_nxt) 
				{
					cseg = pcb->ooseq;
					seqno = pcb->ooseq->tcphdr->seqno;	// 更新序号

					pcb->rcv_nxt += TCP_TCPLEN(cseg);	// 更新下一个期望接收到的序号
					LWIP_ASSERT("tcp_receive: ooseq tcplen > rcv_wnd\n",
					pcb->rcv_wnd >= TCP_TCPLEN(cseg));
					pcb->rcv_wnd -= TCP_TCPLEN(cseg);	// 更新当前可用接收窗口

					tcp_update_rcv_ann_wnd(pcb);

					// 如果该有序报文段中存在数据，则通过全局指针recv_data向上层提交数据
					if (cseg->p->tot_len > 0) 
					{
						// 判断全局指针recv_data是否为空
						if (recv_data) 
						{
							// 如果不为空，意味着有更早的数据准备向上提交
							pbuf_cat(recv_data, cseg->p);	// 将当前数据pbuf挂到recv_data指向的数据链表的尾部
						} 
						else 
						{
							// 如果为空，直接将当前数据pbuf赋给recv_data
							recv_data = cseg->p;
						}
						cseg->p = NULL;
					}

					// 如果该有序报文段的TCP头中带FIN标志
					if (TCPH_FLAGS(cseg->tcphdr) & TCP_FIN) 
					{
						LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_receive: dequeued FIN.\n"));
						recv_flags |= TF_GOT_FIN;		// 则全局变量recv_flags添加TF_GOT_FIN标志

						// 如果当前TCP处于ESTABLISHED状态，则变成CLOSE_WAIT状态
						if (pcb->state == ESTABLISHED) 
						{ 
							/* force passive close or we can move to active close */
							pcb->state = CLOSE_WAIT;
						} 
					}

					pcb->ooseq = cseg->next;
					tcp_seg_free(cseg);
		        }
#endif /* TCP_QUEUE_OOSEQ */


				// 以上都执行完毕后，向源端返回一个ACK，此处其实只是先在TCP控制块中添加ACK标志
				tcp_ack(pcb);

			}
			// 如果该报文数据不处于接收起始位置，意味着该报文不是有序的
			else 
			{
		        /* We get here if the incoming segment is out-of-sequence. */
				// 首先向源端返回一个立即ACK
		        tcp_send_empty_ack(pcb);
#if TCP_QUEUE_OOSEQ
		        /* We queue the segment on the ->ooseq queue. */
				// 然后将该报文段放入ooseq队列
		        if (pcb->ooseq == NULL) 
				{
					// 如果ooseq为空，则拷贝该报文段到新开辟的报文段空间，并将新开辟报文段作为ooseq起始单元
		          	pcb->ooseq = tcp_seg_copy(&inseg);
		        } 
				else 
				{
					/* If the queue is not empty, we walk through the queue and
					try to find a place where the sequence number of the
					incoming segment is between the sequence numbers of the
					previous and the next segment on the ->ooseq queue. That is
					the place where we put the incoming segment. If needed, we
					trim the second edges of the previous and the incoming
					segment so that it will fit into the sequence.

					If the incoming segment has the same sequence number as a
					segment on the ->ooseq queue, we discard the segment that
					contains less data. */

					prev = NULL;	// 定义为ooseq链表中上一个报文段，这里首先清空
					// 遍历ooseq队列，选择合适位置插入该报文段
					for(next = pcb->ooseq; next != NULL; next = next->next) 
					{
						// 依次比较两个报文段的起始序号seqno，如果相等
						if (seqno == next->tcphdr->seqno) 
						{
							/* The sequence number of the incoming segment is the
							same as the sequence number of the segment on
							->ooseq. We check the lengths to see which one to
							discard. */
							// 继续比较两个报文段的数据长度
							if (inseg.len > next->len) 
							{
								/* The incoming segment is larger than the old
								segment. We replace some segments with the new
								one. */
								// 如果输入报文段数据长度更长
								// 拷贝该报文段到新开辟的报文段空间
								cseg = tcp_seg_copy(&inseg);

								// 插入ooseq链表
								if (cseg != NULL) 
								{
									// 如果不是ooseq上的第一个报文段
									if (prev != NULL) 
									{
										prev->next = cseg;	// 插入ooseq链表的上一个报文段之后
									} 
									// 如果是第一个
									else 
									{
										pcb->ooseq = cseg;	// 直接替换原有的第一个
									}
									
									tcp_oos_insert_segment(cseg, next);	// 处理好插入后与原有的下一个报文段的影响
								}
								break;	// 退出循环
							} 
							else 
							{
								/* Either the lenghts are the same or the incoming
								segment was smaller than the old one; in either
								case, we ditch the incoming segment. */
								// 如果输入报文段数据长度更短，则直接丢弃，并退出循环
								break;	
							}
						} 
						// 如果不相等
						else 
						{
							// 如果是ooseq上的第一个报文段
							if (prev == NULL) 
							{
								// 如果该报文段的起始序号大于要插入的报文段起始序号
								if (TCP_SEQ_LT(seqno, next->tcphdr->seqno)) 
								{
									/* The sequence number of the incoming segment is lower
									than the sequence number of the first segment on the
									queue. We put the incoming segment first on the
									queue. */
									cseg = tcp_seg_copy(&inseg);	// 拷贝要插入的报文段到新开辟的报文段空间
									if (cseg != NULL) 
									{
										pcb->ooseq = cseg;	// 将新报文段插到ooseq第一个位置
										tcp_oos_insert_segment(cseg, next);	// 处理好插入后与原有的下一个报文段的影响
									}
									break;		// 退出循环
								}
							} 
							// 如果不是第一个
							else 
							{
								/*if (TCP_SEQ_LT(prev->tcphdr->seqno, seqno) &&
								TCP_SEQ_LT(seqno, next->tcphdr->seqno)) {*/
								// 带插入报文段起始序号在前一个和后一个报文段起始序号之间
								if (TCP_SEQ_BETWEEN(seqno, prev->tcphdr->seqno+1, next->tcphdr->seqno-1)) 
								{
									/* The sequence number of the incoming segment is in
									between the sequence numbers of the previous and
									the next segment on ->ooseq. We trim trim the previous
									segment, delete next segments that included in received segment
									and trim received, if needed. */
									cseg = tcp_seg_copy(&inseg);	// 拷贝要插入的报文段到新开辟的报文段空间
									if (cseg != NULL) 
									{
										// 如果与前一个报文段有数据重合
										if (TCP_SEQ_GT(prev->tcphdr->seqno + prev->len, seqno)) 
										{
											/* We need to trim the prev segment. */
											prev->len = (u16_t)(seqno - prev->tcphdr->seqno);	// 截断前一个报文段尾部
											pbuf_realloc(prev->p, prev->len);					// 因为数据被截断，pbuf中的参数需要相应调整
										}
										prev->next = cseg;		// 将新报文段插入前一个报文段之后
										tcp_oos_insert_segment(cseg, next);	// 处理好插入后与原有的下一个报文段的影响
									}
									break;	// 退出循环
								}
							}
							/* If the "next" segment is the last segment on the
							ooseq queue, we add the incoming segment to the end
							of the list. */
							// 如果已经是ooseq上的最后一个报文段
							// 且待插入的报文段起始序号大于该报文起始序号(其实函数运行到这里该条件必然成立)
							if (next->next == NULL && TCP_SEQ_GT(seqno, next->tcphdr->seqno)) 
							{
								// 如果该报文的TCP头中有FIN标志，则直接丢弃待插入的报文段，退出循环
								if (TCPH_FLAGS(next->tcphdr) & TCP_FIN) 
								{
								/* segment "next" already contains all data */
								break;
								}
								next->next = tcp_seg_copy(&inseg);	// 拷贝要插入的报文段到新开辟的报文段空间，并插在队列尾部

								// 如果新插入的报文段不为空
								if (next->next != NULL) 
								{
									// 如果与前一个报文段有数据重合
									if (TCP_SEQ_GT(next->tcphdr->seqno + next->len, seqno)) 
									{
									/* We need to trim the last segment. */
									next->len = (u16_t)(seqno - next->tcphdr->seqno);	// 截断前一个报文段尾部
									pbuf_realloc(next->p, next->len);					// 因为数据被截断，pbuf中的参数需要相应调整
									}
									
									/* check if the remote side overruns our receive window */
									// 如果新插入的报文段数据长度超出了当前接收窗口大小
									if ((u32_t)tcplen + seqno > pcb->rcv_nxt + (u32_t)pcb->rcv_wnd) 
									{
										LWIP_DEBUGF(TCP_INPUT_DEBUG, 
										("tcp_receive: other end overran receive window"
										"seqno %"U32_F" len %"U16_F" right edge %"U32_F"\n",
										seqno, tcplen, pcb->rcv_nxt + pcb->rcv_wnd));
										// 如果新插入的报文段的TCP头中有FIN标志
										if (TCPH_FLAGS(next->next->tcphdr) & TCP_FIN) 
										{
										/* Must remove the FIN from the header as we're trimming 
										* that byte of sequence-space from the packet */
										TCPH_FLAGS_SET(next->next->tcphdr, TCPH_FLAGS(next->next->tcphdr) &~ TCP_FIN);	// 去掉TCP头中的FIN标志
										}
										/* Adjust length of segment to fit in the window. */
										next->next->len = pcb->rcv_nxt + pcb->rcv_wnd - seqno;	// 根据接收窗口大小调制新插入的报文段数据长度
										pbuf_realloc(next->next->p, next->next->len);			// 因为数据被截断，pbuf中的参数需要相应调整
										tcplen = TCP_TCPLEN(next->next);						// 再次更新该报文的总数据长度
										LWIP_ASSERT("tcp_receive: segment not trimmed correctly to rcv_wnd\n",
										(seqno + tcplen) == (pcb->rcv_nxt + pcb->rcv_wnd));
									}
								}
								break;	// 退出循环
							}
            			}
            			prev = next;	// 以上都不满足，则遍历ooseq链表中下一个
          			}
        		}
#endif /* TCP_QUEUE_OOSEQ */

      		}
    	}
		// 如果数据不在接收范围内
		else 
		{
			/* The incoming segment is not withing the window. */
			tcp_send_empty_ack(pcb);	// 直接向源端返回一个不带任何数据确认的ACK
		}
  	}
	// 如果输入的报文段中不包含数据
	else 
	{
	    /* Segments with length 0 is taken care of here. Segments that
	       fall out of the window are ACKed. */
	    /*if (TCP_SEQ_GT(pcb->rcv_nxt, seqno) ||
	      TCP_SEQ_GEQ(seqno, pcb->rcv_nxt + pcb->rcv_wnd)) {*/
	    // 且序号位于接收窗口之内
	    if(!TCP_SEQ_BETWEEN(seqno, pcb->rcv_nxt, pcb->rcv_nxt + pcb->rcv_wnd-1)){
	      tcp_ack_now(pcb);		// 回一个ACK
	    }
  	}
}

/**
 * Parses the options contained in the incoming segment. 
 *
 * Called from tcp_listen_input() and tcp_process().
 * Currently, only the MSS option is supported!
 *
 * @param pcb the tcp_pcb for which a segment arrived
 */
static void
tcp_parseopt(struct tcp_pcb *pcb)
{
  u16_t c, max_c;
  u16_t mss;
  u8_t *opts, opt;
#if LWIP_TCP_TIMESTAMPS
  u32_t tsval;
#endif

  opts = (u8_t *)tcphdr + TCP_HLEN;

  /* Parse the TCP MSS option, if present. */
  if(TCPH_HDRLEN(tcphdr) > 0x5) {
    max_c = (TCPH_HDRLEN(tcphdr) - 5) << 2;
    for (c = 0; c < max_c; ) {
      opt = opts[c];
      switch (opt) {
      case 0x00:
        /* End of options. */
        LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_parseopt: EOL\n"));
        return;
      case 0x01:
        /* NOP option. */
        ++c;
        LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_parseopt: NOP\n"));
        break;
      case 0x02:
        LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_parseopt: MSS\n"));
        if (opts[c + 1] != 0x04 || c + 0x04 > max_c) {
          /* Bad length */
          LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_parseopt: bad length\n"));
          return;
        }
        /* An MSS option with the right option length. */
        mss = (opts[c + 2] << 8) | opts[c + 3];
        /* Limit the mss to the configured TCP_MSS and prevent division by zero */
        pcb->mss = ((mss > TCP_MSS) || (mss == 0)) ? TCP_MSS : mss;
        /* Advance to next option */
        c += 0x04;
        break;
#if LWIP_TCP_TIMESTAMPS
      case 0x08:
        LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_parseopt: TS\n"));
        if (opts[c + 1] != 0x0A || c + 0x0A > max_c) {
          /* Bad length */
          LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_parseopt: bad length\n"));
          return;
        }
        /* TCP timestamp option with valid length */
        tsval = (opts[c+2]) | (opts[c+3] << 8) | 
          (opts[c+4] << 16) | (opts[c+5] << 24);
        if (flags & TCP_SYN) {
          pcb->ts_recent = ntohl(tsval);
          pcb->flags |= TF_TIMESTAMP;
        } else if (TCP_SEQ_BETWEEN(pcb->ts_lastacksent, seqno, seqno+tcplen)) {
          pcb->ts_recent = ntohl(tsval);
        }
        /* Advance to next option */
        c += 0x0A;
        break;
#endif
      default:
        LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_parseopt: other\n"));
        if (opts[c + 1] == 0) {
          LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_parseopt: bad length\n"));
          /* If the length field is zero, the options are malformed
             and we don't process them further. */
          return;
        }
        /* All other options have a length field, so that we easily
           can skip past them. */
        c += opts[c + 1];
      }
    }
  }
}

#endif /* LWIP_TCP */
