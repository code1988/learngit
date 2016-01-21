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
static struct tcp_seg inseg;	// ������֯�յ��ı��Ķ�pbuf
static struct tcp_hdr *tcphdr;	// ָ���Ķ��е�TCP�ײ�
static struct ip_hdr *iphdr;	// ָ���Ķ��е�IP�ײ�
static u32_t seqno, ackno;		//  TCP�ײ��е���ź�ȷ�Ϻ�
static u8_t flags;				// TCP�ײ��еı�־�ֶ�
static u16_t tcplen;			// TCP���Ķ������ݵĳ���,����SYN��FIN���ģ��ó���Ҫ��1

// ����2��������recv_flags���ڴ洢tcp_process�����Ĵ�������recv_data���ڴ洢���ϲ��ύ�����ݣ�
static u8_t recv_flags;			// Ŀǰֻ�ᱣ��3��������:TF_GOT_FIN��TF_RESET��TF_CLOSED
static struct pbuf *recv_data;	

struct tcp_pcb *tcp_input_pcb;	// ����ǰ���ĶεĿ��ƿ�

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
 	TCP��������뺯������IP���ip_input���ã��������ݰ�������Ӧtcp���ƿ��Լ���Ӧ����	
 	tcp_timewait_input��tcp_listen_input��tcp_process���д���(��Ѱ�ҵ�һ�����ʵĽӿ�)
 	������õ���ǰ2����tcp_input���������������غ�ͽ�����
 	��������õ��ǵ�3������tcp_input��Ҫ���������Ӧ����
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

	// �Թ�IP��ͷ����ȡTCPͷ
	iphdr = (struct ip_hdr *)p->payload;
	tcphdr = (struct tcp_hdr *)((u8_t *)p->payload + IPH_HL(iphdr) * 4);

#if TCP_INPUT_DEBUG
  tcp_debug_print(tcphdr);
#endif

	// �ƶ�pbuf�ṹ�е����ݰ�ָ�룬ʹָ��TCPͷ
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

	// ����������Ĺ㲥��
	if (ip_addr_isbroadcast(&current_iphdr_dest, inp) || ip_addr_ismulticast(&current_iphdr_dest)) 
    {
    	TCP_STATS_INC(tcp.proterr);
    	TCP_STATS_INC(tcp.drop);
    	snmp_inc_tcpinerrs();
    	pbuf_free(p);
    	return;
	}

