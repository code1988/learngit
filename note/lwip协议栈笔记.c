lwip���߼��Ͽ�Ҳ�Ƿ�Ϊ4�㣺��·�㣨ARP��RARP���������(IP����ICMP��IGMP������Э���������Ĳ���Э�飬�����ϸ���������㣩)�������(TCP��UDP)��Ӧ�ò㣬������ͬTCP/IP��ֻ�Ǹ���֮����Խ��н����ȡ��û���ϸ񻮷֡�
Э����ܣ�
	1. ARPЭ�飺����IP��ַ��ȡ�����ַMAC��һ��TCP/IPЭ��
һ�����͵�lwipϵͳ����3�����̣��������ϲ�Ӧ�ó�����̣�Ȼ����lwipЭ��ջ���̣�����ǵײ�Ӳ�����ݰ����ս���

��̬�ڴ����
	����ucos-ii�ڴ����ϵͳ��������һ���ڴ棬�ָ����������С��ͬ���ڴ��
	
һ.	��·��

	
	ͨ������netif_add�������net_if�ṹ��ĳ�ʼ����
						/* Function Name: struct netif *netif_add(struct netif *netif,ip_addr *ipaddr,ip_addr *netmask,ip_addr *gw,void *state,err_t (* init)(struct netif *netif),err_t (*input)(struct pbuf *p,struct netif *inp))
						/* Input		: *state 											- ����cpswportif��ַ
						**				  err_t (* init)(struct netif *netif) 				- ����cpswif_init���ײ�ӿ�������ʼ������
						**				  err_t (*input)(struct pbuf *p,struct netif *inp)	- ����tcpip_input���ײ�������IP���ύ���ݰ��ĺ���
						*/
	
	��ɳ�ʼ�����net_if�ṹ��ᱻ��������Э��ջʹ��2��ȫ�ֱ���������������netif_listָ�������ͷ��netif_defaultָ��ȱʡ��net_if�ṹ�塣
	
	��̫�����ݰ�������ʽ��		Ŀ��MAC��ַ��6�ֽڣ�	+ ԴMAC��ַ��6�ֽڣ� 	+ ���ͣ�2�ֽڣ� 	+ ���ݣ�46-1500�ֽڣ� 	+ У�飨4�ֽڣ�
	��̫�����ݰ����֡��1518�ֽڣ���С֡��64�ֽڣ����޷�������С64�ֽں�ĩβ��trailer�ֶ�����䲹��64�ֽ�
	lwipʹ��һ��eth_hdr�Ľṹ����������̫�����ݰ���ͷ��14���ֽڣ�
																struct eth_hdr{
																	struct eth_addr dest;	// Ŀ��MAC��ַ
																	struct eth_addr src;	// ԴMAC��ַ
																	u16_t type;				// ���ͣ���Ҫ�õ�2�֣�0x0800 - IP	0x0806 - ARP
																}
	lwipʹ��һ��etharp_hdr�Ľṹ��������ARPЭ���ͷ��
													struct etharp_hdr{
														u16_t hwtype;				// Ӳ���ӿ�����	����̫���̶�Ϊ0x0001
														u16_t proto;				// ��һ��Э�����ͣ�IP�̶�Ϊ0x0800
														u8_t  hwlen;				// Ӳ����ַ��MAC�����ȣ��̶�ֵ6
														u8_t  protolen;				// ��һ��Э���ַ��IP�����ȣ��̶�ֵ4
														u16_t opcode;				// �����룬1 - ARP����	2 - ARP�ظ�
														struct eth_addr shwaddr;	// ԴMAC
														struct ip_addr2 sipaddr;	// ԴIP
														struct eth_addr dhwaddr;	// Ŀ��MAC
														struct ip_addr2 dipaddr;	// Ŀ��IP
													}
																						
	������AҪ������Bͨ��ʱ��ARPЭ����Խ�����B��IP��ַ����������B��MAC��ַ�������������£�	
		��һ��������A�ȼ���Լ���ARP���壬���Ƿ��������Bƥ���MAC��ַ�����û�У��ͻ�����㲥һ��ARP�����
		�ڶ��������������յ��󣬷��������IP��ַ���Լ���IP��ַ��ƥ�䣬����ARP����
		������������Bȷ��ARP�����е�IP��ַ���Լ���IP��ַƥ�䣬������A��IP��ַ��MAC��ַӳ�䵽����ARP������
		���Ĳ�������B��������MAC��ַ��ARP�ظ����ظ�����A
		���岽������A�յ�������B������ARP�ظ�ʱ���Ὣ����B��IP��ַ��MAC��ַӳ����µ�����ARP�����С�����B��MAC��ַһ��ȷ��������A�Ϳ���������B����IPͨ����
	
	������·���������Ŧ������̫�����ݰ����ս���tcpip_thread
		static void tcpip_thread(void *arg)
		{
			struct tcpip_msg *msg;		// ��Ϣ�����������ж�
			
			while(1)
			{
				// �������������������Ҫ�������Ϣ���������ݰ�����ʱ������оƬ�жϺ����������ݣ���post��Ϣ���ж��˳��󣬸������ȡ��Ϣ
				sys_timeouts_mbox_fetch(&mbox, (void **)&msg);	
				
				// �жϱ�����Ϣ�����ͣ�ֻ��ע���ݰ���ϢTCPIP_MSG_INPKT
				switch (msg->type)
				{
					case TCPIP_MSG_INPKT:											// ���ݰ���Ϣ
						if(msg->msg.inp.netif->flags & (NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET))
							ethernet_input��msg->msg.inp.p,msg->msg.inp.netif��;	// ���֧��ARP���Ƚ���ARP�������ж��Ƿ�ݽ�IP�㣬����IP���ݰ�������2��ѡ�����ն�Ҫ����ip_input����IP��
						else
							ip_input(msg->msg.inp.p, msg->msg.inp.netif);			// ����ֱ�ӵݽ�IP��,ip_inputΪIP����Ҫ���������������ģ�����ֱ�ӵ���ip_input�������⣬������Ҫ����̫�����ݰ�ָ�룬ʹ�ӹ���ͷ��ָ��IPЭ���ͷ
						memp_free(MEMP_TCPIP_MSG_INPKT, msg);						// �ͷ���Ϣ�ڴ�
						break;
					case TCPIP_MSG_TIMEOUT:											// ��ʱ��Ϣ
						sys_timeout(msg->msg.tmo.msecs, msg->msg.tmo.h, msg->msg.tmo.arg);
						memp_free(MEMP_TCPIP_MSG_API, msg);
						break;
					default:
						break;
				}
			}
		}
		err_t ethernet_input��struct pbuf *p,struct netif *netif)
		{
			struct eth_hdr *ethhdr;					// ��̫�����ݰ�ͷ�ṹ��
			u16_t type;
			s16_t ip_hdr_offset = SIZEOF_ETH_HDR;	// ��ͷ�̶�ֵ14�ֽ�
			
			ethhdr = (eth_hdr *)p->payload;
			type = htons(ethhdr->type);
			
			switch(type)
			{
				case ETHTYPE_IP:												// IP���ݰ�
					etharp_ip_input(netif,p);									// ����ARP��
					pbuf_header(p, -ip_hdr_offset);								// ������̫�����ݰ�ָ�룬ʹ�ӹ���ͷ��ָ��IPЭ���ͷ
					ip_input(p,netif);											// �ύIP�㣬ip_inputΪIP����Ҫ����������������
				case ETHTYPE_ARP:												// ARP���ݰ�
					etharp_arp_input(netif,(struct eth_addr *)netif->hwaddr,p);	// ARP���ݰ������ڶ����β��Ǳ���MAC
					break;
				default:
					break;
			}
		}
		
		ע����Ϣ�ṹ��struct tcpip_msg {
							enum tcpip_msg_type type;				// ������Ϣ�����ͣ�TCPIP_MSG_INPKT - ���ݰ���Ϣ��TCPIP_MSG_TIMEOUT - ��ʱ��Ϣ
							sys_sem_t *sem;							// �¼����ƿ�ECB
							union{
								struct api_msg *apimsg;
								struct netifapi_msg *netifapimsg;
								struct {
									struct pbuf *p;
									struct netif *netif;
								} inp;								// inp�ṹ������Ҫ���ں����ݰ����ݽṹ������ӿڽṹ
								struct {
							      	tcpip_callback_fn function;
							      	void *ctx;
							    } cb;
							    struct {
							      	u32_t msecs;
							      	sys_timeout_handler h;
							      	void *arg;
							    } tmo;
							}msg;
						}
	
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------	
��. �����
		lwipʹ��һ��ip_hdr�Ľṹ��������IPЭ���ͷ��
												struct ip_hdr{
													u16_t _v_hl_tos;	// ����4λ�汾�ţ�IPv4 - 4��IPv6 - 6����4λIP��ͷ����ͨ��Ϊ5*4,�����ṹ���С����8λ��������
													u16_t _len;			// ����IP���ݰ�����
													u16_t _id;			// 16λ��ʶ���ڱ�ʶIP�㷢����ÿһ��IP���ģ�����
													u16_t _offset;		// ����3λ��־��13λƬƫ�ƣ�IP���ݰ���Ƭʱʹ��
													u8_t _ttl;			// TTL������IP���ݰ�����ܱ�ת���Ĵ������Լ�
													u8_t _proto;		// Э���ֶ�����������IP���ݰ����ϲ�Э�飬0x01 - ICMP,0x02 - IGMP,0x06 - TCP,0x17 - UDP
													u16_t _chksum;		// 16λ��IP�ײ�У���
													ip_addr_p_t src;	// ԴIP
													ip_addr_p_t dest;	// Ŀ��IP
												}
	
		ip_inputΪIP�����ɺ����������IP�����ݰ�����(���Ĺ�������IP��ַƥ�䣻�õ��������ݰ�)��Ȼ�󽫺��ʵ����ݰ��ύ���ϲ㣬�����p->payload�Ѿ�Խ����14�ֽڰ�ͷ��ָ����IPͷ
		err_t ip_input(struct pbuf *p,struct netif *inp)
		{
			struct 	ip_hdr *iphdr;	// ָ��IP��ͷ��ָ��
			struct 	netif *netif;	// ָ��netifӲ������ӿ��豸��������ָ��
			u16_t	iphdr_hlen;		// IP��ͷ�ĳ��ȣ�ͨ���ǹ̶�20�ֽ�
			u16_t	iphdr_len;		// ����IP����������IP��ͷ���ϲ�Э��ͷ������
			
			// ȡ�� IP���ݰ�ͷ
			iphdr = (struct ip_hdr *)p->payload;
			
			// ���IP��ͷ�еİ汾���ֶΣ�IPv4 - 4��IPv6 - 6
			if(IPH_V(iphdr) != 4)
			{
				pbuf_free(p);
				return ERR_OK;	
			}
			
			// ��ȡIP��ͷ�е�ͷ�����ֶΣ�ͨ���̶�ֵ20�ֽ�
			iphdr_hlen = IPH_HL(iphdr);
			iphdr_hlen *= 4;
			
			// ��ȡIP��ͷ�е�IP���ܳ����ֶΣ�ȷ��С�ڵݽ�������pbuf���е��ܳ���
			iphdr_len = ntohs(IPH_LEN(iphdr));
			if(iphdr_len > p->len || iphdr_len > p->tot_len)
			{
				pbuf_free(p);
				return ERR_OK;	
			}
			
			// У��IP���ݰ�ͷ
			if (inet_chksum(iphdr, iphdr_hlen) != 0) 
			{
				pbuf_free(p);
				return ERR_OK;	
			}
			
			// ��IP���ݱ����нضϣ��õ�����������IP���ݰ�
			pbuf_realloc(p, iphdr_len);
			
			// ����netif_list����(ϵͳ����2�������豸����ζ����2��netif�ֱ������������ǣ�Ҳ��ζ�ű�����2��IP��ַ,���Դ�ʱ����Ҫ����)�����IP���ݰ��е�Ŀ��IP�Ƿ��뱾�����������������ת��
			ip_addr_copy(current_iphdr_dest, iphdr->dest);
			ip_addr_copy(current_iphdr_src, iphdr->src);
			int first = 1;
			netif = inp;
			do{
				// ͨ��netif->flag��־λ�жϸ������豸�Ƿ�������ʹ��,ͬʱ�жϱ���IP�Ƿ���Ч
				if ((netif_is_up(netif)) && (!ip_addr_isany(&(netif->ip_addr)))) 
				{
					// ���Ŀ��IP��ַ�뱾��IP��ַƥ�����Ŀ��IP��ַ�ǹ㲥���ͣ���ζ�ųɹ�ƥ�䣬�˳�����
					if(ip_addr_cmp(&current_iphdr_dest, &(netif->ip_addr)) || ip_addr_isbroadcast(&current_iphdr_dest, netif))	
					{
						break;	
					}
				}
				if (first) 
				{
					first = 0;
					netif = netif_list;
				} 
				else 
				{
					netif = netif->next;
				}
			  	if (netif == inp) 
			  	{
			    	netif = netif->next;
			  	}
			}while(netif != NULL);
			
			//  ��������ݰ��е�ԴIP��ַ�ǹ㲥IP����ֱ�Ӷ���
			if ((ip_addr_isbroadcast(&current_iphdr_src, inp)) || (ip_addr_ismulticast(&current_iphdr_src))) 
			{
				pbuf_free(p);
				return ERR_OK;
			}
			
			// ��������Ժ��������û���ҵ�ƥ���netif�ṹ�壬˵�������ݰ����Ǹ������ģ�ת������(����ֱ�Ӷ���)
  			if (netif == NULL)
  			{
  				pbuf_free(p);
	    		return ERR_OK;
  			}
  			
  			// �жϸ�IP���Ƿ��Ƿ�Ƭ���ݰ�
  			// ����Ƿ�Ƭ���ݰ�������Ҫ���÷�Ƭ���ݴ棬�Ƚ��������з�Ƭ����ͳһ���������ݰ��ύ���ϲ�
  			if ((IPH_OFFSET(iphdr) & PP_HTONS(IP_OFFMASK | IP_MF)) != 0)
  			{
  				// ������������յ��ķ�Ƭ���������û����������p=NULL
  				p = ip_reass(p);
			    // �����Ƭ����û��������������������
			    if (p == NULL) 
			    {
			      return ERR_OK;
			    }
			    // �����Ƭ��������������ʱ��p�Ѿ���һ�����������ݰ��ṹ����
				// �ٴ�p�л�ȡ������IP��
				iphdr = (struct ip_hdr *)p->payload;			
  			}
  			
			// �ܵ�����һ�������ݰ���Ȼ��δ��Ƭ�Ļ򾭹���Ƭ��������������ݰ�
			current_netif = inp;
  			current_header = iphdr;
  			if (raw_input(p, inp) == 0)
  			{
  				// ����IP���ݰ�ͷ�е�Э���ֶ��жϸ����ݰ�Ӧ�ñ��ݽ����ϲ��ĸ�Э��
  				switch (IPH_PROTO(iphdr)) 
  				{
  					case IP_PROTO_UDP:	// UDPЭ��
  						udp_input(p, inp);	// ��������봫��㣬����������
	      				break;	
	      			case IP_PROTO_TCP:	// TCPЭ��
	      				tcp_input(p, inp);	// ��������봫��㣬����������
						break;
					case IP_PROTO_ICMP:	// ICMPЭ��
						icmp_input(p, inp);
						break;
					case IP_PROTO_IGMP:	// IGMPЭ��
						igmp_input(p, inp, &current_iphdr_dest);
						break;
					default:			// ���������
						// ������ǹ㲥���ݰ�������һ��Э�鲻�ɴ�ICMP���ݰ���Դ����
						if (!ip_addr_isbroadcast(&current_iphdr_dest, inp) && !ip_addr_ismulticast(&current_iphdr_dest)) 
						{
							p->payload = iphdr;
							icmp_dest_unreach(p, ICMP_DUR_PROTO);
						}	
						pbuf_free(p);		
  				}	
  			}
			
			current_netif = NULL;
			current_header = NULL;
			ip_addr_set_any(&current_iphdr_src);
			ip_addr_set_any(&current_iphdr_dest);
		}
											
		IP��Ĳ���Э�飺ICMP��IGMP
			��ʱ������Aѧ��������B��MAC��ַ���Ͱ����MAC��װ��ICMPЭ����������B���ͣ����ĸ�ʽ���£�
				��ͷ14�ֽ�										����ΪICMPЭ������������Э�飬����֡������0x0800
			+	ICMPЭ��ͷ����Ҫ�Ƕ���Э�����͡�ԴIP��Ŀ��IP��	������Э������ICMP��Ӧֵ0x01
			+	ICMPЭ�����壨��Ҫ��һ�����					�����ȡֵ0x00 - ����һ����Ӧ��Ϣ	0x03 - Ŀ�Ĳ��ɴ�	0x08 - �����Ӧ��Ϣ 
			
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
��.����㣨������Ҫ��TCP��
		TCP���ӵĽ������̣��������֣���
			1. �ͻ��˷���һ��SYN��־��1��TCP���ݱ������ְ���ָ��Դ�˿ں�Ŀ�Ķ˿ڣ�ͬʱ��֪�ͻ��˳�ʼ���seqno_client
			2. �����������յ������ݰ���������Ҳ����һ��SYN��־��1�����ݱ���ΪӦ��Ӧ���а����������˳�ʼ���seqno_server��ͬʱ��ACK��־��1����ȷ���������Ϊseqno_client+1
			3. ���ͻ��˽��յ�����˵�SYNӦ��������ٴβ���һ�����ְ�������ACK��־��1��ȷ���������Ϊseqno_server+1
		TCP���ӵĶϿ����̣��Ĵ����֣���
			1. ���ͻ���Ӧ�ó�������ִ�йرղ���ʱ���ͻ��˻������������һ��FIN��־��1�ı��ĶΣ������رմӿͻ��˵������������ݴ��ͣ��ñ��Ķ�����ֶ�Ϊseqno_client
			2. �����������յ����FIN���Ķκ󣬷���һ��ACK���ģ�ȷ�����Ϊseqno_client+1,���ͻ����յ����ACK�󣬴ӿͻ��˵���������������ӾͶϿ���
			3. ������TCP�����ϲ�Ӧ�ó���ͨ���ͻ��˵Ķ˿ڲ�������ᵼ�·�����Ӧ�ó���ر��������ӣ�ͬ������ʱһ��FIN��1�ı��Ķν��������ͻ��ˣ��ñ��Ķ�����ֶ�Ϊseqno_server
			4. ���ͻ����յ����FIN���Ķκ�Ҳ�᷵��һ��ACK��Ϊ��Ӧ��ȷ�����Ϊseqno_server+1,�ӷ��������ͻ��˷��������Ҳ�ͱ��Ͽ���
			
		******************************************************************************************************************************************************************************************************	
		lwipһ��������11��TCP״̬��
							enum tcp_state{
								CLOSED		= 0,	// û������
								LISTEN		= 1,	// ��������������״̬���ȴ��ͻ��˵���������
								SYN_SENT	= 2,	// ���������ѷ��ͣ��ȴ�ȷ��
								SYN_RCVD	= 3,	// ���յ��Է�����������
								ESTABLISHED = 4,	// �����ѽ���
								FIN_WAIT_1	= 5,	// �����ѹرո�����
								FIN_WAIT_2	= 6,	// ��һ���ѽ��ܹرո�����
								CLOSE_WAIT	= 7,	// �ȴ�����ر�����
								CLOSING		= 8,	// ����ͬʱ�յ��Է��Ĺر�����
								LAST_ACK	= 9,	// �������ȴ��Է����ܹرղ���
								TIME_WAIT	= 10,	// �رճɹ����ȴ������п��ܳ��ֵ�ʣ������
							}
							
		��������TCP״̬ת��·����
			1.��һ��·�������˿ͻ������뽨��������Ͽ����ӵ��������̣�
												CLOSED ����������������> SYN_SENT ����������������> ESTABLISHED ��������> FIN_WAIT_1 ��������> FIN_WAIT_2 ������������> TIME_WAIT ����> CLOSED
														������/syn				syn+ack/ack					  /fin					ack/				fin/ack
			2.�ڶ���·�������˷���������������Ͽ����ӵ��������̣�
												CLOSED ����������������> LISTEN ����������������> SYN_RCVD ��������> ESTABLISHED ����������������> CLOSE_WAIT ��������> LAST_ACK ��������> CLOSED
														������/					syn/syn+ack				ack/					fin/ack					  	/fin				ack/
		
		******************************************************************************************************************************************************************************************************													
		lwipʹ��һ��tcp_hdr�Ľṹ��������tcpЭ���ͷ��
												struct tcp_hdr{
													u16_t src;					// Դ�˿�
													u16_t dest;					// Ŀ�Ķ˿�
													u32_t seqno;				// ��ţ�������ʶ��TCP���Ͷ˵����ն˵������ֽ���
													u32_t ackno;				// ȷ����ţ��Ƿ���ȷ�ϵ�һ���������յ�����һ�����
													u16_t _hdrlen_rsvd_flags;	// ����4λTCP��ͷ����ͨ��Ϊ5*4�������ṹ���С����6����־λ��URG��ACK��PSH��RST��SYN��FIN��
													u16_t wnd;					// ���ڴ�С�ֶΣ���ʾ���ܽ��յ��ֽ�����ʵ����������
													u16_t chksum;				// 16λ����TCP����У��ͣ�������TCPͷ��TCP���ݣ��ɷ��Ͷ˼��㲢�ɽ��ն���֤
													u16_t urgp;					// ����ָ�룬����
												}
												
		******************************************************************************************************************************************************************************************************											
		lwipʹ��һ��tcp_pcb���ƿ�������һ��TCP����(lwipʵ�ʶ�����2��TCP���ƿ飬һ��ר��������������LISTEN״̬�����ӣ���һ������������������״̬������)��
												struct tcp_pcb{
													IP_PCB;						// �ú����������ӵ�IP�����Ϣ����Ҫ����ԴIP��Ŀ��IP������Ҫ�ֶ�	
													
													// �ⲿ����2������TCP���ƿ鶼���е��ֶ�										
													struct tcp_pcb *next;		// ָ����һ��tcp_pcb���ƿ������ָ��
													enum tcp_state state;		// TCP���ӵ�״̬������������11��
													u8_t prio;					// �ÿ��ƿ�����ȼ��������ڻ��յ����ȼ����ƿ�
													void *callback_arg;			// ָ���û��Զ������ݣ��ں����ص�ʱʹ��
													tcp_accept_fn accept;		// ����acceptʱ�ص�����
													u16_t local_port;			// �󶨵ı��ض˿�
															
													u16_t remote_port;			// Զ�̶˿�
													u8_t flags;					// ���ƿ�״̬����־�ֶΣ������˵�ǰ���ƿ�����ԣ���λ�ĺ������º궨��
													#define TF_ACK_DELAY	0x01	// �ӳٷ���ACK
													#define TF_ACK_NOW		0x02	// ��������ACK
													#define TF_INFR			0x04	// ���Ӵ��ڿ��ش�״̬
													#define TF_TIMESTAMP	0x08	// ���ӵ�ʱ���ѡ����ʹ��
													#define TF_RXCLOSED 	0x10	// ��TCP���ӶϿ�����RX�ر�
													#define TF_FIN			0x20	// Ӧ�ó����ѹرո�����
													#define TF_NODELAY		0x40	// ��ֹNagle�㷨
													#define TF_NAGLEMEMERR	0x80	// ���ػ��������
													
													// ��������ֶ�
													u32_t rcv_nxt;				// �������յ���һ����ţ�Ҳ���Ǳ��ؽ�Ҫ�������Է���ACK����ţ�Ҳ�Ǳ��ؽ��մ��ڵ���߽�
													u16_t rcv_wnd;				// ��ǰ���մ��ڴ�С�����������ݵĽ�����ݽ���̬�仯
													u16_t rcv_ann_wnd;			// ����Է�ͨ��Ĵ��ڴ�С��Ҳ���������ݵĽ�����ݽ���̬�仯
													u32_t rcv_ann_right_edge;	// ��һ�δ���ͨ��ʱ���ڵ��ұ߽�ֵ
													
													// ʱ������ֶ�
													u32_t tmr;					// ������������������tmr��ֵ��ʵ��		
  													u8_t polltmr, pollinterval;	// �������ֶ����������Ե���һ��������polltmr�����������ӣ�������pollintervalʱ��poll�����ᱻ����
  													s16_t rtime;				// �ش���ʱ����������rto��ֵʱ���ش�����
  													u16_t mss;					// �Է��ɽ��յ�����Ĵ�С
  													
  													// RTT������صĲ���
  													u32_t rttest;
  													u32_t rtseq;
  													s16_t sa, sv;
  													
  													s16_t rto;					// �ش���ʱʱ�䣬ʹ������3��RTT�����������
  													u8_t nrtx;					// �ش�����
  													
  													// �����ش���ָ�����ֶ�
  													u32_t lastack;				// ���յ�����һ��ȷ����ţ�Ҳ�������ȷ�����
  													u8_t dupacks;				// �������ȷ����ű��ظ��յ��Ĵ���	
  													
  													// ����������ز���
  													u16_t cwnd;  				// ���ӵ�ǰ���������ڴ�С
  													u16_t ssthresh;				// ӵ�������㷨������ֵ
  													
  													// ��������ֶ�
  													u32_t snd_nxt;				// ��һ����Ҫ���͵����
  													u16_t snd_wnd;				// ��ǰ���ʹ��ڴ�С
  													u32_t snd_wl1, snd_wl2;		// �ϴδ��ڸ���ʱ�յ����������seqno��ȷ�Ϻ�ackno
  													u32_t snd_lbb; 				// ��һ���������Ӧ�ó������ݵı��
  													
  													u16_t acked;				// �����˱�ȷ�ϵ��ѷ��ͳ���
  													u16_t snd_buf; 				// ���õķ��Ϳռ䣨���ֽ�Ϊ��λ��
  													u16_t snd_queuelen;			// ��ռ�õķ��Ϳռ䣨�����ݶ�pbufΪ��λ��
  													u16_t unsent_oversize;		// ��δ�����͵��ֽ���
  													struct tcp_seg *unsent;		// δ���͵����ݶζ��У�������ʽ
  													struct tcp_seg *unacked;	// ������δ�յ�ȷ�ϵ����ݶζ��У�������ʽ
  													struct tcp_seg *ooseq;		// ���յ����������������ݶζ��У�������ʽ
  													
  													struct pbuf *refused_data;	// ָ����һ�γɹ����յ�δ��Ӧ�ò�ȡ�õ�����pbuf
  													
  													// �ص�����
  													err_t (*sent)(void *arg,struct tcp_pcb *pcb,u16_t space);			// ���ݳɹ����ͺ󱻵���		
  													err_t (*recv)(void *arg,struct tcp_pcb,struct pbuf *p,err_t err);	// ���յ����ݺ󱻵���
  													err_t (*connected)(void *arg, struct tcp_pcb *tpcb, err_t err);		// ���ӽ����󱻵���
  													err_t (*poll)(void *arg, struct tcp_pcb *tpcb);						// �ú������ں������Ե���
  													void  (*errf)(void *arg, err_t err);								// ���ӷ�������ʱ������
  													
  													// ������ز���
  													u32_t keep_idle;			// ���һ���������Ľ����������ʱ����������������ʱ����
  													u32_t keep_intvl;			// �����ʱ�������������ͼ��
  													u32_t keep_cnt;				// �����ʱ��������������ط�����	��
  													u32_t persist_cnt;			// ��ֶ�ʱ������ֵ
  													u8_t persist_backoff;		// ��ֶ�ʱ�����أ�����0����
  													u8_t keep_cnt_sent;			// �����ʱ��������������ط�����	��
												}	
												struct tcp_pcb_listen{
													IP_PCB;
													struct tcp_pcb *next;		
													enum tcp_state state;		
													u8_t prio;					
													void *callback_arg;	
													tcp_accept_fn accept;		
													u16_t local_port;
												}
			ע��#define IP_PCB 	ip_addr_t local_ip;		// ����IP
								ip_addr_t remote_ip;	// Ŀ��IP
								u8_t 	  so_options;	// �׽���ѡ��	��ȡֵ:	#define SOF_ACCEPTCONN    (u8_t)0x02U
																				#define SOF_REUSEADDR     (u8_t)0x04U
																				#define SOF_KEEPALIVE     (u8_t)0x08U
																				#define SOF_BROADCAST     (u8_t)0x20U
																				#define SOF_LINGER        (u8_t)0x80U 
																				#define SOF_INHERITED     (SOF_REUSEADDR|SOF_KEEPALIVE|SOF_LINGER)
								u8_t	  tos;			// ��������
								u8_t	  ttl;			// TTL
			���TCP���ƿ�������TCPЭ��ĺ��ģ�TCPЭ��ʵ�ֵı��ʾ��Ƕ�TCP���ƿ��и��ֶεĲ��������Էǳ���Ҫ!!!
		
		******************************************************************************************************************************************************************************************************	
		tcp_input��TCP��������뺯��������Ϊ���ݰ�Ѱ��һ��ƥ���TCP���ƿ飬�Լ�������Ӧ�ĺ���tcp_timewait_input��tcp_listen_input��tcp_process���д���
		void tcp_input(struct pbuf *p,struct netif *inp)
		{
			struct tcp_pcb 	*pcb,*prev;
			struct tcp_pcb_listen *lpcb;
			u8_t hdrlen;
			err_t err;
			
			// �Թ�IP��ͷ����ȡTCPͷ
			iphdr = (struct ip_hdr *)p->payload;
			tcphdr = (struct tcp_hdr *)((u8_t *)p->payload + IPH_HL(iphdr)*4)
			
			// �ƶ�pbuf�ṹ�е����ݰ�ָ�룬ʹָ��TCPͷ
			if (pbuf_header(p, -((s16_t)(IPH_HL(iphdr) * 4))) || (p->tot_len < sizeof(struct tcp_hdr))) 
			{
				pbuf_free(p);
				return;	
			}
			
			// ����������Ĺ㲥��
			if (ip_addr_isbroadcast(&current_iphdr_dest, inp) || ip_addr_ismulticast(&current_iphdr_dest))
			{
				pbuf_free(p);
				return;		
			}
			
			// ��֤TCPУ���
			if (inet_chksum_pseudo(p, ip_current_src_addr(), ip_current_dest_addr(),IP_PROTO_TCP, p->tot_len) != 0)
			{
				pbuf_free(p);
    			return;	
			}
			
			// �����ƶ�pbuf�ṹ�е����ݰ�ָ�룬ʹָ��TCP����
			hdrlen = TCPH_HDRLEN(tcphdr);
			if(pbuf_header(p, -(hdrlen * 4))
			{
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
			tcplen = p->tot_len + ((flags & (TCP_FIN | TCP_SYN)) ? 1 : 0);	// TCP���ݰ������ݵ��ܳ��ȣ�������FIN��SYN��־�����ݰ����ó���Ҫ��1
			
			// ���¾��ǶԽ��յ������ݰ����з��ദ��Ҳ����Ѱ�Һ��ʵĽӿڣ�����IP��port
			// ������tcp_active_pcbs ��������ң���û��ƥ���tcp_pcb
			prev = NULL;
			for(pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next)
			{
				if (pcb->remote_port == tcphdr->src && pcb->local_port == tcphdr->dest && ip_addr_cmp(&(pcb->remote_ip), &current_iphdr_src) && ip_addr_cmp(&(pcb->local_ip), &current_iphdr_dest)) 
				{
					// �ҵ�ƥ��Ľӿ�֮�󣬽���tcp_pcb��tcp_active_pcbs�������ȡ����Ȼ���˳�ѭ���������У���ʱpcb != NULL
					if (prev != NULL) 
					{
						prev->next = pcb->next;
						pcb->next = tcp_active_pcbs;
						tcp_active_pcbs = pcb;
					}	
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
	    			if (pcb->remote_port == tcphdr->src && pcb->local_port == tcphdr->dest && ip_addr_cmp(&(pcb->remote_ip), &current_iphdr_src) && ip_addr_cmp(&(pcb->local_ip), &current_iphdr_dest)) 
	    			{
	    				// ����TIME_WAIT״̬����(����������)��������ֱ�����ﷵ�ز�����������
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
	      				// Ȼ���ж�IP�Ƿ�ƥ�䣬������IPADDR_ANY�����κ�IP
				        if (ip_addr_cmp(&(lpcb->local_ip), &current_iphdr_dest) || ip_addr_isany(&(lpcb->local_ip))) 
				        {
				        	// �ҵ�ƥ��Ľӿ�֮���˳�ѭ���������У���ʱlpcb != NULL
				          	break;
				        }		
	      			}
	      			prev = (struct tcp_pcb *)lpcb;
	    		}
	    		
	    		// �������ж���tcp_listen_pcbs���Ƿ��ҵ�
	    		if (lpcb != NULL) 
	    		{
	    			// ����tcp_pcb��tcp_listen_pcbs.listen_pcbs�������ȡ��
	    			if (prev != NULL)
	    			{
	    				((struct tcp_pcb_listen *)prev)->next = lpcb->next;
	    				lpcb->next = tcp_listen_pcbs.listen_pcbs;
	    				tcp_listen_pcbs.listen_pcbs = lpcb;	
	    			}	
	    			
	    			// ����LISTEN״̬�������������ģ���������ֱ�����ﷵ�ز�����������
					tcp_listen_input(lpcb);
					pbuf_free(p);
					return;
	    		}
			}
			
			// �����tcp_active_pcbs���ҵ��ˣ��򾭹���������tcp_process
  			if (pcb != NULL) 
  			{
  				inseg.next = NULL;		// �رձ��Ķζ��й���
			    inseg.len = p->tot_len;	// ���øñ��Ķε����ݳ���
			    inseg.p = p;			// ���ñ��Ķ���������ͷָ��
			    inseg.tcphdr = tcphdr;	// ���Ķε�TCPͷ
		
			    recv_data = NULL;		// ���ݽ��ս���������ڸ�ȫ�ֱ�����Ȼ�����ϲ��ύ
			    recv_flags = 0;			// tcp_processִ�����Ľ�������ƿ��״̬��Ǩ�����ᱻ�����ڸ�ȫ�ֱ��������������ﱻ��0
				
				// tcp_pcb��refused_dataָ�����Ƿ񻹼�¼����δ���ϲ�ݽ�������
	    		if (pcb->refused_data != NULL) 
	    		{
	    			// �еĻ��ص��û�recv��������δ�ݽ�������
	    			TCP_EVENT_RECV(pcb, pcb->refused_data, ERR_OK, err);
	    			
	    			// �жϴ���recv�����Ĵ��������ɹ�refused_dataָ����գ���������ִ��tcp_process
					if (err == ERR_OK) 
					{
						pcb->refused_data = NULL;
					} 	
					// ʧ����ζ��tcp_pcb����ռ�������������հ����ٴ���ֱ�ӷ���
					else if ((err == ERR_ABRT) || (tcplen > 0)) 
					{
						pbuf_free(p);
						return;
					}
	    		}
	    		
	    		tcp_input_pcb = pcb;	// ��¼����ǰ���ĵĿ��ƿ�
	    		
	    		// ������ǽ���tcp_process������հ������ˣ����������ģ�,�ú���ʵ����TCP״̬ת������ 
			    err = tcp_process(pcb);
			    
			    // ������ֵΪERR_ABRT��˵�����ƿ��Ѿ�����ȫɾ��(tcp_abort()),ʲôҲ����Ҫ��
			    if (err != ERR_ABRT) 
			    {
			    	// ����ֵ��ΪERR_ABRTʱ���жϱ��Ĵ����3�ֽ��
					if (recv_flags & TF_RESET) 			// ���յ��Է��ĸ�λ����
					{
						// �ص��û���errf����
						TCP_EVENT_ERR(pcb->errf, pcb->callback_arg, ERR_RST);
						// ɾ�����ƿ�
						tcp_pcb_remove(&tcp_active_pcbs, pcb);
						// �ͷſ��ƿ�ռ�
						memp_free(MEMP_TCP_PCB, pcb);
					}	
					else if (recv_flags & TF_CLOSED) 	// ˫�����ӳɹ��Ͽ�
					{
						// ɾ�����ƿ�
						tcp_pcb_remove(&tcp_active_pcbs, pcb);
						// �ͷſ��ƿ�ռ�
						memp_free(MEMP_TCP_PCB, pcb);
					} 
					else
					{
						err = ERR_OK;
						if (pcb->acked > 0) 			// ����б�ȷ�ϵ��ѷ������ݳ���		
						{
							// �ص��û���send����
						  	TCP_EVENT_SENT(pcb, pcb->acked, err);
						  	if (err == ERR_ABRT) 
						  	{
						    	goto aborted;
						  	}
						}
						
						if (recv_data != NULL)			// ��������ݱ����յ� 
						{
							if (pcb->flags & TF_RXCLOSED) 	// �������TCP���ƿ��Ѿ�����TF_RXCLOSED״̬����������յ������ݶ�����
							{
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
							if (err == ERR_ABRT) 
							{
								goto aborted;
							}
							
							// ����֮�⣬�������ֵ��ʧ�ܣ����ⲿ����δ���ϵݽ��������ݴ浽refused_dataָ����
							if (err != ERR_OK) 
							{
								pcb->refused_data = recv_data;
							}
						}
						
						if (recv_flags & TF_GOT_FIN)	// ����յ��Է���FIN���� 
						{
							// �������մ��� 
							if (pcb->rcv_wnd != TCP_WND) 
							{
								pcb->rcv_wnd++;
							}
							// ��һ��NULLָ��ص��û���recv������ͨ�����ַ�ʽ�û��������֪���Է��Ĺر�����
							TCP_EVENT_CLOSED(pcb, err);
							if (err == ERR_ABRT) 
							{
								goto aborted;
							}
						}
						
						tcp_input_pcb = NULL;		// ��ǰ���ĵ��˴�����ϣ���յ�ǰ���ĵĿ��ƿ�
						tcp_output(pcb);			// �������
					}
			    }
		aborted:
			    tcp_input_pcb = NULL;
			    recv_data = NULL;
		
			    if (inseg.p != NULL)
			    {
			      pbuf_free(inseg.p);
			      inseg.p = NULL;
			    }
			    
  			}
  			else
  			{
  				// �����3�������ﶼδ�ҵ�ƥ���pcb�������tcp_rst��Դ��������һ��TCP��λ���ݰ�
  				if (!(TCPH_FLAGS(tcphdr) & TCP_RST))
  				{
  					tcp_rst(ackno, seqno + tcplen,ip_current_dest_addr(), ip_current_src_addr(),tcphdr->dest, tcphdr->src);
  				}
  				
  				pbuf_free(p);
  			}
		}
		
		******************************************************************************************************************************************************************************************************	
		// �������Ǵ���LISTEN״̬�Ŀ��ƿ�����뱨�ĵĴ�����,����LISTEN״̬�Ŀ��ƿ�ֻ����ӦSYN���ְ�
		err_t tcp_listen_input(struct tcp_pcb_listen *pcb)
		{
			struct tcp_pcb *npcb;
			err_t rc;
			
			// ����listen״̬��pcbֻ����ӦSYN���ְ����Ժ���ACK��־�����뱨�ķ���һ��RST����
			if (flags & TCP_ACK) 
			{
				tcp_rst(ackno + 1, seqno + tcplen,ip_current_dest_addr(), ip_current_src_addr(),tcphdr->dest, tcphdr->src);
			}
			// ����listen״̬�ķ������˵ȵ���SYN���ְ�
			else if (flags & TCP_SYN)
			{
				// ����һ���µ�tcp_pcb����Ϊ����tcp_listen_pcbs�����ϵ�pcb��tcp_pcb_listen�ṹ�ģ������������ϵ�pcb��tcp_pcb�ṹ
	    		npcb = tcp_alloc(pcb->prio);	
	    		
	    		// ����½�ʧ�ܣ���������Ϊ�ڴ治��
	    		if (npcb == NULL) 
	    		{
	    			TCP_STATS_INC(tcp.memerr);
	      			  ERR_MEM;
	    		}
	    		
	    		// Ϊ����½���tcp_pcb����Ա
			    ip_addr_copy(npcb->local_ip, current_iphdr_dest);
			    npcb->local_port = pcb->local_port;
			    ip_addr_copy(npcb->remote_ip, current_iphdr_src);
			    npcb->remote_port = tcphdr->src;
			    npcb->state = SYN_RCVD;								// ����SYN_RCVD״̬
			    npcb->rcv_nxt = seqno + 1;							// �������յ�����һ����ţ�ע���1
			    npcb->rcv_ann_right_edge = npcb->rcv_nxt;			// ��ʼ���Ҳ�ͨ�洰��
			    npcb->snd_wnd = tcphdr->wnd;						// ����TCPͷ�жԷ��ɽ������ݳ��ȣ���ʼ�����ط��ʹ��ڴ�С
			    npcb->ssthresh = npcb->snd_wnd;						// ӵ���㷨��أ�����
			    npcb->snd_wl1 = seqno - 1;							// ��ʼ���ϴδ��ڸ���ʱ�յ������
			    npcb->callback_arg = pcb->callback_arg;				// ��ʼ���û��Զ�������
			    npcb->accept = pcb->accept;							// ��ʼ������acceptʱ�Ļص�����	
			    npcb->so_options = pcb->so_options & SOF_INHERITED;	// �̳�socketѡ��
			    
			    TCP_REG(&tcp_active_pcbs, npcb);					// ��������úõ�tcp_pcbע�ᵽtcp_active_pcbs������ȥ
			    tcp_parseopt(npcb);									// ���յ���SYN���ְ�����ȡTCPͷ��ѡ���ֶε�ֵ�������õ��Լ���tcp_pcb
			    npcb->mss = tcp_eff_send_mss(npcb->mss, &(npcb->remote_ip));	// ��ʼ��mss
			    
			    // �ظ�����SYN��ACK��־���������ݰ�
			    rc = tcp_enqueue_flags(npcb, TCP_SYN | TCP_ACK);
			    if (rc != ERR_OK) 
			    {
			      	tcp_abandon(npcb, 0);
			      	return rc;
			    }
			    
			    // TCP���������������������
	   			return tcp_output(npcb);
			}
			return ERR_OK;
		}
		
		******************************************************************************************************************************************************************************************************
		// �������Ǵ���TIMEWAIT״̬�Ŀ��ƿ鴦�����뱨�ĵĺ���
		err_t tcp_timewait_input(struct tcp_pcb *pcb)
		{
			// ��������к�RST��־��ֱ�Ӷ���
			if (flags & TCP_RST)  
			{
				return ERR_OK;	
			}	
			
			// ��������к�SYN��־
			if (flags & TCP_SYN) 
			{
				// ���SYN������ڽ��մ����ڣ�����һ��RST����
				if (TCP_SEQ_BETWEEN(seqno, pcb->rcv_nxt, pcb->rcv_nxt+pcb->rcv_wnd)) 
				{
					tcp_rst(ackno, seqno + tcplen, ip_current_dest_addr(), ip_current_src_addr(),tcphdr->dest, tcphdr->src);
	      			return ERR_OK;	
				}
			}
			// ��������к�FIN��־
			else if(flags & TCP_FIN)
			{
				pcb->tmr = tcp_ticks;
			}
			
			// ���TCP������������
			if(tcp_len > 0)
			{
				pcb->flags |= TF_ACK_NOW;	// ����ǰ���ƿ���ΪTF_ACK_NOW״̬
				
				// TCP���������������������
    			return tcp_output(pcb);		
			}
			
			return ERR_OK;
		}
		
		******************************************************************************************************************************************************************************************************	
		// ���˴���LISTEN��TIME_WAIT״̬����������״̬��pcb���ƿ飬�䱨�ĵ����봦��������ú�����Ҫʵ����TCP״̬ת������
		err_t tcp_process(struct tcp_pcb *pcb)
		{
			struct 	tcp_seg *rseg;
			u8_t	acceptable = 0;
			err_t	err;
			
			err	= ERR_OK;
			
			// �����жϸñ����ǲ���һ��RST����
			if(flags & TCP_RST)
			{
				// �жϸ�RST�����Ƿ�Ϸ�
				if (pcb->state == SYN_SENT) 	// ��һ����������Ӵ���SYN_SENT״̬
				{
					if (ackno == pcb->snd_nxt) 	// �����뱨���е�ȷ�Ϻž��ǿ��ƿ�����Ҫ���͵���һ�����
					{
						acceptable = 1;	
					}
				}
				else							// �ڶ������������״̬�£����뱨���е�����ڽ��մ�����
				{
					if (TCP_SEQ_BETWEEN(seqno, pcb->rcv_nxt,pcb->rcv_nxt+pcb->rcv_wnd)) 
					{
						acceptable = 1;
					}	
				}
				
				// ���RST���ĺϷ�������Ҫ��λ��ǰ���ӵĿ��ƿ飬�Ƿ���ֱ�ӷ��ز�������
				if (acceptable)
				{
					recv_flags |= TF_RESET;			// ���������뱨�ĵĴ������а���TF_RESET
		  			pcb->flags &= ~TF_ACK_DELAY;	// ��Ϊ������RST���ģ���ζ��ǰ���ƿ��Ȼ������TF_ACK_DELAY״̬
		  			return ERR_RST;	
				}
				else
				{
					return ERR_OK;
				}
			}
			
			// Ȼ�������ֱ���SYN���������Ѿ���������£������ǽ��յ��Է������ְ���˵���������һ����ʱ�ط������ְ���ֱ����Է�����һ��ACK����
			if ((flags & TCP_SYN) && (pcb->state != SYN_SENT && pcb->state != SYN_RCVD)) 
			{
				tcp_ack_now(pcb);					// #define tcp_ack_now(pcb) 	pcb->flags |= TF_ACK_NOW	- ����ǰ���ƿ���ΪTF_ACK_NOW״̬
				return ERR_OK;	
			}
			
			// TCP���Ӳ����ڰ�ر�ǰ���£����¿��ƿ�Ļ������
			if ((pcb->flags & TF_RXCLOSED) == 0) 
			{
				pcb->tmr = tcp_ticks;
			}
			
			// ����ļ�������0
  			pcb->keep_cnt_sent = 0;
  			
  			// �������ײ��е�ѡ���ֶΣ����ԣ�
  			tcp_parseopt(pcb);
  			
  			// ���ݵ�ǰ�����Ĳ�ͬ��TCP״ִ̬����Ӧ����
  			switch (pcb->state) 
  			{
  				case SYN_SENT:	// �ͻ��˷���SYN�󣬾ʹ��ڸ�״̬�ȴ�����������SYN+ACK
  					// ����յ�����SYN+ACK�������뱨���е�ȷ�Ϻţ����ǿ��ƿ����ѷ��ͣ�����δ�յ�Ӧ���Ķ��е����+1
  					if ((flags & TCP_ACK) && (flags & TCP_SYN) && ackno == ntohl(pcb->unacked->tcphdr->seqno) + 1)
  					{
  						pcb->snd_buf++;							// ����SYN�����ص�ACKȷ�ϣ��ͷ�1�ֽڿռ䣬���Կ��õķ��Ϳռ��1�ֽ�	
  						pcb->rcv_nxt = seqno + 1;				// �������յ���һ����ţ������ն����Ͷ�ACK�����е�ȷ�Ϻ�
  						pcb->rcv_ann_right_edge = pcb->rcv_nxt;	// ��ʼ��ͨ�洰�ڵ��ұ߽�ֵ���Դ����ʣ�
  						pcb->lastack = ackno;					// ���½��յ������ȷ�Ϻ��ֶΣ�Ҳ���Ǹ�����һ��ȷ�Ϻ��ֶ�
  						pcb->snd_wnd = tcphdr->wnd;				// ���ʹ�������Ϊ���մ��ڴ�С��ʵ����������
  						pcb->snd_wl1 = seqno - 1; 				// �ϴδ��ڸ���ʱ�յ����������
  						pcb->state = ESTABLISHED;				// ����ESTABLISHED״̬
  						
  						pcb->mss = tcp_eff_send_mss(pcb->mss, &(pcb->remote_ip));	// ���㲢��������Ķ�
  						pcb->ssthresh = pcb->mss * 10;								// ����mss��ssthreshֵҲҪ��Ӧ�޸�
  						pcb->cwnd = ((pcb->cwnd == 1) ? (pcb->mss * 2) : pcb->mss);	// ��ʼ����������
  							
  						--pcb->snd_queuelen;			// SYN�����ص�ACKȷ�ϣ�����ռ�õ�pbuf������1	
  						
  						rseg = pcb->unacked;			// �ӷ�����δ�յ�ȷ�ϵ����ݶζ�����ȡ��SYN���ģ��൱��ɾ��
  						pcb->unacked = rseg->next;		// ָ����һ��������δ�յ�ȷ�ϵ����ݶ�
  						if(pcb->unacked == NULL)		// ���δȷ�ϵ����ݶ�Ϊ�գ���ֹͣ�ش���ʱ��
							pcb->rtime = -1;
						else 							// ��������л��б��ģ���λ�ش���ʱ�����ش�����
						{
							pcb->rtime = 0;
							pcb->nrtx = 0;
						}
						
						tcp_seg_free(rseg);				// �ͷ�ȡ�µ�SYN���Ķ��ڴ�ռ�
						
						TCP_EVENT_CONNECTED(pcb, ERR_OK, err);	// �ص��û���connect�������������ģ�
						if (err == ERR_ABRT) 
						{
							return ERR_ABRT;
						}
						
						tcp_ack_now(pcb);				// �����������ACK���������ֽ��������庬���L753
  					}
  					// ���ֻ�յ��Է���ACKȴû��SYN������Է�����RST����
  					else if(flag & TCP_ACK)
  					{
  						tcp_rst(ackno, seqno + tcplen, ip_current_dest_addr(), ip_current_src_addr(),tcphdr->dest, tcphdr->src);
  					}
  					break;
  				case SYN_RCVD:	// ����������SYN+ACK�󣬾ʹ��ڸ�״̬���ȴ��ͻ��˷���ACK
  					// ����յ�ACK��Ҳ�����������ֵ����һ������
  					if(flags & TCP_ACK)
  					{
  						// ���ACK�Ϸ�
  						if (TCP_SEQ_BETWEEN(ackno, pcb->lastack+1, pcb->snd_nxt)) 
  						{
  							if (TCP_SEQ_BETWEEN(ackno, pcb->lastack+1, pcb->snd_nxt)) 
  							{
  								u16_t old_cwnd;
  								
  								pcb->state = ESTABLISHED;	// ����ESTABLISHED״̬
  								
  								TCP_EVENT_ACCEPT(pcb, ERR_OK, err);		// �ص��û���accept����
  								if (err != ERR_OK) 						// ���accept�������ش�����رյ�ǰ����
  								{
  									if (err != ERR_ABRT) 
  									{
									    tcp_abort(pcb);
									}
									return ERR_ABRT;
  								}
  								
  								old_cwnd = pcb->cwnd;		// ����ɵ���������
  								
  								tcp_receive(pcb);			// �����ACK�����л�Я�������ݣ������tcp_receive�������е����ݣ����������ģ� 
  								
  								// ��������δ��ȷ�ϵ��ֽ�������ΪSYN����ռ��1���ֽڣ����Լ�1
  								if (pcb->acked != 0) 		
								{
								  	pcb->acked--;				
								}
								
								pcb->cwnd = ((old_cwnd == 1) ? (pcb->mss * 2) : pcb->mss);	// ��ʼ����������
								
								// ����������tcp_receive�������а���FIN��־
						        if (recv_flags & TF_GOT_FIN) 
								{
						          	tcp_ack_now(pcb);			// �ظ�ACK����Ӧ�Է���FIN���ֱ�־
						          	pcb->state = CLOSE_WAIT;	// ����CLOSE_WAIT״̬
						        }
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
		      			tcp_rexmit(pcb);
		    		}
		    		break;
		    	case CLOSE_WAIT:	// ���������ڽ��չرյİ�����״̬����һֱ�ȴ��ϲ�Ӧ��ִ�йر�ָ�����FIN������״̬��ΪLASK_ACK
		    	case ESTABLISHED:	// ����˫���������ȶ�״̬
		    		tcp_receive(pcb);				// ���ú����������е�����
		    		
		    		// ����������tcp_receive�������а���FIN��־
		    		if (recv_flags & TF_GOT_FIN) 
		    		{
		    			tcp_ack_now(pcb);			// �ظ�ACK����Ӧ�Է���FIN���ֱ�־
		    			pcb->state = CLOSE_WAIT;	// ����CLOSE_WAIT״̬
		    		}
		    		break;
		    	case FIN_WAIT_1:	// �ϲ�Ӧ������ִ�йر�ָ�����FIN���ڸ�״̬(ͨ�����ڿͻ�������)
		    		tcp_receive(pcb);						// ���ú����������е�����
		    		
		    		// ����������tcp_receive�������а���FIN��־
		    		if (recv_flags & TF_GOT_FIN) 
		    		{
		    			// ����ñ���ͬʱ����һ���Ϸ�ACK,��ζ�ű��ض˽�ֱ������FIN_WAIT_2����TIME_WAIT״̬
	      				if ((flags & TCP_ACK) && (ackno == pcb->snd_nxt)) 
	      				{
	      					tcp_ack_now(pcb);				// �ظ�ACK
					        tcp_pcb_purge(pcb);				// ����������е������ִ�����
					        TCP_RMV(&tcp_active_pcbs, pcb);	// ��tcp_active_pcbs������ɾ���ÿ��ƿ�
					        pcb->state = TIME_WAIT;			// ����FIN_WAIT_2״̬��ֱ�ӽ���TIME_WAIT״̬
					        TCP_REG(&tcp_tw_pcbs, pcb);		// ���ÿ��ƿ����tcp_tw_pcbs����	
	      				}
	      				// ����ñ��Ĳ���ACK������ʾ˫��ͬʱִ���˹ر����Ӳ���
	      				else
	      				{
	      					tcp_ack_now(pcb);				// ����ACK
	        				pcb->state = CLOSING;			// ����CLOSING״̬
	      				}
		    		}
		    		// ���ֻ�յ���Ч��ACK
					else if ((flags & TCP_ACK) && (ackno == pcb->snd_nxt)) 
					{
		      			pcb->state = FIN_WAIT_2;			// ����FIN_WAIT_2״̬
				    }
				    break;	
				case FIN_WAIT_2:	// �����رգ�����FIN�������յ�ACK���ڸ�״̬		
					tcp_receive(pcb);						// ���ú����������е�����
					
					// ����������tcp_receive�������а���FIN��־
		    		if (recv_flags & TF_GOT_FIN) 
		    		{
		    			tcp_ack_now(pcb);					// �ظ�ACK
						tcp_pcb_purge(pcb);					// ����������е������ִ�����
						TCP_RMV(&tcp_active_pcbs, pcb);		// ��tcp_active_pcbs������ɾ���ÿ��ƿ�
						pcb->state = TIME_WAIT;				// ����TIME_WAIT״̬
						TCP_REG(&tcp_tw_pcbs, pcb);			// ���ÿ��ƿ����tcp_tw_pcbs����
		    		}
		    		break;
		    	case CLOSING:		// ˫��ͬʱִ�������رգ����ڸ�״̬(�������)
		    		tcp_receive(pcb);						// ���ú����������е�����
					
		    		// ����յ��Ϸ�ACK
					if (flags & TCP_ACK && ackno == pcb->snd_nxt) 
					{
						tcp_pcb_purge(pcb);					// ����������е������ִ�����
						TCP_RMV(&tcp_active_pcbs, pcb);		// ��tcp_active_pcbs������ɾ���ÿ��ƿ�
						pcb->state = TIME_WAIT;				// ����TIME_WAIT״̬
						TCP_REG(&tcp_tw_pcbs, pcb);			// ���ÿ��ƿ����tcp_tw_pcbs����
					}
		    		break;
		    	case LAST_ACK:		// ��������ִ�б����ر�ʱ��������FIN���ȴ�ACKʱ���ڸ�״̬
		    		tcp_receive(pcb);						// ���ú����������е�����
		    		
		    		// ����յ��Ϸ�ACK
					if (flags & TCP_ACK && ackno == pcb->snd_nxt) 
					{
						recv_flags |= TF_CLOSED;			// recv_flags����ΪTF_CLOSED����tcp_input�����Ըÿ��ƿ�����ͷź����
					}
					break;
				default:
					break;
  			}
  			return ERR_OK;
		}
		
		******************************************************************************************************************************************************************************************************	
		// ֻ�ᱻtcp_process�������ã����ڽ�һ����ɶ����뱨�ĵĴ���������˵���ú�����Ҫ��������뱨�ĵ�����ضϣ�����unacked��unsent��ooseq��������
		void tcp_receive(struct tcp_pcb *pcb)
		{
			struct tcp_seg *next;
			struct tcp_seg *prev, *cseg;
			struct pbuf *p;
			s32_t off;
			s16_t m;
			u32_t right_wnd_edge;	// ���ط��ʹ����ұ߽�
			u16_t new_tot_len;
			int found_dupack = 0;	// �ظ�ack��־����1��ʾ���ظ�ack	
			
			// ���ȼ�ⱨ���Ƿ����ACK��־
			if (flags & TCP_ACK) 
			{
				right_wnd_edge = pcb->snd_wl2 + pcb->snd_wnd;	// ��ȡ���ط��ʹ����ұ߽�
				
				// ��3��������Ե��±��ط��ʹ��ڸ���
				if (TCP_SEQ_LT(pcb->snd_wl1, seqno)||								// snd_wl1С����seqno��˵���Է��з�������
					(pcb->snd_wl1 == seqno && TCP_SEQ_LT(pcb->snd_wl2, ackno))||	// snd_wl1������seqno��snd_wl2С����ackno��˵���Է�û�з������ݣ�ֻ�����յ����ݺ���һ��ȷ��
					(pcb->snd_wl2 == ackno && tcphdr->wnd > pcb->snd_wnd)) 			// snd_wl2������ackno��snd_wndС�ڱ����ײ��Ĵ���ͨ��wnd��˵���ҷ�û�з����ݹ�ȥ�������Է���֪���մ��ڱ��	
				{
					pcb->snd_wnd = tcphdr->wnd;		// ���±��ط��ʹ��ڴ�С	�����Է������Ľ��մ���ͨ��ƥ��
					pcb->snd_wl1 = seqno;			// ���½��յ������
					pcb->snd_wl2 = ackno;			// ���½��յ���ȷ�Ϻ�
					
					// ������ʹ��ڷ�0����̽�쿪��
					if (pcb->snd_wnd > 0 && pcb->persist_backoff > 0) 
					{
						pcb->persist_backoff = 0;	// ֹͣ����̽��
					}
				}
				
				// �ж��Ƿ���һ���ظ���ACK����Ҫ����5������
				// 1.���acknoС�ڵ���lastack����û��ȷ��������
				if (TCP_SEQ_LEQ(ackno, pcb->lastack)) 						
				{
					pcb->acked = 0;		// û��ȷ�������ݣ���ôackedΪ0
					
					// 2.������Ķ���û������
					if (tcplen == 0) 
					{
						// 3.���ط��ʹ���û�и���
						if (pcb->snd_wl2 + pcb->snd_wnd == right_wnd_edge)
						{
							// 4.����ش���ʱ���������У����������������ȴ���ȷ��
							if (pcb->rtime >= 0) 
							{
								// 5.���ackno����lastack
								if (pcb->lastack == ackno)
								{
									// ��ʱ����ȷ������һ���ظ���ack��˵�����ķ����˶�ʧ
									found_dupack = 1;
									// ��ack���ظ��յ��Ĵ�������
									if (pcb->dupacks + 1 > pcb->dupacks)
										++pcb->dupacks;
									// �����ack�ظ��յ�����3�Σ�˵��������ӵ��
									if (pcb->dupacks > 3) 
									{
										if ((u16_t)(pcb->cwnd + pcb->mss) > pcb->cwnd) 
										{
											pcb->cwnd += pcb->mss;
										}
									}
									// �����ack�ظ���3���յ���ִ�п����ش��㷨
									else if (pcb->dupacks == 3)
									{
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
					// ������ƿ鴦�ڿ����ش�״̬	����ر��ش�״̬��ӵ������	
					if (pcb->flags & TF_INFR) 
					{
						pcb->flags &= ~TF_INFR;
						pcb->cwnd = pcb->ssthresh;
					}
					
					pcb->nrtx = 0;								// �ش�������0
					pcb->rto = (pcb->sa >> 3) + pcb->sv;		// ��λ�ش���ʱʱ��
					pcb->acked = (u16_t)(ackno - pcb->lastack);	// ����acked�ֶ�Ϊ��ȷ�ϵ��ѷ������ݳ���
					pcb->snd_buf += pcb->acked;					// ���¿��õķ��Ϳռ�
					pcb->dupacks = 0;							// ��ack�ظ��յ��Ĵ�����0
					pcb->lastack = ackno;						// ���½��յ���ackno
					
					// �������TCP�����Ѿ�����״̬������ӵ���㷨����ģ��
					if (pcb->state >= ESTABLISHED) 
					{
						if (pcb->cwnd < pcb->ssthresh) 
						{
						  	if ((u16_t)(pcb->cwnd + pcb->mss) > pcb->cwnd) 
							{
						    	pcb->cwnd += pcb->mss;
						  	}
						} 
						else 
						{
						  	u16_t new_cwnd = (pcb->cwnd + pcb->mss * pcb->mss / pcb->cwnd);
						  	if (new_cwnd > pcb->cwnd) 
							{
						    	pcb->cwnd = new_cwnd;
						  	}
						}
					}
					
					// ����unacked���У����������ݱ��С�ڵ���ackno�ı��Ķ��Ƴ�
					while (pcb->unacked != NULL && TCP_SEQ_LEQ(ntohl(pcb->unacked->tcphdr->seqno) + TCP_TCPLEN(pcb->unacked), ackno)) 
					{  	
						// ������Ҫ��ı��Ĵ�unacked����ȡ��
						next = pcb->unacked;
						pcb->unacked = pcb->unacked->next;
						
						// ����ñ��İ���FIN��־����ζ�ŵ�ǰ�յ���ACK��FIN����ȷ�ϣ���acked�ֶμ�1��������Ҫ�ύ�ϲ�ʹ֪��FIN���Է��ɹ�����
						if ((pcb->acked != 0) && ((TCPH_FLAGS(next->tcphdr) & TCP_FIN) != 0)) 
						{
						  	pcb->acked--;
						}
						
						pcb->snd_queuelen -= pbuf_clen(next->p);		// �ͷű��ñ���ռ�õķ��Ϳռ�
						tcp_seg_free(next);								// �ͷű��ñ���ռ�õ�tcp���Ķ�
					}
					
					// ����������Ҫ��ı��Ķ��Ƴ��ɹ����ж�unacked�����Ƿ�Ϊ��
					if(pcb->unacked == NULL)
						pcb->rtime = -1;	// ��Ϊ�գ��ر��ش���ʱ��
					else
						pcb->rtime = 0;		// ����λ�ش���ʱ��
		
					pcb->polltmr = 0;		// ��λ��ѯ��ʱ��
				}		
				// �����ACK�Ȳ����ظ�ACK���ֲ�������ACK����acked�ֶ���0������ACK��ȷ���κ��ѷ�������
				else 
				{
					pcb->acked = 0;
    			}
    			
    			// ����unsent���У����������ݱ��С�ڵ���ackno�ı��Ķ��Ƴ�
			    // ������Ϊ������Ҫ�ش��ı��ĶΣ�lwipֱ�ӽ����ǹ���unsent�����ϣ������յ���ACK�����Ƕ��ѳ�ʱ���Ķε�ȷ��
			    while (pcb->unsent != NULL && TCP_SEQ_BETWEEN(ackno, ntohl(pcb->unsent->tcphdr->seqno) + TCP_TCPLEN(pcb->unsent), pcb->snd_nxt)) 
			    {
					// ������Ҫ��ı��Ĵ�unsent����ȡ��
					next = pcb->unsent;
					pcb->unsent = pcb->unsent->next;
					
					// ����ñ��İ���FIN��־����ζ�ŵ�ǰ�յ���ACK��FIN����ȷ�ϣ���acked�ֶμ�1��������Ҫ�ύ�ϲ�ʹ֪��FIN���Է��ɹ�����
					if ((pcb->acked != 0) && ((TCPH_FLAGS(next->tcphdr) & TCP_FIN) != 0)) 
					{
						pcb->acked--;
					}
					
					pcb->snd_queuelen -= pbuf_clen(next->p);		// �ͷű��ñ���ռ�õķ��Ϳռ�
					tcp_seg_free(next);								// �ͷű��ñ���ռ�õ�tcp���Ķ�
				}
				
				// RTT���㣬����
	    		if (pcb->rttest && TCP_SEQ_LT(pcb->rtseq, ackno)) 
	    		{
	    			m = (s16_t)(tcp_ticks - pcb->rttest);
	    			m = m - (pcb->sa >> 3);
					pcb->sa += m;
					if (m < 0) {
					m = -m;
					}
					m = m - (pcb->sv >> 2);
					pcb->sv += m;
					pcb->rto = (pcb->sa >> 3) + pcb->sv;	
					pcb->rttest = 0;
	    		}
			}
			
			// ��������뱨�Ļ����������ݣ���Ҫ���������ݽ��д���
			if (tcplen > 0) 
			{
				// ���seqno + 1 <= rcv_nxt <= seqno + tcplen - 1����ζ���յ�����������ͷ������Ч���ݣ��յ��������в��ִ��ڱ��������մ����⣩����Ҫ�ض�����ͷ
				if (TCP_SEQ_BETWEEN(pcb->rcv_nxt, seqno + 1, seqno + tcplen - 1))
				{
					off = pcb->rcv_nxt - seqno;							// ��Ҫ�ص������ݳ���
					p = inseg.p;										// ��ȡ�յ��ı��Ķε�pbuf����ͷ
					
					// �ж���Ҫ�ضϵĳ����Ƿ񳬳��˵�һ��pbuf�д洢�����ݳ���
					if (inseg.p->len < off) 
					{
						new_tot_len = (u16_t)(inseg.p->tot_len - off);	// �ض��ظ����ݺ����Ч���ݳ���

						// �������������Ҫ����pbuf��������ժ�����ݣ�ֱ�����һ������ժ�����ݵ�pbuf
						while (p->len < off) 
						{
							off -= p->len;								// ʣ��ժ������
							p->tot_len = new_tot_len;					// ���µ�ǰpbuf�е������ܳ���
							p->len = 0;									// ��Ϊ���ݱ�ժ�������Ե�ǰpbuf�е����ݷֳ���0
							p = p->next;								// ָ����һ��pbuf
						}
						
						// �������һ������ժ�����ݵ�pbuf�����ǵ�������ָ���Թ�ժ������
						pbuf_header(p, (s16_t)-off);
					}
					else 
					{
						// ���δ�������������һ��pbuf�е�����ָ���Թ�ժ������
						pbuf_header(inseg.p, (s16_t)-off);
					}
					
					inseg.len -= (u16_t)(pcb->rcv_nxt - seqno);	// ����TCP���Ķ������ܳ�
					inseg.tcphdr->seqno = seqno = pcb->rcv_nxt;	// ����TCPͷ�е�seqno��ָ����մ���ͷλ��
				}
				else 
				{
					// ���seqno < rcv_nxt����ζ��seqno+tcplen-1 < rcv_nxt��˵�����Ǹ���ȫ�ظ��ı��Ķ�
					if (TCP_SEQ_LT(seqno, pcb->rcv_nxt))
					{
						tcp_ack_now(pcb);		// ֻ�ظ�һ��ACK���Է�(�����Ƿ�Ӧ��ֱ�ӷ��ز���������ȥ)
					}
				}
				
				// ���������ʼ����ڽ��մ�����
			    if (TCP_SEQ_BETWEEN(seqno, pcb->rcv_nxt, pcb->rcv_nxt + pcb->rcv_wnd - 1))
		        {
		        	// ����ñ������ݴ��ڽ�����ʼλ�ã���ζ�Ÿñ���������������
					if (pcb->rcv_nxt == seqno) 
					{
						tcplen = TCP_TCPLEN(&inseg);		// ���¸ñ��ĵ������ݳ���

						// ����ܳ����ڽ��մ��ڴ�С������Ҫ��β���ضϴ������������FIN��SYN���ֱ�־�Ĳ�ͬ��������ע�����
						if (tcplen > pcb->rcv_wnd) 
						{
							// ���TCPͷ�д�FIN��־�����FIN��־����Ϊ�Է���������Ҫ������
							if (TCPH_FLAGS(inseg.tcphdr) & TCP_FIN) 
							{
								TCPH_FLAGS_SET(inseg.tcphdr, TCPH_FLAGS(inseg.tcphdr) &~ TCP_FIN);
							}
							
							inseg.len = pcb->rcv_wnd;		// ���ݽ��մ��ڵ������ݳ���

							// ���TCPͷ�д�SYN��־�����Ķ����ݳ��ȼ�1
							if (TCPH_FLAGS(inseg.tcphdr) & TCP_SYN) 
							{
								inseg.len -= 1;
							}
							
							pbuf_realloc(inseg.p, inseg.len);	//  ��Ϊ���ݱ��ضϣ�pbuf�еĲ�����Ҫ��Ӧ����
							tcplen = TCP_TCPLEN(&inseg);		// �ٴθ��¸ñ��ĵ������ݳ���
						}
						
						// ��������Ķζ���ooseq�ϴ��ڱ��Ķ�
				        if (pcb->ooseq != NULL) 
						{
							// �жϵ�ǰ�����Ķε�TCPͷ���Ƿ��FIN��־
							if (TCPH_FLAGS(inseg.tcphdr) & TCP_FIN) 
							{
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
								// ����ooseq����ɾ����ű���ǰ�����Ķ���ȫ���ǵı��Ķ�
								while (next && TCP_SEQ_GEQ(seqno + tcplen,next->tcphdr->seqno + next->len)) 
								{
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
								
								// �����ǰ�����Ķ�β����ooseq�еı��Ķδ��ڲ����ص�	
								if (next && TCP_SEQ_GT(seqno + tcplen,next->tcphdr->seqno)) 
								{
									inseg.len = (u16_t)(next->tcphdr->seqno - seqno);	// �ضϵ�ǰ�����Ķ�β�����ص����֣��õ���Ч���ֳ���
									
									// �����ǰ�����Ķ�TCPͷ�д�SYN��־�����Ķ����ݳ��ȼ�1
									if (TCPH_FLAGS(inseg.tcphdr) & TCP_SYN) 
									{
										inseg.len -= 1;
									}
									
									pbuf_realloc(inseg.p, inseg.len);			//  ��Ϊ���ݱ��ضϣ�pbuf�еĲ�����Ҫ��Ӧ����
									tcplen = TCP_TCPLEN(&inseg);				// �ٴθ��¸ñ��ĵ������ݳ���
								}
								pcb->ooseq = next;
							}
						}
						
						pcb->rcv_nxt = seqno + tcplen;	// ������һ���������յ�����ţ�Ҳ���ǽ��մ�����߽�
						pcb->rcv_wnd -= tcplen;			// ���µ�ǰ���ý��մ���

		        		tcp_update_rcv_ann_wnd(pcb);	// ���¹��洰��
		        		
		        		// ����������Ķ��д�������
				        if (inseg.p->tot_len > 0) 
						{
							recv_data = inseg.p;		// ��ȫ��ָ��recv_dataָ���Ķ��е�����pbuf
							inseg.p = NULL;
						}
						
						// ����������Ķε�TCPͷ�д�FIN��־
				        if (TCPH_FLAGS(inseg.tcphdr) & TCP_FIN) 
						{
				          recv_flags |= TF_GOT_FIN;		// ���ڱ��Ĵ���������recv_flags���TF_GOT_FIN��־
				        }
				        
				        // ����ooseq���У�ȡ����������ı��Ķ�
				        // (ͨ���Ƚ�ooseq�����б��Ķε�seqno�͵�ǰTCP���ƿ��б����rcv_nxt���ж��ñ��Ķ��Ƿ�����)
				        while (pcb->ooseq != NULL && pcb->ooseq->tcphdr->seqno == pcb->rcv_nxt) 
						{
							cseg = pcb->ooseq;
							seqno = pcb->ooseq->tcphdr->seqno;	// �������
							pcb->rcv_nxt += TCP_TCPLEN(cseg);	// ������һ���������յ������
							pcb->rcv_wnd -= TCP_TCPLEN(cseg);	// ���µ�ǰ���ý��մ���
							tcp_update_rcv_ann_wnd(pcb);		// ���¹��洰��
							
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
								recv_flags |= TF_GOT_FIN;		// ��ȫ�ֱ���recv_flags���TF_GOT_FIN��־

								// �����ǰTCP����ESTABLISHED״̬������CLOSE_WAIT״̬
								if (pcb->state == ESTABLISHED) 
								{ 
									pcb->state = CLOSE_WAIT;
								}
							}
							
							pcb->ooseq = cseg->next;
							tcp_seg_free(cseg);
						}
						
						// ���϶�ִ����Ϻ���Դ�˷���һ��ACK���˴���ʵֻ������TCP���ƿ������ACK��־
						tcp_ack(pcb);
					}
					// ����ñ������ݲ����ڽ�����ʼλ�ã���ζ�Ÿñ��Ĳ��������
					else 
					{
						// ������Դ�˷���һ������ACK
		        		tcp_send_empty_ack(pcb);
		        		
		        		// Ȼ�󽫸ñ��Ķη���ooseq����
				        if (pcb->ooseq == NULL) 
						{
							// ���ooseqΪ�գ��򿽱��ñ��Ķε��¿��ٵı��Ķοռ䣬�����¿��ٱ��Ķ���Ϊooseq��ʼ��Ԫ
				          	pcb->ooseq = tcp_seg_copy(&inseg);
				        } 
						else 
						{
							prev = NULL;	// ����Ϊooseq��������һ�����ĶΣ������������
							// ����ooseq���У�ѡ�����λ�ò���ñ��Ķ�
							for(next = pcb->ooseq; next != NULL; next = next->next) 
							{
								// ���αȽ��������Ķε���ʼ���seqno��������
								if (seqno == next->tcphdr->seqno) 
								{
									// �����Ƚ��������Ķε����ݳ���
									if (inseg.len > next->len) 
									{
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
											
											tcp_oos_insert_segment(cseg, next);	// ����ò������ԭ�е���һ�����Ķε�Ӱ�죬����˵�������е����࣬�ͷ��ڴ�
										}
										break;	// �˳�ѭ��							
									}
									else
									{
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
											cseg = tcp_seg_copy(&inseg);			// ����Ҫ����ı��Ķε��¿��ٵı��Ķοռ�
											if (cseg != NULL) 
											{
												pcb->ooseq = cseg;					// ���±��Ķβ嵽ooseq��һ��λ��
												tcp_oos_insert_segment(cseg, next);	// ����ò������ԭ�еĵ�һ�����Ķε�Ӱ��
											}
											break;		// �˳�ѭ��
										}
									}
									// ������ǵ�һ��
									else 
									{
										// ��������뱨�Ķ���ʼ�����ǰһ���ͺ�һ�����Ķ���ʼ���֮��
										if (TCP_SEQ_BETWEEN(seqno, prev->tcphdr->seqno+1, next->tcphdr->seqno-1)) 
										{
											cseg = tcp_seg_copy(&inseg);	// ����Ҫ����ı��Ķε��¿��ٵı��Ķοռ�
											if (cseg != NULL) 
											{
												// �����ǰһ�����Ķ��������غ�
												if (TCP_SEQ_GT(prev->tcphdr->seqno + prev->len, seqno)) 
												{
													prev->len = (u16_t)(seqno - prev->tcphdr->seqno);	// �ض�ǰһ�����Ķ�β��
													pbuf_realloc(prev->p, prev->len);					// ��Ϊ���ݱ��ضϣ�pbuf�еĲ�����Ҫ��Ӧ����
												}
												
												prev->next = cseg;					// ���±��Ķβ���ǰһ�����Ķ�֮��
												tcp_oos_insert_segment(cseg, next);	// ����ò������ԭ�е���һ�����Ķε�Ӱ��
											}
											break;
										}
									}
									
									// ����Ѿ���ooseq�ϵ����һ�����Ķ�
									// �Ҵ�����ı��Ķ���ʼ��Ŵ��ڸñ�����ʼ���(��ʵ�������е������������Ȼ����)
									if (next->next == NULL && TCP_SEQ_GT(seqno, next->tcphdr->seqno)) 
									{
										// ����ñ��ĵ�TCPͷ����FIN��־����ֱ�Ӷ���������ı��ĶΣ��˳�ѭ��
										if (TCPH_FLAGS(next->tcphdr) & TCP_FIN) 
										{
											break;
										}
										
										next->next = tcp_seg_copy(&inseg);	// ����Ҫ����ı��Ķε��¿��ٵı��Ķοռ䣬�����ڶ���β��

										// ����²���ı��Ķβ�Ϊ��
										if (next->next != NULL) 
										{
											// �����ǰһ�����Ķ��������غ�
											if (TCP_SEQ_GT(next->tcphdr->seqno + next->len, seqno)) 
											{
												next->len = (u16_t)(seqno - next->tcphdr->seqno);	// �ض�ǰһ�����Ķ�β��
												pbuf_realloc(next->p, next->len);					// ��Ϊ���ݱ��ضϣ�pbuf�еĲ�����Ҫ��Ӧ����
											}
											
											// ����²���ı��Ķ����ݳ��ȳ����˵�ǰ���մ��ڴ�С
											if ((u32_t)tcplen + seqno > pcb->rcv_nxt + (u32_t)pcb->rcv_wnd) 
											{
												// ����²���ı��Ķε�TCPͷ����FIN��־
												if (TCPH_FLAGS(next->next->tcphdr) & TCP_FIN) 
												{
													TCPH_FLAGS_SET(next->next->tcphdr, TCPH_FLAGS(next->next->tcphdr) &~ TCP_FIN);	// ȥ��TCPͷ�е�FIN��־
												}
												
												next->next->len = pcb->rcv_nxt + pcb->rcv_wnd - seqno;	// ���ݽ��մ��ڴ�С�����²���ı��Ķ����ݳ���
												pbuf_realloc(next->next->p, next->next->len);			// ��Ϊ���ݱ��ضϣ�pbuf�еĲ�����Ҫ��Ӧ����
												tcplen = TCP_TCPLEN(next->next);						// �ٴθ��¸ñ��ĵ������ݳ���
											}
											
										}
										break;
									}
								}
								
								prev = next;	// ���϶������㣬�����ooseq��������һ��
							}
						}
					}
		        }
		        // ������ݲ��ڽ��շ�Χ��
				else 
				{
					tcp_send_empty_ack(pcb);	// ֱ����Դ�˷���һ������ȷ��ACK
				}
			}
			// �������ı��Ķ��в���������
			else 
			{
				// �����λ�ڽ��մ���֮��
			    if(!TCP_SEQ_BETWEEN(seqno, pcb->rcv_nxt, pcb->rcv_nxt + pcb->rcv_wnd-1))
			    {
			      	tcp_ack_now(pcb);		// ��һ��ACK
			    }
			}
		}
		
		err_t tcp_output(struct tcp_pcb *pcb)
		{
			struct tcp_seg *seg,*useg;
			u32_t wnd,snd_nxt;
			
			if(tcp_input_pcb == pcb)
			{
				return ERR_OK;	
			}
			 
			wnd = LWIP_MIN(pcb->snd_wnd,pcb->cwnd);
			seg = pcb->unsent;
			
			if(pcb->flags & TF_ACK_NOW && (seg = NULL || ntohl(seg->tcphdr->seqno)  ))
			{
					
			}
		}
		
		
		
		