#if CHECKSUM_CHECK_TCP
	// ��֤TCPУ���
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

	// �����ƶ�pbuf�ṹ�е����ݰ�ָ�룬ʹָ��TCP����
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

	// �����ֽ���ת�����ֽ���
	tcphdr->src = ntohs(tcphdr->src);				// Դ�˿�
	tcphdr->dest = ntohs(tcphdr->dest);				// Ŀ�Ķ˿�
	seqno = tcphdr->seqno = ntohl(tcphdr->seqno);	// ���
	ackno = tcphdr->ackno = ntohl(tcphdr->ackno);	// ȷ�����
	tcphdr->wnd = ntohs(tcphdr->wnd);				// ���ڴ�С

	flags = TCPH_FLAGS(tcphdr);						// 6λ��־λ
	tcplen = p->tot_len + ((flags & (TCP_FIN | TCP_SYN)) ? 1 : 0);	// ���ݰ������ݵĳ��ȣ�����FIN��SYN��־��1�����ݰ����ó���Ҫ��1

	// ���¾��ǶԽ��յ������ݰ����з��ദ��Ҳ����Ѱ�Һ��ʵĽӿڣ�����addr��port
	// ������tcp_active_pcbs ���ң���û��ƥ���tcp_pcb��
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
			 // �ҵ�ƥ��Ľӿ�֮��,����tcp_pcb��tcp_active_pcbs�������ȡ�����˳�ѭ���������У���ʱpcb != NULL
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

	// �����tcp_active_pcbs��û���ҵ���������tcp_tw_pcbs ��tcp_listen_pcbs����
  	if (pcb == NULL) 
	{
	    // ��tcp_tw_pcbs����
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
				// ����TIME_WAIT״̬����������ֱ�����ﷵ�ز�����������
				tcp_timewait_input(pcb);
				pbuf_free(p);
				return;
			}
	    }

	    // ��tcp_listen_pcbs����
	    prev = NULL;
	    for(lpcb = tcp_listen_pcbs.listen_pcbs; lpcb != NULL; lpcb = lpcb->next) 
		{
			// �ж϶˿��Ƿ�ƥ��
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
				// Ȼ���ж�IP�Ƿ�ƥ�䣬������IPADDR_ANY�����κ�IP
		        if (ip_addr_cmp(&(lpcb->local_ip), &current_iphdr_dest) ||
		            ip_addr_isany(&(lpcb->local_ip))) 
		        {
		        	 // �ҵ�ƥ��Ľӿ�֮���˳�ѭ���������У���ʱlpcb != NULL
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
		// �������ж���tcp_listen_pcbs���Ƿ��ҵ�
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
			// ����LISTEN	״̬����������ֱ�����ﷵ�ز�����������
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

	// �����tcp_active_pcbs���ҵ��ˣ��򾭹���������tcp_process
  	if (pcb != NULL) 
	{
    	/* The incoming segment belongs to a connection. */
#if TCP_INPUT_DEBUG
#if TCP_DEBUG
    tcp_debug_print_state(pcb->state);
#endif /* TCP_DEBUG */
#endif /* TCP_INPUT_DEBUG */

	    /* Set up a tcp_seg structure. */
	    inseg.next = NULL;		// �رձ��Ķζ��й���
	    inseg.len = p->tot_len;	// ���ñ��Ķ������ܳ�
	    inseg.p = p;			// ���ñ��Ķ���������ͷָ��
	    inseg.tcphdr = tcphdr;	// ���ñ��Ķε�TCPͷ

	    recv_data = NULL;
	    recv_flags = 0;			// �����¼��ǰ���ƿ�״̬�ı�־���������ﱻ��0

	    // tcp_pcb��refused_dataָ�����Ƿ񻹼�¼����δ���ϲ�ݽ�������
	    if (pcb->refused_data != NULL) 
		{
			// �еĻ��ص��û�recv��������δ�ݽ�������
			LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_input: notify kept packet\n"));
			TCP_EVENT_RECV(pcb, pcb->refused_data, ERR_OK, err);
			// �жϴ���recv�����Ĵ��������ɹ�refused_dataָ����գ���������ִ��tcp_process
			if (err == ERR_OK) 
			{
			    pcb->refused_data = NULL;
			} 
			// ʧ����ζ��tcp_pcb����ռ�������������հ����ٴ���ֱ�ӷ���
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
    	tcp_input_pcb = pcb;	// ��¼����ǰ���ĵĿ��ƿ�

		// ������ǽ���tcp_process������հ�������
	    err = tcp_process(pcb);
	    // ������ֵΪERR_ABRT��˵�����ƿ��Ѿ�����ȫɾ��(tcp_abort()),ʲôҲ����Ҫ��
	    if (err != ERR_ABRT) 
		{
			// ����ֵ��ΪERR_ABRTʱ���жϱ��Ĵ���ĸ��ֽ��
			if (recv_flags & TF_RESET) 			// ���յ��Է��ĸ�λ����
			{
				/* TF_RESET means that the connection was reset by the other
				   end. We then call the error callback to inform the
				   application that the connection is dead before we
				   deallocate the PCB. */
				// �ص��û���errf����
				TCP_EVENT_ERR(pcb->errf, pcb->callback_arg, ERR_RST);
				// ɾ�����ƿ�
				tcp_pcb_remove(&tcp_active_pcbs, pcb);
				// �ͷſ��ƿ�ռ�
				memp_free(MEMP_TCP_PCB, pcb);
			} 
			else if (recv_flags & TF_CLOSED) 	// ˫�����ӳɹ��Ͽ�
			{
				/* The connection has been closed and we will deallocate the
				   PCB. */
				// ɾ�����ƿ�
				tcp_pcb_remove(&tcp_active_pcbs, pcb);
				// �ͷſ��ƿ�ռ�
				memp_free(MEMP_TCP_PCB, pcb);
			} 
			else 								
			{
				err = ERR_OK;
				/* If the application has registered a "sent" function to be
				   called when new send buffer space is available, we call it
				   now. */
				if (pcb->acked > 0) 		// ��������ݱ�ȷ��		
				{
					// �ص��û���send����
				  	TCP_EVENT_SENT(pcb, pcb->acked, err);
				  	if (err == ERR_ABRT) {
				    goto aborted;
				  	}
				}

				if (recv_data != NULL)			// ��������ݱ����յ� 
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
					if (flags & TCP_PSH) 		// ���TCP��־λ�д���PSH
					{
						// ����pbuf�ײ���flag�ֶ�
						recv_data->flags |= PBUF_FLAG_PUSH;
					}

					// �ص��û���recv���������յݽ���ȥ��TCP����recv_data
					TCP_EVENT_RECV(pcb, recv_data, ERR_OK, err);
					// �жϷ���ֵ�������ERR_ABRT������������
					if (err == ERR_ABRT) {
					goto aborted;
					}

					// ����֮�⣬�������ֵ��ʧ�ܣ����ⲿ����δ���ϵݽ��������ݴ浽refused_dataָ����
					if (err != ERR_OK) {
					pcb->refused_data = recv_data;
					LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_input: keep incoming packet, because pcb is \"full\"\n"));
					}
				}

				/* If a FIN segment was received, we call the callback
				   function with a NULL buffer to indicate EOF. */
				if (recv_flags & TF_GOT_FIN)	// ����յ��Է���FIN���� 
				{
					/* correct rcv_wnd as the application won't call tcp_recved()
					 for the FIN's seqno */
					// �������մ��� 
					if (pcb->rcv_wnd != TCP_WND) {
					pcb->rcv_wnd++;
					}
					// ��һ��NULLָ��ص��û���recv������ͨ�����ַ�ʽ�û��������֪���Է��Ĺر�����
					TCP_EVENT_CLOSED(pcb, err);
					if (err == ERR_ABRT) {
					goto aborted;
					}
				}

				tcp_input_pcb = NULL;		// ��ǰ���ĵ��˴�����ϣ���յ�ǰ���ĵĿ��ƿ�
				/* Try to send something out. */
				tcp_output(pcb);			// �������
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
	    // �����3�������ﶼδ�ҵ�ƥ���pcb�������tcp_rst��Դ��������һ��TCP��λ���ݰ�
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
 *	tcp_pcb�ȵ�����SYN���ݰ�ʱ���뱾����
 	�������Ǵ���LISTEN״̬�Ŀ��ƿ鴦�����뱨�ĺ���
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
  	if (flags & TCP_ACK) 		// TCP_ACK - ����listen״̬��pcbֻ����ӦSYN���ְ������ԶԷ����ְ�����һ��tcp_rst
  	{
	    /* For incoming segments with the ACK flag set, respond with a
	       RST. */
	    LWIP_DEBUGF(TCP_RST_DEBUG, ("tcp_listen_input: ACK in LISTEN, sending reset\n"));
	    tcp_rst(ackno + 1, seqno + tcplen,
	      ip_current_dest_addr(), ip_current_src_addr(),
	      tcphdr->dest, tcphdr->src);
  	} 
	else if (flags & TCP_SYN) 	// TCP_SYN - ����listen״̬�ķ������˵ȵ���SYN���ְ�
	{
    	LWIP_DEBUGF(TCP_DEBUG, ("TCP connection request %"U16_F" -> %"U16_F".\n", tcphdr->src, tcphdr->dest));
#if TCP_LISTEN_BACKLOG
    if (pcb->accepts_pending >= pcb->backlog) {
      LWIP_DEBUGF(TCP_DEBUG, ("tcp_listen_input: listen backlog exceeded for port %"U16_F"\n", tcphdr->dest));
      return ERR_ABRT;
    }
#endif /* TCP_LISTEN_BACKLOG */
		// ����һ���µ�tcp_pcb����Ϊ����tcp_listen_pcbs�����ϵ�pcb��tcp_pcb_listen�ṹ�ģ������������ϵ�pcb��tcp_pcb�ṹ
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
	    // Ϊ����½���tcp_pcb����Ա
	    ip_addr_copy(npcb->local_ip, current_iphdr_dest);
	    npcb->local_port = pcb->local_port;
	    ip_addr_copy(npcb->remote_ip, current_iphdr_src);
	    npcb->remote_port = tcphdr->src;
	    npcb->state = SYN_RCVD;			// SYN_RCVD - ��ʾ�������յ�SYN״̬
	    npcb->rcv_nxt = seqno + 1;		// �������յ�����һ����ţ�ע���1
	    npcb->rcv_ann_right_edge = npcb->rcv_nxt;
	    npcb->snd_wnd = tcphdr->wnd;	// ���÷��ʹ��ڴ�С
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
	    //��������úõ�tcp_pcbע�ᵽtcp_active_pcbs������ȥ
	    TCP_REG(&tcp_active_pcbs, npcb);

	    // ���յ���SYN���ְ�����ȡTCPͷ��ѡ���ֶε�ֵ�������õ��Լ���tcp_pcb
	    tcp_parseopt(npcb);
#if TCP_CALCULATE_EFF_SEND_MSS
    	npcb->mss = tcp_eff_send_mss(npcb->mss, &(npcb->remote_ip));
#endif /* TCP_CALCULATE_EFF_SEND_MSS */

	    snmp_inc_tcppassiveopens();

	    // �ظ�����SYN��ACK��־���������ݰ�
	    rc = tcp_enqueue_flags(npcb, TCP_SYN | TCP_ACK);
	    if (rc != ERR_OK) {
	      tcp_abandon(npcb, 0);
	      return rc;
	    }

		// �������ͻظ���������
	    return tcp_output(npcb);
  	}
  	return ERR_OK;
}

/**
 * Called by tcp_input() when a segment arrives for a connection in
 * TIME_WAIT.
 *	TIME_WAIT״̬��ָtcp�ر�һ�����Ӻ������״̬
 	�������Ǵ���TIME_WAIT״̬�Ŀ��ƿ鴦�����뱨�ĵĺ���
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
  	if (flags & TCP_RST)  	// TCP_RST - ��λ���ӣ�ֱ�ӷ���
  	{
    return ERR_OK;
  	}
  	/* - fourth, check the SYN bit, */
  	if (flags & TCP_SYN) 	// TCP_SYN - ͬ�����кţ�����һ��ACK
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
	else if (flags & TCP_FIN)	// TCP_FIN - ����������ʱ�� 	
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
 *	����LISTEN��TIME_WAIT��������״̬�Ŀ��ƿ飬�����뱨�ĵĴ���������
 	�ú�����Ҫʵ����TCP״̬ת������
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

	// �����жϸñ����ǲ���һ��RST����
	if (flags & TCP_RST) 
	{
		// �жϸ�RST�����Ƿ�Ϸ�
		if (pcb->state == SYN_SENT) 	// ��һ����������Ӵ���SYN_SENT״̬
		{
			if (ackno == pcb->snd_nxt) 	// �ұ�����ȷ�Ϻ���snd_nxt���
			{
				acceptable = 1;
			}
		} 
		else 							// �ڶ������������״̬�±����е����к��ڽ��մ�����
		{
			if (TCP_SEQ_BETWEEN(seqno, pcb->rcv_nxt,pcb->rcv_nxt+pcb->rcv_wnd)) 
			{
				acceptable = 1;
			}
		}

		// ���RST���ĺϷ�������Ҫ��λ��ǰ���ӵĿ��ƿ飬�Ƿ���ֱ�ӷ��ز�������
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

	// �������ֱ���SYN���������Ѿ���������£������ǽ��յ��Է������ְ�
	// ˵���������һ����ʱ�ط������ְ���ֱ����Է�����һ��ACK����
	if ((flags & TCP_SYN) && (pcb->state != SYN_SENT && pcb->state != SYN_RCVD)) 
	{ 
		/* Cope with new connection attempt after remote end crashed */
		tcp_ack_now(pcb);
		return ERR_OK;
	}

	// ��λ���ƿ�Ļ������
  	if ((pcb->flags & TF_RXCLOSED) == 0) 
  	{
    	/* Update the PCB (in)activity timer unless rx is closed (see tcp_shutdown) */
    	pcb->tmr = tcp_ticks;
  	}

	// ����ļ�������0
  	pcb->keep_cnt_sent = 0;

	// �������ײ��е�ѡ���ֶ�
  	tcp_parseopt(pcb);

  	// ���ݲ�ͬ��TCP״ִ̬����Ӧ����
  	switch (pcb->state) 
  	{
	  	case SYN_SENT:	// �ͻ��˷���SYN�󣬾ʹ��ڸ�״̬�ȴ�����������SYN+ACK
			LWIP_DEBUGF(TCP_INPUT_DEBUG, ("SYN-SENT: ackno %"U32_F" pcb->snd_nxt %"U32_F" unacked %"U32_F"\n", ackno,
			 pcb->snd_nxt, ntohl(pcb->unacked->tcphdr->seqno)));
			// ����յ�����SYN+ACK�������к���ȷ
			if ((flags & TCP_ACK) && (flags & TCP_SYN) && ackno == ntohl(pcb->unacked->tcphdr->seqno) + 1) 
	        {
				pcb->snd_buf++;		// SYN�����ص�ACKȷ�ϣ��ñ���ռ��1���ֽڣ����Կ��õķ��Ϳռ��1�ֽ�
				pcb->rcv_nxt = seqno + 1;		// �������յ���һ����ţ������ն����Ͷ�ACK�����е�ȷ�Ϻ�
				pcb->rcv_ann_right_edge = pcb->rcv_nxt;	// ��ʼ��ͨ�洰�ڵ��ұ߽�ֵ
				pcb->lastack = ackno;			// ���½��յ������ȷ�����
				pcb->snd_wnd = tcphdr->wnd;		// ���ʹ�������Ϊ���մ��ڴ�С
				pcb->snd_wl1 = seqno - 1; 		// �ϴδ��ڸ���ʱ�յ����������
				pcb->state = ESTABLISHED;		// ����ESTABLISHED״̬

#if TCP_CALCULATE_EFF_SEND_MSS
      			pcb->mss = tcp_eff_send_mss(pcb->mss, &(pcb->remote_ip));	// ��������Ķ�
#endif /* TCP_CALCULATE_EFF_SEND_MSS */

				/* Set ssthresh again after changing pcb->mss (already set in tcp_connect
				* but for the default value of pcb->mss) */
				pcb->ssthresh = pcb->mss * 10;	// ����mss��ssthreshֵҲҪ��Ӧ�޸�

				pcb->cwnd = ((pcb->cwnd == 1) ? (pcb->mss * 2) : pcb->mss);	// ��ʼ����������
				LWIP_ASSERT("pcb->snd_queuelen > 0", (pcb->snd_queuelen > 0));
				--pcb->snd_queuelen;			// SYN�����ص�ACKȷ�ϣ�����ռ�õ�pbuf������1
				LWIP_DEBUGF(TCP_QLEN_DEBUG, ("tcp_process: SYN-SENT --queuelen %"U16_F"\n", (u16_t)pcb->snd_queuelen));
				rseg = pcb->unacked;			// �ڷ�����δ�յ�ȷ�ϵ����ݶζ�����ɾ��SYN����
				pcb->unacked = rseg->next;
				
				if(pcb->unacked == NULL)		// ���δȷ�ϵ����ݶζ���Ϊ�գ���ֹͣ�ش���ʱ��
				pcb->rtime = -1;
				else 							// ��������л��б��ģ���λ�ش���ʱ�����ش�����
				{
				pcb->rtime = 0;
				pcb->nrtx = 0;
				}

				tcp_seg_free(rseg);				// �ͷ�ȡ�µ�SYN���Ķοռ�

				/* Call the user specified function to call when sucessfully
				* connected. */
				TCP_EVENT_CONNECTED(pcb, ERR_OK, err);	// �ص��û���connect����
				if (err == ERR_ABRT) {
				return ERR_ABRT;
				}
				tcp_ack_now(pcb);				// �����������ACK���������ֽ���
				}
				// ���ֻ�յ��Է���ACKȴû��SYN������Է�����RST
				else if (flags & TCP_ACK) {
				/* send a RST to bring the other side in a non-synchronized state. */
				tcp_rst(ackno, seqno + tcplen, ip_current_dest_addr(), ip_current_src_addr(),
				tcphdr->dest, tcphdr->src);
				}
				break;
		case SYN_RCVD:	// ����������SYN+ACK�󣬾ʹ��ڸ�״̬���ȴ��ͻ��˷���ACK
			// ����յ�ACK��Ҳ�����������ֵ����һ������
			if (flags & TCP_ACK) 
			{
				// ���ACK�Ϸ�
				if (TCP_SEQ_BETWEEN(ackno, pcb->lastack+1, pcb->snd_nxt)) 
				{
					u16_t old_cwnd;
					pcb->state = ESTABLISHED;	// ����ESTABLISHED״̬
					LWIP_DEBUGF(TCP_DEBUG, ("TCP connection established %"U16_F" -> %"U16_F".\n", inseg.tcphdr->src, inseg.tcphdr->dest));
#if LWIP_CALLBACK_API
		#ifdef sys_bios_ind_sdk
		 LWIP_ASSERT("pcb->accept != NULL", pcb->acceptdata != NULL);
		#else
         LWIP_ASSERT("pcb->accept != NULL", pcb->accept != NULL);
        #endif
#endif
					/* Call the accept function. */
					TCP_EVENT_ACCEPT(pcb, ERR_OK, err);		// �ص��û���accept����
					if (err != ERR_OK) 						// ���accept�������ش�����رյ�ǰ����
					{
					  /* If the accept function returns with an error, we abort
					   * the connection. */
					  /* Already aborted? */
					  if (err != ERR_ABRT) {
					    tcp_abort(pcb);
					  }
					  return ERR_ABRT;
					}
					old_cwnd = pcb->cwnd;		// ����ɵ���������
					/* If there was any data contained within this ACK,
					 * we'd better pass it on to the application as well. */
					tcp_receive(pcb);			// ���ú����������е����� 

					// ���������δȷ�����ݱ������е�ACKȷ��
					if (pcb->acked != 0) 		
					{
					  pcb->acked--;				// ����ȷ�ϵ��ֽ�������ΪSYN����ռ��1���ֽڣ����Լ�1
					}
			
			        pcb->cwnd = ((old_cwnd == 1) ? (pcb->mss * 2) : pcb->mss);	// ��ʼ����������

					// ����������tcp_receive����ʱ�����˹ر����ӱ�־
			        if (recv_flags & TF_GOT_FIN) 
					{
			          tcp_ack_now(pcb);			// �ظ�ACK����Ӧ�Է���FIN���ֱ�־
			          pcb->state = CLOSE_WAIT;	// ����CLOSE_WAIT״̬
			        }
			  	} 
				else 
				{
			        // ���ڲ��Ϸ���ACK���򷵻�һ��RST
			        tcp_rst(ackno, seqno + tcplen, ip_current_dest_addr(), ip_current_src_addr(),tcphdr->dest, tcphdr->src);
      			}
    		}
			// ����յ��ͻ����ظ�SYN���ְ���˵��SYN+ACK����ʧ����Ҫ�ش�
			else if ((flags & TCP_SYN) && (seqno == pcb->rcv_nxt - 1)) 
			{
      			/* Looks like another copy of the SYN - retransmit our SYN-ACK */
      			tcp_rexmit(pcb);
    		}
    		break;
  		case CLOSE_WAIT:	// ���������ڰ�ر�״̬����һֱ�ȴ��ϲ�Ӧ��ִ�йر�ָ�����״̬��ΪLASK_ACK
    	/* FALLTHROUGH */
  		case ESTABLISHED:	// ����˫���������ȶ�״̬
    		tcp_receive(pcb);		// ���ú����������е�����

			// ����������tcp_receive����ʱ�����˹ر����ӱ�־
    		if (recv_flags & TF_GOT_FIN) 
			{ /* passive close */
      			tcp_ack_now(pcb);			// �ظ�ACK����Ӧ�Է���FIN���ֱ�־
      			pcb->state = CLOSE_WAIT;	// ����CLOSE_WAIT״̬
    		}
    		break;
  		case FIN_WAIT_1:	// �ϲ�Ӧ������ִ�йر�ָ�����FIN���ڸ�״̬
    		tcp_receive(pcb);	// ���ú����������е�����

			// ����������tcp_receive����ʱ�����˹ر����ӱ�־�����յ�FIN����
    		if (recv_flags & TF_GOT_FIN) 
			{
				// ����ñ���ͬʱ����һ���Ϸ�ACK
	      		if ((flags & TCP_ACK) && (ackno == pcb->snd_nxt)) 
				{
			        LWIP_DEBUGF(TCP_DEBUG,
			          ("TCP connection closed: FIN_WAIT_1 %"U16_F" -> %"U16_F".\n", inseg.tcphdr->src, inseg.tcphdr->dest));
			        tcp_ack_now(pcb);			// �ظ�ACK
			        tcp_pcb_purge(pcb);			// ����������е������ִ�����
			        TCP_RMV(&tcp_active_pcbs, pcb);	// ��tcp_active_pcbs������ɾ���ÿ��ƿ�
			        pcb->state = TIME_WAIT;		// ����FIN_WAIT_2״̬��ֱ�ӽ���TIME_WAIT״̬
			        TCP_REG(&tcp_tw_pcbs, pcb);	// ���ÿ��ƿ����tcp_tw_pcbs����
	      		} 
				// ����ñ��Ĳ���ACK������ʾ˫��ͬʱִ���˹ر����Ӳ���
				else 
				{
	        		tcp_ack_now(pcb);		// ����ACK
	        		pcb->state = CLOSING;	// ����CLOSING״̬
	      		}
	    	} 
			// ���ֻ�յ���Ч��ACK
			else if ((flags & TCP_ACK) && (ackno == pcb->snd_nxt)) 
			{
      			pcb->state = FIN_WAIT_2;	// ����FIN_WAIT_2״̬
		    }
		    break;
		case FIN_WAIT_2:	// �����رգ�����FIN�������յ�ACK���ڸ�״̬
			tcp_receive(pcb);	// ���ú����������е�����

			// ����������tcp_receive����ʱ�����˹ر����ӱ�־�����յ�FIN����
			if (recv_flags & TF_GOT_FIN) 
			{
				LWIP_DEBUGF(TCP_DEBUG, ("TCP connection closed: FIN_WAIT_2 %"U16_F" -> %"U16_F".\n", inseg.tcphdr->src, inseg.tcphdr->dest));
				tcp_ack_now(pcb);		// �ظ�ACK
				tcp_pcb_purge(pcb);		// ����������е������ִ�����
				TCP_RMV(&tcp_active_pcbs, pcb);	// ��tcp_active_pcbs������ɾ���ÿ��ƿ�
				pcb->state = TIME_WAIT;	// ����TIME_WAIT״̬
				TCP_REG(&tcp_tw_pcbs, pcb);		// ���ÿ��ƿ����tcp_tw_pcbs����
			}
			break;
		case CLOSING:		// ˫��ͬʱִ�������رգ����ڸ�״̬
			tcp_receive(pcb);	// ���ú����������е�����

			// ����յ��Ϸ�ACK
			if (flags & TCP_ACK && ackno == pcb->snd_nxt) 
			{
				LWIP_DEBUGF(TCP_DEBUG, ("TCP connection closed: CLOSING %"U16_F" -> %"U16_F".\n", inseg.tcphdr->src, inseg.tcphdr->dest));
				tcp_pcb_purge(pcb);		// ����������е������ִ�����
				TCP_RMV(&tcp_active_pcbs, pcb);	// ��tcp_active_pcbs������ɾ���ÿ��ƿ�
				pcb->state = TIME_WAIT;	// ����TIME_WAIT״̬
				TCP_REG(&tcp_tw_pcbs, pcb);		// ���ÿ��ƿ����tcp_tw_pcbs����
			}
			break;
		case LAST_ACK:		// ��������ִ�б����ر�ʱ��������FIN���ȴ�ACKʱ���ڸ�״̬
			tcp_receive(pcb);	// ���ú����������е�����

			// ����յ��Ϸ�ACK
			if (flags & TCP_ACK && ackno == pcb->snd_nxt) 
			{
				LWIP_DEBUGF(TCP_DEBUG, ("TCP connection closed: LAST_ACK %"U16_F" -> %"U16_F".\n", inseg.tcphdr->src, inseg.tcphdr->dest));
				/* bugfix #21699: don't set pcb->state to CLOSED here or we risk leaking segments */
				recv_flags |= TF_CLOSED;	// recv_flags����ΪTF_CLOSED����tcp_input�����Ըÿ��ƿ�����ͷź����
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
 * �����뱨�Ķβ����������ͷ�ԭ�б��Ķε��ڴ�
 * Called from tcp_receive()
 */
static void
tcp_oos_insert_segment(struct tcp_seg *cseg, struct tcp_seg *next)
{
	struct tcp_seg *old_seg;

	// ������뱨�Ķε�TCPͷ����FIN��־����ζ��ooseq�����к������б��Ķζ�����
	if (TCPH_FLAGS(cseg->tcphdr) & TCP_FIN) 
	{
		/* received segment overlaps all following segments */
		tcp_segs_free(next);	// ֱ���ͷ�ԭ�б��Ķ��ڴ�
		next = NULL;			// �����������б��Ķ�
	}
	else 
	{
		/* delete some following segments
		   oos queue may have segments with FIN flag */
		// ����ooseq�����б��ĶΣ�ɾ�����б����뱨�Ķ�������ȫ���ǵı��Ķ�
		while (next && TCP_SEQ_GEQ((seqno + cseg->len),(next->tcphdr->seqno + next->len))) 
		{
			/* cseg with FIN already processed */
			// �����ɾ���ı��Ķε�TCPͷ����FIN��־
			if (TCPH_FLAGS(next->tcphdr) & TCP_FIN) 
			{
				TCPH_SET_FLAG(cseg->tcphdr, TCP_FIN);	// ����FIN��־�����뱨�Ķε�TCPͷ��
			}
			old_seg = next;
			next = next->next;
			tcp_seg_free(old_seg);						// �ͷű���ȫ���ǵı��Ķ�
		}

		// ������뱨�Ķκ�ooseq������ԭ�б��Ķδ��������ص�
		// ע�⣬�˴�lwipԴ�������������ȷӦ��if (next && TCP_SEQ_LT(seqno + cseg->len, next->tcphdr->seqno)) 
		if (next && TCP_SEQ_GT(seqno + cseg->len, next->tcphdr->seqno)) 
		{
			/* We need to trim the incoming segment. */
			cseg->len = (u16_t)(next->tcphdr->seqno - seqno);	// �Բ��뱨�Ķ�β�����ݽ��нضϣ�ȥ���ص�����
			pbuf_realloc(cseg->p, cseg->len);	//  ��Ϊ���ݱ��ضϣ�pbuf�еĲ�����Ҫ��Ӧ����
		}
	}
	cseg->next = next;	// �����뱨�Ķκ�ooseq�������ı��Ķ��ν�
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
	int found_dupack = 0;	// �ظ�ack��־����1��ʾ���ظ�ack

	// ���ȼ�ⱨ���Ƿ����ACK��־
	if (flags & TCP_ACK) 
	{
		right_wnd_edge = pcb->snd_wnd + pcb->snd_wl2;

		// ��3��������Ե��±��ط��ʹ��ڸ���
		if (TCP_SEQ_LT(pcb->snd_wl1, seqno)||								// snd_wl1С����seqno
			(pcb->snd_wl1 == seqno && TCP_SEQ_LT(pcb->snd_wl2, ackno))||	// snd_wl1������seqno��snd_wl2С����ackno��˵���Է�û�з������ݣ�ֻ�����յ����ݺ���һ��ȷ��
			(pcb->snd_wl2 == ackno && tcphdr->wnd > pcb->snd_wnd)) 			// snd_wl2������ackno��snd_wndС�ڱ����ײ��Ĵ���ͨ��wnd
		{
		  	pcb->snd_wnd = tcphdr->wnd;		// ���±��ط��ʹ��ڴ�С
		  	pcb->snd_wl1 = seqno;			// ���½��յ����������
		  	pcb->snd_wl2 = ackno;			// ���½��յ���Ӧ�����
		  	if (pcb->snd_wnd > 0 && pcb->persist_backoff > 0) 	// ��⵽��0������̽�鿪��
		  	{
		      pcb->persist_backoff = 0;		// ֹͣ����̽��
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
     ֻ����������5��������ackno�Żᱻ����һ���ظ���ack
     * 1) It doesn't ACK new data 		û��ȷ��������
     * 2) length of received packet is zero (i.e. no payload) 	���Ķ���û������
     * 3) the advertised window hasn't changed 	���ط��ʹ���û�и���
     * 4) There is outstanding unacknowledged data (retransmission timer running)		�������������ȴ���ȷ��
     * 5) The ACK is == biggest ACK sequence number so far seen (snd_una)		ackno����lastack
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
		// �ж��Ƿ���һ���ظ���ACK
    	if (TCP_SEQ_LEQ(ackno, pcb->lastack)) 						// ���acknoС�ڵ���lastack����û��ȷ��������
		{
	      	pcb->acked = 0;		// ����ǰ����ȷ�ϵ��ѷ��ͳ�����0
	      	/* Clause 2 */
	      	if (tcplen == 0) 										// ������Ķ���û������
		  	{
	        	/* Clause 3 */
	        	if (pcb->snd_wl2 + pcb->snd_wnd == right_wnd_edge)	// ���ط��ʹ���û�и���
				{
					/* Clause 4 */
					if (pcb->rtime >= 0) 							// �ش���ʱ���������У����������������ȴ���ȷ��
					{
						/* Clause 5 */
						if (pcb->lastack == ackno) 					// ���ackno����lastack
						{
							// ��ʱ����ȷ������һ���ظ���ack��˵�����ķ����˶�ʧ
							found_dupack = 1;
							if (pcb->dupacks + 1 > pcb->dupacks)	// ��ack���ظ��յ��Ĵ�������	
								++pcb->dupacks;
							if (pcb->dupacks > 3) 					// �����ack�ظ��յ�����3�Σ�˵��������ӵ��
							{
								/* Inflate the congestion window, but not if it means that
								the value overflows. */
								if ((u16_t)(pcb->cwnd + pcb->mss) > pcb->cwnd) 
								{
									pcb->cwnd += pcb->mss;
								}
							} 
							else if (pcb->dupacks == 3) 			// �����ack�ظ���3���յ���ִ�п����ش��㷨
							{
								/* Do fast retransmit */
								tcp_rexmit_fast(pcb);
							}
						}
					}
	        	}
	      	}
	      	// ���û��ȷ�������ݵ��ֲ������ظ�ack
	      	if (!found_dupack) 
			{
	        	pcb->dupacks = 0;		// ��ack�ظ��յ��Ĵ�����0
	      	}
	    } 

		// ��������������ACK��lastack+1<=ackno<=snd_nxt
		else if (TCP_SEQ_BETWEEN(ackno, pcb->lastack+1, pcb->snd_nxt))
		{
			/* We come here when the ACK acknowledges new data. */

			/* Reset the "IN Fast Retransmit" flag, since we are no longer
			 in fast retransmit. Also reset the congestion window to the
			 slow start threshold. */
			// ������ƿ鴦�ڿ����ش�״̬	����ر��ش�״̬��ӵ������		 
			if (pcb->flags & TF_INFR) 
			{
				pcb->flags &= ~TF_INFR;
				pcb->cwnd = pcb->ssthresh;
			}

			/* Reset the number of retransmissions. */
			pcb->nrtx = 0;								// �ش�������0

			/* Reset the retransmission time-out. */
			pcb->rto = (pcb->sa >> 3) + pcb->sv;		// ��λ�ش���ʱʱ��

			/* Update the send buffer space. Diff between the two can never exceed 64K? */
			pcb->acked = (u16_t)(ackno - pcb->lastack);	// ����acked�ֶ�Ϊ��ȷ�ϵ�������

			pcb->snd_buf += pcb->acked;					// �ͷſ��õķ��Ϳռ�

			/* Reset the fast retransmit variables. */
			pcb->dupacks = 0;							// ��ack�ظ��յ��Ĵ�����0
			pcb->lastack = ackno;						// ���½��յ���ackno

			/* Update the congestion control variables (cwnd and
			 ssthresh). */
			// �������TCP�����Ѿ�����״̬������ӵ���㷨����ģ��
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

			// ����unacked���У����������ݱ��С�ڵ���ackno�ı��Ķ��Ƴ�
			while (pcb->unacked != NULL &&
			     TCP_SEQ_LEQ(ntohl(pcb->unacked->tcphdr->seqno) + TCP_TCPLEN(pcb->unacked), ackno)) 
			{
				LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_receive: removing %"U32_F":%"U32_F" from pcb->unacked\n",
				                              ntohl(pcb->unacked->tcphdr->seqno),
				                              ntohl(pcb->unacked->tcphdr->seqno) +
				                              TCP_TCPLEN(pcb->unacked)));
				// ������Ҫ��ı��Ĵ�unacked����ȡ��
				next = pcb->unacked;
				pcb->unacked = pcb->unacked->next;

				LWIP_DEBUGF(TCP_QLEN_DEBUG, ("tcp_receive: queuelen %"U16_F" ... ", (u16_t)pcb->snd_queuelen));
				LWIP_ASSERT("pcb->snd_queuelen >= pbuf_clen(next->p)", (pcb->snd_queuelen >= pbuf_clen(next->p)));
				// �����ACK����Ա��ط�����FIN�������ģ���acked�ֶμ�1��������Ҫ�ύ�ϲ�ʹ֪��FIN���Է��ɹ�����
				if ((pcb->acked != 0) && ((TCPH_FLAGS(next->tcphdr) & TCP_FIN) != 0)) 
				{
				  	pcb->acked--;
				}

				pcb->snd_queuelen -= pbuf_clen(next->p);		// �ͷű��ñ���ռ�õķ��Ϳռ�
				tcp_seg_free(next);								// �ͷű��ñ���ռ�õ�tcp���Ķ�

				LWIP_DEBUGF(TCP_QLEN_DEBUG, ("%"U16_F" (after freeing unacked)\n", (u16_t)pcb->snd_queuelen));
				if (pcb->snd_queuelen != 0) 
				{
				  LWIP_ASSERT("tcp_receive: valid queue length", pcb->unacked != NULL ||
				              pcb->unsent != NULL);
				}
			}

			// ����������Ҫ��ı��Ķ��Ƴ��ɹ����ж�unacked�����Ƿ�Ϊ��
			if(pcb->unacked == NULL)
				pcb->rtime = -1;	// ��Ϊ�գ��ر��ش���ʱ��
			else
				pcb->rtime = 0;		// ����λ�ش���ʱ��

			pcb->polltmr = 0;		// ��λ��ѯ��ʱ��
    	}
		// �����ACK�Ȳ����ظ�ACK���ֲ�������ACK����acked�ֶ���0
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
	    // ����unsent���У����������ݱ��С�ڵ���ackno�ı��Ķ��Ƴ�
	    // ������Ϊ������Ҫ�ش��ı��ĶΣ�lwipֱ�ӽ����ǹ���unsent�����ϣ������յ���ACK�����Ƕ��ѳ�ʱ���Ķε�ȷ��
	    while (pcb->unsent != NULL &&
	           TCP_SEQ_BETWEEN(ackno, ntohl(pcb->unsent->tcphdr->seqno) + TCP_TCPLEN(pcb->unsent), pcb->snd_nxt)) 
	    {
			LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_receive: removing %"U32_F":%"U32_F" from pcb->unsent\n",
			                            ntohl(pcb->unsent->tcphdr->seqno), ntohl(pcb->unsent->tcphdr->seqno) +
			                            TCP_TCPLEN(pcb->unsent)));
			// ������Ҫ��ı��Ĵ�unsent����ȡ��
			next = pcb->unsent;
			pcb->unsent = pcb->unsent->next;
			LWIP_DEBUGF(TCP_QLEN_DEBUG, ("tcp_receive: queuelen %"U16_F" ... ", (u16_t)pcb->snd_queuelen));
			LWIP_ASSERT("pcb->snd_queuelen >= pbuf_clen(next->p)", (pcb->snd_queuelen >= pbuf_clen(next->p)));
			
			// �����ACK����Ա��ط�����FIN�������ģ���acked�ֶμ�1��������Ҫ�ύ�ϲ�ʹ֪��FIN���Է��ɹ�����
			if ((pcb->acked != 0) && ((TCPH_FLAGS(next->tcphdr) & TCP_FIN) != 0)) 
			{
				pcb->acked--;
			}
			pcb->snd_queuelen -= pbuf_clen(next->p);		// �ͷű��ñ���ռ�õķ��Ϳռ�
			tcp_seg_free(next);								// �ͷű��ñ���ռ�õ�tcp���Ķ�
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
	    // RTT���㣬����
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

	// ��������뱨�Ļ����������ݣ���Ҫ���������ݽ��д���
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
	    // ���seqno+1 <= rcv_nxt <= seqno + tcplen-1����ζ���յ�����������ͷ������Ч���ݣ���Ҫ�ض�����ͷ
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

			off = pcb->rcv_nxt - seqno;		// ��Ҫ�ص������ݳ���
			p = inseg.p;					// ��ȡ�յ��ı��Ķε�pbuf����ͷ
			LWIP_ASSERT("inseg.p != NULL", inseg.p);
			LWIP_ASSERT("insane offset!", (off < 0x7fff));

			// �ж���Ҫ�ضϵĳ����Ƿ񳬳��˵�һ��pbuf�д洢�����ݳ���
			if (inseg.p->len < off) 
			{
				LWIP_ASSERT("pbuf too short!", (((s32_t)inseg.p->tot_len) >= off));
				new_tot_len = (u16_t)(inseg.p->tot_len - off);	// �ض��ظ����ݺ����Ч���ݳ���

				// �������������Ҫ����pbuf��������ժ�����ݣ�ֱ�����һ������ժ�����ݵ�pbuf
				while (p->len < off) 
				{
					off -= p->len;				// ʣ��ժ������
					/* KJM following line changed (with addition of new_tot_len var)
					 to fix bug #9076
					 inseg.p->tot_len -= p->len; */
					p->tot_len = new_tot_len;	// ���µ�ǰpbuf�е������ܳ���
					p->len = 0;					// ��Ϊ���ݱ�ժ�������Ե�ǰpbuf�е����ݷֳ���0
					p = p->next;				// ָ����һ��pbuf
				}

				// �������һ������ժ�����ݵ�pbuf�����ǵ�������ָ���Թ�ժ������
				if(pbuf_header(p, (s16_t)-off)) 
				{
				  /* Do we need to cope with this failing?  Assert for now */
				  LWIP_ASSERT("pbuf_header failed", 0);
				}
			} 
			else 
			{
				// ���δ�������������һ��pbuf�е�����ָ���Թ�ժ������
				if(pbuf_header(inseg.p, (s16_t)-off)) {
				  /* Do we need to cope with this failing?  Assert for now */
				  LWIP_ASSERT("pbuf_header failed", 0);
				}
			}
			inseg.len -= (u16_t)(pcb->rcv_nxt - seqno);	// ����TCP���Ķ������ܳ�
			inseg.tcphdr->seqno = seqno = pcb->rcv_nxt;	// ����TCPͷ�е�seqno��ָ����մ���ͷλ��
    	}
    	else 
		{
			// ���seqno < rcv_nxt����ζ��seqno+tcplen-1 < rcv_nxt��˵�����Ǹ��ظ��ı��Ķ�
			if (TCP_SEQ_LT(seqno, pcb->rcv_nxt))
			{
				/* the whole segment is < rcv_nxt */
				/* must be a duplicate of a packet that has already been correctly handled */

				LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_receive: duplicate seqno %"U32_F"\n", seqno));
				tcp_ack_now(pcb);		// ֻ�ظ�һ��ACK���Է�(�����Ƿ�Ӧ��ֱ�ӷ��ز���������ȥ)
			}
    	}

	    /* The sequence number must be within the window (above rcv_nxt
	       and below rcv_nxt + rcv_wnd) in order to be further
	       processed. */
	    // ���������ʼ����ڽ��շ�Χ��
	    if (TCP_SEQ_BETWEEN(seqno, pcb->rcv_nxt, pcb->rcv_nxt + pcb->rcv_wnd - 1))
        {
        	// ����ñ������ݴ��ڽ�����ʼλ�ã���ζ�Ÿñ���������������
			if (pcb->rcv_nxt == seqno) 
			{
				/* The incoming segment is the next in sequence. We check if
				   we have to trim the end of the segment and update rcv_nxt
				   and pass the data to the application. */
				tcplen = TCP_TCPLEN(&inseg);		// ���¸�TCP���ĵ������ݳ���

				// ����ܳ����ڽ��մ��ڴ�С������Ҫ���ضϴ������������FIN��SYN���ֱ�־�Ĳ�ͬ��������ע�����
				if (tcplen > pcb->rcv_wnd) 
				{
					LWIP_DEBUGF(TCP_INPUT_DEBUG, 
					("tcp_receive: other end overran receive window"
					"seqno %"U32_F" len %"U16_F" right edge %"U32_F"\n",
					seqno, tcplen, pcb->rcv_nxt + pcb->rcv_wnd));

					// ���TCPͷ�д�FIN��־�����FIN��־
					if (TCPH_FLAGS(inseg.tcphdr) & TCP_FIN) 
					{
						/* Must remove the FIN from the header as we're trimming 
						* that byte of sequence-space from the packet */
						TCPH_FLAGS_SET(inseg.tcphdr, TCPH_FLAGS(inseg.tcphdr) &~ TCP_FIN);
					}
					
					/* Adjust length of segment to fit in the window. */
					inseg.len = pcb->rcv_wnd;		// ���ݽ��մ��ڵ������ݳ���

					// ���TCPͷ�д�SYN��־�����Ķ����ݳ��ȼ�1
					if (TCPH_FLAGS(inseg.tcphdr) & TCP_SYN) 
					{
						inseg.len -= 1;
					}
					
					pbuf_realloc(inseg.p, inseg.len);	//  ��Ϊ���ݱ��ضϣ�pbuf�еĲ�����Ҫ��Ӧ����
					tcplen = TCP_TCPLEN(&inseg);		// �ٴθ��¸ñ��ĵ������ݳ���
					LWIP_ASSERT("tcp_receive: segment not trimmed correctly to rcv_wnd\n",
					(seqno + tcplen) == (pcb->rcv_nxt + pcb->rcv_wnd));
				}
#if TCP_QUEUE_OOSEQ
				/* Received in-sequence data, adjust ooseq data if:
				   - FIN has been received or
				   - inseq overlaps with ooseq */
				// ��������Ķζ���ooseq�ϴ��ڱ��Ķ�
		        if (pcb->ooseq != NULL) 
				{
					// �жϵ�ǰ�����Ķε�TCPͷ���Ƿ��FIN��־
					if (TCPH_FLAGS(inseg.tcphdr) & TCP_FIN) 
					{
						LWIP_DEBUGF(TCP_INPUT_DEBUG, 
						            ("tcp_receive: received in-order FIN, binning ooseq queue\n"));
						/* Received in-order FIN means anything that was received
						 * out of order must now have been received in-order, so
						 * bin the ooseq queue */
						// ����������Ķδ�FIN��־����ζ�ŵ���TCP���ӽ���
						// �������ٴӶԷ��յ��µı��ĶΣ�ooseq�����еı��Ķ�û�г�Ϊ�����Ķο��ܣ�ֻ������
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
						// ����ooseq����ɾ����ű���ǰ�����Ķ���ȫ���ǵı��Ķ�
						while (next && TCP_SEQ_GEQ(seqno + tcplen,next->tcphdr->seqno + next->len)) 
						{
							/* inseg cannot have FIN here (already processed above) */
							// �����Щ������ɾ���ı��Ķδ�FIN��־�ҵ�ǰ�����Ķβ���SYN��־
							if (TCPH_FLAGS(next->tcphdr) & TCP_FIN &&(TCPH_FLAGS(inseg.tcphdr) & TCP_SYN) == 0) 
							{
								TCPH_SET_FLAG(inseg.tcphdr, TCP_FIN);	// �ڵ�ǰ��Ч���Ķε�TCPͷ�����FIN��־
								tcplen = TCP_TCPLEN(&inseg);			// �ٴθ��¸ñ��ĵ������ݳ���
							}
							prev = next;
							next = next->next;
							tcp_seg_free(prev);
						}
						/* Now trim right side of inseg if it overlaps with the first
						 * segment on ooseq */
						// �����ǰ�����Ķ�β����ooseq�еı��Ķδ��ڲ����ص�
						if (next && TCP_SEQ_GT(seqno + tcplen,next->tcphdr->seqno)) 
						{
							/* inseg cannot have FIN here (already processed above) */
							inseg.len = (u16_t)(next->tcphdr->seqno - seqno);	// �ضϵ�ǰ�����Ķ�β�����ص�����

							// �����ǰ�����Ķ�TCPͷ�д�SYN��־�����Ķ����ݳ��ȼ�1
							if (TCPH_FLAGS(inseg.tcphdr) & TCP_SYN) 
							{
								inseg.len -= 1;
							}
							
							pbuf_realloc(inseg.p, inseg.len);			//  ��Ϊ���ݱ��ضϣ�pbuf�еĲ�����Ҫ��Ӧ����
							tcplen = TCP_TCPLEN(&inseg);				// �ٴθ��¸ñ��ĵ������ݳ���
							LWIP_ASSERT("tcp_receive: segment not trimmed correctly to ooseq queue\n",
							(seqno + tcplen) == next->tcphdr->seqno);
						}
						pcb->ooseq = next;
					}
        		}
#endif /* TCP_QUEUE_OOSEQ */

		        pcb->rcv_nxt = seqno + tcplen;	// ������һ���������յ������

		        /* Update the receiver's (our) window. */
		        LWIP_ASSERT("tcp_receive: tcplen > rcv_wnd\n", pcb->rcv_wnd >= tcplen);
		        pcb->rcv_wnd -= tcplen;			// ���µ�ǰ���ý��մ���

		        tcp_update_rcv_ann_wnd(pcb);

		        /* If there is data in the segment, we make preparations to
		           pass this up to the application. The ->recv_data variable
		           is used for holding the pbuf that goes to the
		           application. The code for reassembling out-of-sequence data
		           chains its data on this pbuf as well.

		           If the segment was a FIN, we set the TF_GOT_FIN flag that will
		           be used to indicate to the application that the remote side has
		           closed its end of the connection. */
		        // ����������Ķ��д�������
		        if (inseg.p->tot_len > 0) 
				{
					recv_data = inseg.p;		// ��ȫ��ָ��recv_dataָ���Ķ��е�����pbuf
					/* Since this pbuf now is the responsibility of the
					 application, we delete our reference to it so that we won't
					 (mistakingly) deallocate it. */
					inseg.p = NULL;
		        }

				// ����������Ķε�TCPͷ�д�FIN��־
		        if (TCPH_FLAGS(inseg.tcphdr) & TCP_FIN) 
				{
		          LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_receive: received FIN.\n"));
		          recv_flags |= TF_GOT_FIN;		// ��ȫ�ֱ���recv_flags���TF_GOT_FIN��־
		        }

#if TCP_QUEUE_OOSEQ
		        /* We now check if we have segments on the ->ooseq queue that
		           are now in sequence. */
		        // ����ooseq���У�ȡ����������ı��Ķ�
		        // (ͨ���Ƚ�ooseq�����б��Ķε�seqno�͵�ǰTCP���ƿ��б����rcv_nxt���ж��ñ��Ķ��Ƿ�����)
		        while (pcb->ooseq != NULL && pcb->ooseq->tcphdr->seqno == pcb->rcv_nxt) 
				{
					cseg = pcb->ooseq;
					seqno = pcb->ooseq->tcphdr->seqno;	// �������

					pcb->rcv_nxt += TCP_TCPLEN(cseg);	// ������һ���������յ������
					LWIP_ASSERT("tcp_receive: ooseq tcplen > rcv_wnd\n",
					pcb->rcv_wnd >= TCP_TCPLEN(cseg));
					pcb->rcv_wnd -= TCP_TCPLEN(cseg);	// ���µ�ǰ���ý��մ���

					tcp_update_rcv_ann_wnd(pcb);

					// ����������Ķ��д������ݣ���ͨ��ȫ��ָ��recv_data���ϲ��ύ����
					if (cseg->p->tot_len > 0) 
					{
						// �ж�ȫ��ָ��recv_data�Ƿ�Ϊ��
						if (recv_data) 
						{
							// �����Ϊ�գ���ζ���и��������׼�������ύ
							pbuf_cat(recv_data, cseg->p);	// ����ǰ����pbuf�ҵ�recv_dataָ������������β��
						} 
						else 
						{
							// ���Ϊ�գ�ֱ�ӽ���ǰ����pbuf����recv_data
							recv_data = cseg->p;
						}
						cseg->p = NULL;
					}

					// ����������Ķε�TCPͷ�д�FIN��־
					if (TCPH_FLAGS(cseg->tcphdr) & TCP_FIN) 
					{
						LWIP_DEBUGF(TCP_INPUT_DEBUG, ("tcp_receive: dequeued FIN.\n"));
						recv_flags |= TF_GOT_FIN;		// ��ȫ�ֱ���recv_flags���TF_GOT_FIN��־

						// �����ǰTCP����ESTABLISHED״̬������CLOSE_WAIT״̬
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


				// ���϶�ִ����Ϻ���Դ�˷���һ��ACK���˴���ʵֻ������TCP���ƿ������ACK��־
				tcp_ack(pcb);

			}
			// ����ñ������ݲ����ڽ�����ʼλ�ã���ζ�Ÿñ��Ĳ��������
			else 
			{
		        /* We get here if the incoming segment is out-of-sequence. */
				// ������Դ�˷���һ������ACK
		        tcp_send_empty_ack(pcb);
#if TCP_QUEUE_OOSEQ
		        /* We queue the segment on the ->ooseq queue. */
				// Ȼ�󽫸ñ��Ķη���ooseq����
		        if (pcb->ooseq == NULL) 
				{
					// ���ooseqΪ�գ��򿽱��ñ��Ķε��¿��ٵı��Ķοռ䣬�����¿��ٱ��Ķ���Ϊooseq��ʼ��Ԫ
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

					prev = NULL;	// ����Ϊooseq��������һ�����ĶΣ������������
					// ����ooseq���У�ѡ�����λ�ò���ñ��Ķ�
					for(next = pcb->ooseq; next != NULL; next = next->next) 
					{
						// ���αȽ��������Ķε���ʼ���seqno��������
						if (seqno == next->tcphdr->seqno) 
						{
							/* The sequence number of the incoming segment is the
							same as the sequence number of the segment on
							->ooseq. We check the lengths to see which one to
							discard. */
							// �����Ƚ��������Ķε����ݳ���
							if (inseg.len > next->len) 
							{
								/* The incoming segment is larger than the old
								segment. We replace some segments with the new
								one. */
								// ������뱨�Ķ����ݳ��ȸ���
								// �����ñ��Ķε��¿��ٵı��Ķοռ�
								cseg = tcp_seg_copy(&inseg);

								// ����ooseq����
								if (cseg != NULL) 
								{
									// �������ooseq�ϵĵ�һ�����Ķ�
									if (prev != NULL) 
									{
										prev->next = cseg;	// ����ooseq�������һ�����Ķ�֮��
									} 
									// ����ǵ�һ��
									else 
									{
										pcb->ooseq = cseg;	// ֱ���滻ԭ�еĵ�һ��
									}
									
									tcp_oos_insert_segment(cseg, next);	// ����ò������ԭ�е���һ�����Ķε�Ӱ��
								}
								break;	// �˳�ѭ��
							} 
							else 
							{
								/* Either the lenghts are the same or the incoming
								segment was smaller than the old one; in either
								case, we ditch the incoming segment. */
								// ������뱨�Ķ����ݳ��ȸ��̣���ֱ�Ӷ��������˳�ѭ��
								break;	
							}
						} 
						// ��������
						else 
						{
							// �����ooseq�ϵĵ�һ�����Ķ�
							if (prev == NULL) 
							{
								// ����ñ��Ķε���ʼ��Ŵ���Ҫ����ı��Ķ���ʼ���
								if (TCP_SEQ_LT(seqno, next->tcphdr->seqno)) 
								{
									/* The sequence number of the incoming segment is lower
									than the sequence number of the first segment on the
									queue. We put the incoming segment first on the
									queue. */
									cseg = tcp_seg_copy(&inseg);	// ����Ҫ����ı��Ķε��¿��ٵı��Ķοռ�
									if (cseg != NULL) 
									{
										pcb->ooseq = cseg;	// ���±��Ķβ嵽ooseq��һ��λ��
										tcp_oos_insert_segment(cseg, next);	// ����ò������ԭ�е���һ�����Ķε�Ӱ��
									}
									break;		// �˳�ѭ��
								}
							} 
							// ������ǵ�һ��
							else 
							{
								/*if (TCP_SEQ_LT(prev->tcphdr->seqno, seqno) &&
								TCP_SEQ_LT(seqno, next->tcphdr->seqno)) {*/
								// �����뱨�Ķ���ʼ�����ǰһ���ͺ�һ�����Ķ���ʼ���֮��
								if (TCP_SEQ_BETWEEN(seqno, prev->tcphdr->seqno+1, next->tcphdr->seqno-1)) 
								{
									/* The sequence number of the incoming segment is in
									between the sequence numbers of the previous and
									the next segment on ->ooseq. We trim trim the previous
									segment, delete next segments that included in received segment
									and trim received, if needed. */
									cseg = tcp_seg_copy(&inseg);	// ����Ҫ����ı��Ķε��¿��ٵı��Ķοռ�
									if (cseg != NULL) 
									{
										// �����ǰһ�����Ķ��������غ�
										if (TCP_SEQ_GT(prev->tcphdr->seqno + prev->len, seqno)) 
										{
											/* We need to trim the prev segment. */
											prev->len = (u16_t)(seqno - prev->tcphdr->seqno);	// �ض�ǰһ�����Ķ�β��
											pbuf_realloc(prev->p, prev->len);					// ��Ϊ���ݱ��ضϣ�pbuf�еĲ�����Ҫ��Ӧ����
										}
										prev->next = cseg;		// ���±��Ķβ���ǰһ�����Ķ�֮��
										tcp_oos_insert_segment(cseg, next);	// ����ò������ԭ�е���һ�����Ķε�Ӱ��
									}
									break;	// �˳�ѭ��
								}
							}
							/* If the "next" segment is the last segment on the
							ooseq queue, we add the incoming segment to the end
							of the list. */
							// ����Ѿ���ooseq�ϵ����һ�����Ķ�
							// �Ҵ�����ı��Ķ���ʼ��Ŵ��ڸñ�����ʼ���(��ʵ�������е������������Ȼ����)
							if (next->next == NULL && TCP_SEQ_GT(seqno, next->tcphdr->seqno)) 
							{
								// ����ñ��ĵ�TCPͷ����FIN��־����ֱ�Ӷ���������ı��ĶΣ��˳�ѭ��
								if (TCPH_FLAGS(next->tcphdr) & TCP_FIN) 
								{
								/* segment "next" already contains all data */
								break;
								}
								next->next = tcp_seg_copy(&inseg);	// ����Ҫ����ı��Ķε��¿��ٵı��Ķοռ䣬�����ڶ���β��

								// ����²���ı��Ķβ�Ϊ��
								if (next->next != NULL) 
								{
									// �����ǰһ�����Ķ��������غ�
									if (TCP_SEQ_GT(next->tcphdr->seqno + next->len, seqno)) 
									{
									/* We need to trim the last segment. */
									next->len = (u16_t)(seqno - next->tcphdr->seqno);	// �ض�ǰһ�����Ķ�β��
									pbuf_realloc(next->p, next->len);					// ��Ϊ���ݱ��ضϣ�pbuf�еĲ�����Ҫ��Ӧ����
									}
									
									/* check if the remote side overruns our receive window */
									// ����²���ı��Ķ����ݳ��ȳ����˵�ǰ���մ��ڴ�С
									if ((u32_t)tcplen + seqno > pcb->rcv_nxt + (u32_t)pcb->rcv_wnd) 
									{
										LWIP_DEBUGF(TCP_INPUT_DEBUG, 
										("tcp_receive: other end overran receive window"
										"seqno %"U32_F" len %"U16_F" right edge %"U32_F"\n",
										seqno, tcplen, pcb->rcv_nxt + pcb->rcv_wnd));
										// ����²���ı��Ķε�TCPͷ����FIN��־
										if (TCPH_FLAGS(next->next->tcphdr) & TCP_FIN) 
										{
										/* Must remove the FIN from the header as we're trimming 
										* that byte of sequence-space from the packet */
										TCPH_FLAGS_SET(next->next->tcphdr, TCPH_FLAGS(next->next->tcphdr) &~ TCP_FIN);	// ȥ��TCPͷ�е�FIN��־
										}
										/* Adjust length of segment to fit in the window. */
										next->next->len = pcb->rcv_nxt + pcb->rcv_wnd - seqno;	// ���ݽ��մ��ڴ�С�����²���ı��Ķ����ݳ���
										pbuf_realloc(next->next->p, next->next->len);			// ��Ϊ���ݱ��ضϣ�pbuf�еĲ�����Ҫ��Ӧ����
										tcplen = TCP_TCPLEN(next->next);						// �ٴθ��¸ñ��ĵ������ݳ���
										LWIP_ASSERT("tcp_receive: segment not trimmed correctly to rcv_wnd\n",
										(seqno + tcplen) == (pcb->rcv_nxt + pcb->rcv_wnd));
									}
								}
								break;	// �˳�ѭ��
							}
            			}
            			prev = next;	// ���϶������㣬�����ooseq��������һ��
          			}
        		}
#endif /* TCP_QUEUE_OOSEQ */

      		}
    	}
		// ������ݲ��ڽ��շ�Χ��
		else 
		{
			/* The incoming segment is not withing the window. */
			tcp_send_empty_ack(pcb);	// ֱ����Դ�˷���һ�������κ�����ȷ�ϵ�ACK
		}
  	}
	// �������ı��Ķ��в���������
	else 
	{
	    /* Segments with length 0 is taken care of here. Segments that
	       fall out of the window are ACKed. */
	    /*if (TCP_SEQ_GT(pcb->rcv_nxt, seqno) ||
	      TCP_SEQ_GEQ(seqno, pcb->rcv_nxt + pcb->rcv_wnd)) {*/
	    // �����λ�ڽ��մ���֮��
	    if(!TCP_SEQ_BETWEEN(seqno, pcb->rcv_nxt, pcb->rcv_nxt + pcb->rcv_wnd-1)){
	      tcp_ack_now(pcb);		// ��һ��ACK
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
