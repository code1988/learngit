lwip从逻辑上看也是分为4层：链路层（ARP、RARP）、网络层(IP、（ICMP、IGMP这两个协议是网络层的补充协议，并不严格属于网络层）)、传输层(TCP、UDP)、应用层，基本等同TCP/IP，只是各层之间可以进行交叉存取，没有严格划分。
协议汇总：
	1. ARP协议：根据IP地址获取物理地址MAC的一个TCP/IP协议
一个典型的lwip系统包含3个进程：首先是上层应用程序进程，然后是lwip协议栈进程，最后是底层硬件数据包接收进程

动态内存管理：
	采用ucos-ii内存管理系统，即申请一块内存，分割成整数个大小相同的内存块
	
一.	链路层

	
	通过调用netif_add函数完成net_if结构体的初始化：
						/* Function Name: struct netif *netif_add(struct netif *netif,ip_addr *ipaddr,ip_addr *netmask,ip_addr *gw,void *state,err_t (* init)(struct netif *netif),err_t (*input)(struct pbuf *p,struct netif *inp))
						/* Input		: *state 											- 传入cpswportif地址
						**				  err_t (* init)(struct netif *netif) 				- 传入cpswif_init，底层接口驱动初始化函数
						**				  err_t (*input)(struct pbuf *p,struct netif *inp)	- 传入tcpip_input，底层网卡向IP层提交数据包的函数
						*/
	
	完成初始化后的net_if结构体会被链入链表，协议栈使用2个全局变量管理这张链表：netif_list指向链表表头，netif_default指向缺省的net_if结构体。
	
	以太网数据包基本格式：		目的MAC地址（6字节）	+ 源MAC地址（6字节） 	+ 类型（2字节） 	+ 数据（46-1500字节） 	+ 校验（4字节）
	以太网数据包最大帧长1518字节，最小帧长64字节，当无法满足最小64字节后，末尾用trailer字段来填充补足64字节
	lwip使用一个eth_hdr的结构体来描述以太网数据包包头的14个字节：
																struct eth_hdr{
																	struct eth_addr dest;	// 目的MAC地址
																	struct eth_addr src;	// 源MAC地址
																	u16_t type;				// 类型，主要用到2种：0x0800 - IP	0x0806 - ARP
																}
	lwip使用一个etharp_hdr的结构体来描述ARP协议包头：
													struct etharp_hdr{
														u16_t hwtype;				// 硬件接口类型	，以太网固定为0x0001
														u16_t proto;				// 高一层协议类型，IP固定为0x0800
														u8_t  hwlen;				// 硬件地址（MAC）长度，固定值6
														u8_t  protolen;				// 高一层协议地址（IP）长度，固定值4
														u16_t opcode;				// 操作码，1 - ARP请求	2 - ARP回复
														struct eth_addr shwaddr;	// 源MAC
														struct ip_addr2 sipaddr;	// 源IP
														struct eth_addr dhwaddr;	// 目的MAC
														struct ip_addr2 dipaddr;	// 目的IP
													}
																						
	当主机A要与主机B通信时，ARP协议可以将主机B的IP地址解析成主机B的MAC地址，工作流程如下：	
		第一步：主机A先检查自己的ARP缓冲，看是否存在主机B匹配的MAC地址，如果没有，就会向外广播一个ARP请求包
		第二步：其他主机收到后，发现请求的IP地址与自己的IP地址不匹配，则丢弃ARP请求
		第三步：主机B确定ARP请求中的IP地址与自己的IP地址匹配，则将主机A的IP地址和MAC地址映射到本地ARP缓存中
		第四步：主机B将包含其MAC地址的ARP回复发回给主机A
		第五步：主机A收到从主机B发来的ARP回复时，会将主机B的IP地址和MAC地址映射更新到本地ARP缓存中。主机B的MAC地址一旦确定，主机A就可以向主机B发送IP通信了
	
	连接链路层和网络层的纽带：以太网数据包接收进程tcpip_thread
		static void tcpip_thread(void *arg)
		{
			struct tcpip_msg *msg;		// 消息来自于网卡中断
			
			while(1)
			{
				// 该任务阻塞在这里接收要处理的消息，当有数据包到来时，网卡芯片中断函数接收数据，并post消息，中断退出后，该任务获取消息
				sys_timeouts_mbox_fetch(&mbox, (void **)&msg);	
				
				// 判断本条消息的类型，只关注数据包消息TCPIP_MSG_INPKT
				switch (msg->type)
				{
					case TCPIP_MSG_INPKT:											// 数据包消息
						if(msg->msg.inp.netif->flags & (NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET))
							ethernet_input（msg->msg.inp.p,msg->msg.inp.netif）;	// 如果支持ARP，先进行ARP处理，再判断是否递交IP层，对于IP数据包，这里2个选择最终都要调用ip_input进入IP层
						else
							ip_input(msg->msg.inp.p, msg->msg.inp.netif);			// 否则直接递交IP层,ip_input为IP层主要函数，解析见下文，这里直接调用ip_input存在问题，有误，需要先以太网数据包指针，使掠过包头，指向IP协议包头
						memp_free(MEMP_TCPIP_MSG_INPKT, msg);						// 释放消息内存
						break;
					case TCPIP_MSG_TIMEOUT:											// 超时消息
						sys_timeout(msg->msg.tmo.msecs, msg->msg.tmo.h, msg->msg.tmo.arg);
						memp_free(MEMP_TCPIP_MSG_API, msg);
						break;
					default:
						break;
				}
			}
		}
		err_t ethernet_input（struct pbuf *p,struct netif *netif)
		{
			struct eth_hdr *ethhdr;					// 以太网数据包头结构体
			u16_t type;
			s16_t ip_hdr_offset = SIZEOF_ETH_HDR;	// 包头固定值14字节
			
			ethhdr = (eth_hdr *)p->payload;
			type = htons(ethhdr->type);
			
			switch(type)
			{
				case ETHTYPE_IP:												// IP数据包
					etharp_ip_input(netif,p);									// 更新ARP表
					pbuf_header(p, -ip_hdr_offset);								// 调整以太网数据包指针，使掠过包头，指向IP协议包头
					ip_input(p,netif);											// 提交IP层，ip_input为IP层主要函数，解析见下文
				case ETHTYPE_ARP:												// ARP数据包
					etharp_arp_input(netif,(struct eth_addr *)netif->hwaddr,p);	// ARP数据包处理，第二个形参是本机MAC
					break;
				default:
					break;
			}
		}
		
		注：消息结构体struct tcpip_msg {
							enum tcpip_msg_type type;				// 本条消息的类型：TCPIP_MSG_INPKT - 数据包消息，TCPIP_MSG_TIMEOUT - 超时消息
							sys_sem_t *sem;							// 事件控制块ECB
							union{
								struct api_msg *apimsg;
								struct netifapi_msg *netifapimsg;
								struct {
									struct pbuf *p;
									struct netif *netif;
								} inp;								// inp结构体最重要，内含数据包内容结构、网络接口结构
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
二. 网络层
		lwip使用一个ip_hdr的结构体来描述IP协议包头：
												struct ip_hdr{
													u16_t _v_hl_tos;	// 包含4位版本号（IPv4 - 4，IPv6 - 6）、4位IP包头长（通常为5*4,即本结构体大小）、8位服务类型
													u16_t _len;			// 整个IP数据包长度
													u16_t _id;			// 16位标识用于标识IP层发出的每一份IP报文，自增
													u16_t _offset;		// 包含3位标志和13位片偏移，IP数据包分片时使用
													u8_t _ttl;			// TTL描述该IP数据包最多能被转发的次数，自减
													u8_t _proto;		// 协议字段用于描述该IP数据包的上层协议，0x01 - ICMP,0x02 - IGMP,0x06 - TCP,0x17 - UDP
													u16_t _chksum;		// 16位的IP首部校验和
													ip_addr_p_t src;	// 源IP
													ip_addr_p_t dest;	// 目的IP
												}
	
		ip_input为IP层主干函数，完成了IP层数据包处理(核心工作就是IP地址匹配；得到完整数据包)，然后将合适的数据包提交给上层，这里的p->payload已经越过了14字节包头，指向了IP头
		err_t ip_input(struct pbuf *p,struct netif *inp)
		{
			struct 	ip_hdr *iphdr;	// 指向IP包头的指针
			struct 	netif *netif;	// 指向netif硬件网络接口设备描述符的指针
			u16_t	iphdr_hlen;		// IP包头的长度，通常是固定20字节
			u16_t	iphdr_len;		// 整个IP包长，包含IP包头、上层协议头、数据
			
			// 取出 IP数据包头
			iphdr = (struct ip_hdr *)p->payload;
			
			// 检查IP包头中的版本号字段，IPv4 - 4，IPv6 - 6
			if(IPH_V(iphdr) != 4)
			{
				pbuf_free(p);
				return ERR_OK;	
			}
			
			// 提取IP包头中的头长度字段，通常固定值20字节
			iphdr_hlen = IPH_HL(iphdr);
			iphdr_hlen *= 4;
			
			// 提取IP包头中的IP包总长度字段，确保小于递交上来的pbuf包中的总长度
			iphdr_len = ntohs(IPH_LEN(iphdr));
			if(iphdr_len > p->len || iphdr_len > p->tot_len)
			{
				pbuf_free(p);
				return ERR_OK;	
			}
			
			// 校验IP数据包头
			if (inet_chksum(iphdr, iphdr_hlen) != 0) 
			{
				pbuf_free(p);
				return ERR_OK;	
			}
			
			// 对IP数据报进行截断，得到完整无冗余IP数据包
			pbuf_realloc(p, iphdr_len);
			
			// 遍历netif_list链表(系统存在2个网卡设备，意味着有2个netif分别用于描述它们，也意味着本机有2个IP地址,所以此时就需要遍历)，检测IP数据包中的目的IP是否与本机相符，不符则丢弃或转发
			ip_addr_copy(current_iphdr_dest, iphdr->dest);
			ip_addr_copy(current_iphdr_src, iphdr->src);
			int first = 1;
			netif = inp;
			do{
				// 通过netif->flag标志位判断该网卡设备是否配置且使能,同时判断本机IP是否有效
				if ((netif_is_up(netif)) && (!ip_addr_isany(&(netif->ip_addr)))) 
				{
					// 如果目的IP地址与本机IP地址匹配或者目的IP地址是广播类型，意味着成功匹配，退出遍历
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
			
			//  如果该数据包中的源IP地址是广播IP，则直接丢弃
			if ((ip_addr_isbroadcast(&current_iphdr_src, inp)) || (ip_addr_ismulticast(&current_iphdr_src))) 
			{
				pbuf_free(p);
				return ERR_OK;
			}
			
			// 遍历完成以后，如果依旧没有找到匹配的netif结构体，说明该数据包不是给本机的，转发或丢弃(这里直接丢弃)
  			if (netif == NULL)
  			{
  				pbuf_free(p);
	    		return ERR_OK;
  			}
  			
  			// 判断该IP包是否是分片数据包
  			// 如果是分片数据包，则需要将该分片包暂存，等接收完所有分片包后，统一将整个数据包提交给上层
  			if ((IPH_OFFSET(iphdr) & PP_HTONS(IP_OFFMASK | IP_MF)) != 0)
  			{
  				// 在这里重组接收到的分片包，如果还没接收完整，p=NULL
  				p = ip_reass(p);
			    // 如果分片包还没接收完整，本函数结束
			    if (p == NULL) 
			    {
			      return ERR_OK;
			    }
			    // 如果分片包接收完整，这时的p已经是一个完整的数据包结构体了
				// 再从p中获取完整的IP包
				iphdr = (struct ip_hdr *)p->payload;			
  			}
  			
			// 能到达这一步的数据包必然是未分片的或经过分片重组完整后的数据包
			current_netif = inp;
  			current_header = iphdr;
  			if (raw_input(p, inp) == 0)
  			{
  				// 根据IP数据包头中的协议字段判断该数据包应该被递交给上层哪个协议
  				switch (IPH_PROTO(iphdr)) 
  				{
  					case IP_PROTO_UDP:	// UDP协议
  						udp_input(p, inp);	// 从这里进入传输层，解析见下文
	      				break;	
	      			case IP_PROTO_TCP:	// TCP协议
	      				tcp_input(p, inp);	// 从这里进入传输层，解析见下文
						break;
					case IP_PROTO_ICMP:	// ICMP协议
						icmp_input(p, inp);
						break;
					case IP_PROTO_IGMP:	// IGMP协议
						igmp_input(p, inp, &current_iphdr_dest);
						break;
					default:			// 如果都不是
						// 如果不是广播数据包，返回一个协议不可达ICMP数据包给源主机
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
											
		IP层的补充协议：ICMP、IGMP
			这时候主机A学到了主机B的MAC地址，就把这个MAC封装到ICMP协议中向主机B发送，报文格式如下：
				包头14字节										：因为ICMP协议包属于网络层协议，所以帧类型是0x0800
			+	ICMP协议头（主要是二级协议类型、源IP、目的IP）	：二级协议类型ICMP对应值0x01
			+	ICMP协议主体（主要是一个类别）					：类别取值0x00 - 这是一条回应信息	0x03 - 目的不可达	0x08 - 请求回应信息 
			
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
三.传输层（这里主要讲TCP）
		TCP连接的建立过程（三次握手）：
			1. 客户端发送一个SYN标志置1的TCP数据报，握手包中指明源端口和目的端口，同时告知客户端初始序号seqno_client
			2. 当服务器接收到该数据包并解析后，也发回一个SYN标志置1的数据报作为应答，应答中包含服务器端初始序号seqno_server，同时将ACK标志置1，将确认序号设置为seqno_client+1
			3. 当客户端接收到服务端的SYN应答包，会再次产生一个握手包，包中ACK标志置1，确认序号设置为seqno_server+1
		TCP连接的断开过程（四次握手）：
			1. 当客户端应用程序主动执行关闭操作时，客户端会向服务器发送一个FIN标志置1的报文段，用来关闭从客户端到服务器的数据传送，该报文段序号字段为seqno_client
			2. 当服务器接收到这个FIN报文段后，返回一个ACK报文，确认序号为seqno_client+1,当客户端收到这个ACK后，从客户端到服务器方向的连接就断开了
			3. 服务器TCP向其上层应用程序通过客户端的端口操作，这会导致服务器应用程序关闭它的连接，同样，此时一个FIN置1的报文段将被发往客户端，该报文段序号字段为seqno_server
			4. 当客户端收到这个FIN报文段后，也会返回一个ACK作为响应，确认序号为seqno_server+1,从服务器到客户端方向的连接也就被断开了
			
		******************************************************************************************************************************************************************************************************	
		lwip一共定义了11种TCP状态：
							enum tcp_state{
								CLOSED		= 0,	// 没有连接
								LISTEN		= 1,	// 服务器进入侦听状态，等待客户端的连接请求
								SYN_SENT	= 2,	// 连接请求已发送，等待确认
								SYN_RCVD	= 3,	// 已收到对方的连接请求
								ESTABLISHED = 4,	// 连接已建立
								FIN_WAIT_1	= 5,	// 程序已关闭该连接
								FIN_WAIT_2	= 6,	// 另一端已接受关闭该连接
								CLOSE_WAIT	= 7,	// 等待程序关闭连接
								CLOSING		= 8,	// 两端同时收到对方的关闭请求
								LAST_ACK	= 9,	// 服务器等待对方接受关闭操作
								TIME_WAIT	= 10,	// 关闭成功，等待网络中可能出现的剩余数据
							}
							
		两条最经典的TCP状态转换路径：
			1.第一条路径描述了客户端申请建立连接与断开连接的整个过程：
												CLOSED ————————> SYN_SENT ————————> ESTABLISHED ————> FIN_WAIT_1 ————> FIN_WAIT_2 ——————> TIME_WAIT ——> CLOSED
														主动打开/syn				syn+ack/ack					  /fin					ack/				fin/ack
			2.第二条路径描述了服务器建立连接与断开连接的整个过程：
												CLOSED ————————> LISTEN ————————> SYN_RCVD ————> ESTABLISHED ————————> CLOSE_WAIT ————> LAST_ACK ————> CLOSED
														被动打开/					syn/syn+ack				ack/					fin/ack					  	/fin				ack/
		
		******************************************************************************************************************************************************************************************************													
		lwip使用一个tcp_hdr的结构体来描述tcp协议包头：
												struct tcp_hdr{
													u16_t src;					// 源端口
													u16_t dest;					// 目的端口
													u32_t seqno;				// 序号，用来标识从TCP发送端到接收端的数据字节流
													u32_t ackno;				// 确认序号，是发送确认的一段所期望收到的下一个序号
													u16_t _hdrlen_rsvd_flags;	// 包含4位TCP包头长（通常为5*4，即本结构体大小）、6个标志位（URG、ACK、PSH、RST、SYN、FIN）
													u16_t wnd;					// 窗口大小字段，表示还能接收的字节数，实现流量控制
													u16_t chksum;				// 16位整个TCP报文校验和，包含了TCP头和TCP数据，由发送端计算并由接收端验证
													u16_t urgp;					// 紧急指针，暂略
												}
												
		******************************************************************************************************************************************************************************************************											
		lwip使用一个tcp_pcb控制块来描述一个TCP连接(lwip实际定义了2种TCP控制块，一种专门用于描述处于LISTEN状态的连接，另一种用于描述处于其他状态的连接)：
												struct tcp_pcb{
													IP_PCB;						// 该宏描述了连接的IP相关信息，主要包含源IP、目的IP两个重要字段	
													
													// 这部分是2种类型TCP控制块都具有的字段										
													struct tcp_pcb *next;		// 指向下一个tcp_pcb控制块的链表指针
													enum tcp_state state;		// TCP连接的状态，如上所述共11种
													u8_t prio;					// 该控制块的优先级，可用于回收低优先级控制块
													void *callback_arg;			// 指向用户自定义数据，在函数回调时使用
													tcp_accept_fn accept;		// 连接accept时回调函数
													u16_t local_port;			// 绑定的本地端口
															
													u16_t remote_port;			// 远程端口
													u8_t flags;					// 控制块状态、标志字段，描述了当前控制块的特性，各位的含义如下宏定义
													#define TF_ACK_DELAY	0x01	// 延迟发送ACK
													#define TF_ACK_NOW		0x02	// 立即发送ACK
													#define TF_INFR			0x04	// 连接处于快重传状态
													#define TF_TIMESTAMP	0x08	// 连接的时间戳选项已使能
													#define TF_RXCLOSED 	0x10	// 因TCP连接断开导致RX关闭
													#define TF_FIN			0x20	// 应用程序已关闭该连接
													#define TF_NODELAY		0x40	// 禁止Nagle算法
													#define TF_NAGLEMEMERR	0x80	// 本地缓冲区溢出
													
													// 接收相关字段
													u32_t rcv_nxt;				// 期望接收的下一个序号，也即是本地将要反馈给对方的ACK的序号，也是本地接收窗口的左边界
													u16_t rcv_wnd;				// 当前接收窗口大小，会随着数据的接收与递交动态变化
													u16_t rcv_ann_wnd;			// 将向对方通告的窗口大小，也会随着数据的接收与递交动态变化
													u32_t rcv_ann_right_edge;	// 上一次窗口通告时窗口的右边界值
													
													// 时间相关字段
													u32_t tmr;					// 其它各计数器都基于tmr的值来实现		
  													u8_t polltmr, pollinterval;	// 这两个字段用于周期性调用一个函数，polltmr会周期性增加，当超过pollinterval时，poll函数会被调用
  													s16_t rtime;				// 重传定时器，当大于rto的值时则重传报文
  													u16_t mss;					// 对方可接收的最大报文大小
  													
  													// RTT估计相关的参数
  													u32_t rttest;
  													u32_t rtseq;
  													s16_t sa, sv;
  													
  													s16_t rto;					// 重传超时时间，使用上面3个RTT参数计算出来
  													u8_t nrtx;					// 重传次数
  													
  													// 快速重传与恢复相关字段
  													u32_t lastack;				// 接收到的上一个确认序号，也就是最大确认序号
  													u8_t dupacks;				// 上述最大确认序号被重复收到的次数	
  													
  													// 阻塞控制相关参数
  													u16_t cwnd;  				// 连接当前的阻塞窗口大小
  													u16_t ssthresh;				// 拥塞避免算法启动阈值
  													
  													// 发送相关字段
  													u32_t snd_nxt;				// 下一个将要发送的序号
  													u16_t snd_wnd;				// 当前发送窗口大小
  													u32_t snd_wl1, snd_wl2;		// 上次窗口更新时收到的数据序号seqno和确认号ackno
  													u32_t snd_lbb; 				// 下一个被缓冲的应用程序数据的编号
  													
  													u16_t acked;				// 保存了被确认的已发送长度
  													u16_t snd_buf; 				// 可用的发送空间（以字节为单位）
  													u16_t snd_queuelen;			// 被占用的发送空间（以数据段pbuf为单位）
  													u16_t unsent_oversize;		// 尚未被发送的字节数
  													struct tcp_seg *unsent;		// 未发送的数据段队列，链表形式
  													struct tcp_seg *unacked;	// 发送了未收到确认的数据段队列，链表形式
  													struct tcp_seg *ooseq;		// 接收到有序序号以外的数据段队列，链表形式
  													
  													struct pbuf *refused_data;	// 指向上一次成功接收但未被应用层取用的数据pbuf
  													
  													// 回调函数
  													err_t (*sent)(void *arg,struct tcp_pcb *pcb,u16_t space);			// 数据成功发送后被调用		
  													err_t (*recv)(void *arg,struct tcp_pcb,struct pbuf *p,err_t err);	// 接收到数据后被调用
  													err_t (*connected)(void *arg, struct tcp_pcb *tpcb, err_t err);		// 连接建立后被调用
  													err_t (*poll)(void *arg, struct tcp_pcb *tpcb);						// 该函数被内核周期性调用
  													void  (*errf)(void *arg, err_t err);								// 连接发生错误时被调用
  													
  													// 心跳相关参数
  													u32_t keep_idle;			// 最后一个正常报文结束到保活计时器（心跳）启动的时间间隔
  													u32_t keep_intvl;			// 保活计时器（心跳）发送间隔
  													u32_t keep_cnt;				// 保活计时器（心跳）最大重发次数	？
  													u32_t persist_cnt;			// 坚持定时器计数值
  													u8_t persist_backoff;		// 坚持定时器开关，大于0开启
  													u8_t keep_cnt_sent;			// 保活计时器（心跳）最大重发次数	？
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
			注：#define IP_PCB 	ip_addr_t local_ip;		// 本地IP
								ip_addr_t remote_ip;	// 目的IP
								u8_t 	  so_options;	// 套接字选项	可取值:	#define SOF_ACCEPTCONN    (u8_t)0x02U
																				#define SOF_REUSEADDR     (u8_t)0x04U
																				#define SOF_KEEPALIVE     (u8_t)0x08U
																				#define SOF_BROADCAST     (u8_t)0x20U
																				#define SOF_LINGER        (u8_t)0x80U 
																				#define SOF_INHERITED     (SOF_REUSEADDR|SOF_KEEPALIVE|SOF_LINGER)
								u8_t	  tos;			// 服务类型
								u8_t	  ttl;			// TTL
			这个TCP控制块是整个TCP协议的核心，TCP协议实现的本质就是对TCP控制块中各字段的操作，所以非常重要!!!
		
		******************************************************************************************************************************************************************************************************	
		tcp_input是TCP层的总输入函数，它会为数据包寻找一个匹配的TCP控制块，以及调用相应的函数tcp_timewait_input，tcp_listen_input，tcp_process进行处理
		void tcp_input(struct pbuf *p,struct netif *inp)
		{
			struct tcp_pcb 	*pcb,*prev;
			struct tcp_pcb_listen *lpcb;
			u8_t hdrlen;
			err_t err;
			
			// 略过IP包头，提取TCP头
			iphdr = (struct ip_hdr *)p->payload;
			tcphdr = (struct tcp_hdr *)((u8_t *)p->payload + IPH_HL(iphdr)*4)
			
			// 移动pbuf结构中的数据包指针，使指向TCP头
			if (pbuf_header(p, -((s16_t)(IPH_HL(iphdr) * 4))) || (p->tot_len < sizeof(struct tcp_hdr))) 
			{
				pbuf_free(p);
				return;	
			}
			
			// 不处理输入的广播包
			if (ip_addr_isbroadcast(&current_iphdr_dest, inp) || ip_addr_ismulticast(&current_iphdr_dest))
			{
				pbuf_free(p);
				return;		
			}
			
			// 验证TCP校验和
			if (inet_chksum_pseudo(p, ip_current_src_addr(), ip_current_dest_addr(),IP_PROTO_TCP, p->tot_len) != 0)
			{
				pbuf_free(p);
    			return;	
			}
			
			// 继续移动pbuf结构中的数据包指针，使指向TCP数据
			hdrlen = TCPH_HDRLEN(tcphdr);
			if(pbuf_header(p, -(hdrlen * 4))
			{
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
			tcplen = p->tot_len + ((flags & (TCP_FIN | TCP_SYN)) ? 1 : 0);	// TCP数据包中数据的总长度，对于有FIN或SYN标志的数据包，该长度要加1
			
			// 以下就是对接收到的数据包进行分类处理，也就是寻找合适的接口，根据IP，port
			// 首先在tcp_active_pcbs 链表池中找，有没有匹配的tcp_pcb
			prev = NULL;
			for(pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next)
			{
				if (pcb->remote_port == tcphdr->src && pcb->local_port == tcphdr->dest && ip_addr_cmp(&(pcb->remote_ip), &current_iphdr_src) && ip_addr_cmp(&(pcb->local_ip), &current_iphdr_dest)) 
				{
					// 找到匹配的接口之后，将该tcp_pcb从tcp_active_pcbs链表池中取出，然后退出循环往下运行，这时pcb != NULL
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
			
			// 如果在tcp_active_pcbs中没有找到，继续在tcp_tw_pcbs 和tcp_listen_pcbs中找
			if (pcb == NULL) 
			{
				// 在tcp_tw_pcbs中找
	    		for(pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next) 	
	    		{
	    			if (pcb->remote_port == tcphdr->src && pcb->local_port == tcphdr->dest && ip_addr_cmp(&(pcb->remote_ip), &current_iphdr_src) && ip_addr_cmp(&(pcb->local_ip), &current_iphdr_dest)) 
	    			{
	    				// 进入TIME_WAIT状态处理(解析见下文)，处理完直接这里返回不再往下运行
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
	      				// 然后判断IP是否匹配，或者是IPADDR_ANY接收任何IP
				        if (ip_addr_cmp(&(lpcb->local_ip), &current_iphdr_dest) || ip_addr_isany(&(lpcb->local_ip))) 
				        {
				        	// 找到匹配的接口之后退出循环往下运行，这时lpcb != NULL
				          	break;
				        }		
	      			}
	      			prev = (struct tcp_pcb *)lpcb;
	    		}
	    		
	    		// 这里是判断在tcp_listen_pcbs中是否找到
	    		if (lpcb != NULL) 
	    		{
	    			// 将该tcp_pcb从tcp_listen_pcbs.listen_pcbs链表池中取出
	    			if (prev != NULL)
	    			{
	    				((struct tcp_pcb_listen *)prev)->next = lpcb->next;
	    				lpcb->next = tcp_listen_pcbs.listen_pcbs;
	    				tcp_listen_pcbs.listen_pcbs = lpcb;	
	    			}	
	    			
	    			// 进入LISTEN状态处理（解析见下文），处理完直接这里返回不再往下运行
					tcp_listen_input(lpcb);
					pbuf_free(p);
					return;
	    		}
			}
			
			// 如果在tcp_active_pcbs中找到了，则经过处理后进入tcp_process
  			if (pcb != NULL) 
  			{
  				inseg.next = NULL;		// 关闭报文段队列功能
			    inseg.len = p->tot_len;	// 设置该报文段的数据长度
			    inseg.p = p;			// 设置报文段数据链表头指针
			    inseg.tcphdr = tcphdr;	// 报文段的TCP头
		
			    recv_data = NULL;		// 数据接收结果被保存在该全局变量，然后往上层提交
			    recv_flags = 0;			// tcp_process执行完后的结果（控制块的状态变迁）将会被保存在该全局变量，首先在这里被清0
				
				// tcp_pcb的refused_data指针上是否还记录有尚未往上层递交的数据
	    		if (pcb->refused_data != NULL) 
	    		{
	    			// 有的话回调用户recv函数接收未递交的数据
	    			TCP_EVENT_RECV(pcb, pcb->refused_data, ERR_OK, err);
	    			
	    			// 判断处理recv函数的处理结果，成功refused_data指针清空，继续往下执行tcp_process
					if (err == ERR_OK) 
					{
						pcb->refused_data = NULL;
					} 	
					// 失败意味着tcp_pcb都被占用满，丢弃接收包不再处理，直接返回
					else if ((err == ERR_ABRT) || (tcplen > 0)) 
					{
						pbuf_free(p);
						return;
					}
	    		}
	    		
	    		tcp_input_pcb = pcb;	// 记录处理当前报文的控制块
	    		
	    		// 这里就是进入tcp_process处理接收包环节了（解析见下文）,该函数实现了TCP状态转换功能 
			    err = tcp_process(pcb);
			    
			    // 若返回值为ERR_ABRT，说明控制块已经被完全删除(tcp_abort()),什么也不需要做
			    if (err != ERR_ABRT) 
			    {
			    	// 返回值不为ERR_ABRT时，判断报文处理的3种结果
					if (recv_flags & TF_RESET) 			// 接收到对方的复位报文
					{
						// 回调用户的errf函数
						TCP_EVENT_ERR(pcb->errf, pcb->callback_arg, ERR_RST);
						// 删除控制块
						tcp_pcb_remove(&tcp_active_pcbs, pcb);
						// 释放控制块空间
						memp_free(MEMP_TCP_PCB, pcb);
					}	
					else if (recv_flags & TF_CLOSED) 	// 双方连接成功断开
					{
						// 删除控制块
						tcp_pcb_remove(&tcp_active_pcbs, pcb);
						// 释放控制块空间
						memp_free(MEMP_TCP_PCB, pcb);
					} 
					else
					{
						err = ERR_OK;
						if (pcb->acked > 0) 			// 如果有被确认的已发送数据长度		
						{
							// 回调用户的send函数
						  	TCP_EVENT_SENT(pcb, pcb->acked, err);
						  	if (err == ERR_ABRT) 
						  	{
						    	goto aborted;
						  	}
						}
						
						if (recv_data != NULL)			// 如果有数据被接收到 
						{
							if (pcb->flags & TF_RXCLOSED) 	// 如果本地TCP控制块已经处于TF_RXCLOSED状态，则后续接收到的数据都作废
							{
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
							if (err == ERR_ABRT) 
							{
								goto aborted;
							}
							
							// 除此之外，如果返回值是失败，将这部分尚未往上递交的数据暂存到refused_data指针中
							if (err != ERR_OK) 
							{
								pcb->refused_data = recv_data;
							}
						}
						
						if (recv_flags & TF_GOT_FIN)	// 如果收到对方的FIN请求 
						{
							// 纠正接收窗口 
							if (pcb->rcv_wnd != TCP_WND) 
							{
								pcb->rcv_wnd++;
							}
							// 用一个NULL指针回调用户的recv函数，通过这种方式用户程序可以知道对方的关闭请求
							TCP_EVENT_CLOSED(pcb, err);
							if (err == ERR_ABRT) 
							{
								goto aborted;
							}
						}
						
						tcp_input_pcb = NULL;		// 当前报文到此处理完毕，清空当前报文的控制块
						tcp_output(pcb);			// 输出报文
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
  				// 如果在3张链表里都未找到匹配的pcb，则调用tcp_rst向源主机发送一个TCP复位数据包
  				if (!(TCPH_FLAGS(tcphdr) & TCP_RST))
  				{
  					tcp_rst(ackno, seqno + tcplen,ip_current_dest_addr(), ip_current_src_addr(),tcphdr->dest, tcphdr->src);
  				}
  				
  				pbuf_free(p);
  			}
		}
		
		******************************************************************************************************************************************************************************************************	
		// 本函数是处于LISTEN状态的控制块对输入报文的处理函数,处于LISTEN状态的控制块只能响应SYN握手包
		err_t tcp_listen_input(struct tcp_pcb_listen *pcb)
		{
			struct tcp_pcb *npcb;
			err_t rc;
			
			// 处于listen状态的pcb只能响应SYN握手包，对含有ACK标志的输入报文返回一个RST报文
			if (flags & TCP_ACK) 
			{
				tcp_rst(ackno + 1, seqno + tcplen,ip_current_dest_addr(), ip_current_src_addr(),tcphdr->dest, tcphdr->src);
			}
			// 处于listen状态的服务器端等到了SYN握手包
			else if (flags & TCP_SYN)
			{
				// 建立一个新的tcp_pcb，因为处于tcp_listen_pcbs链表上的pcb是tcp_pcb_listen结构的，而其他链表上的pcb是tcp_pcb结构
	    		npcb = tcp_alloc(pcb->prio);	
	    		
	    		// 如果新建失败，往往是因为内存不够
	    		if (npcb == NULL) 
	    		{
	    			TCP_STATS_INC(tcp.memerr);
	      			  ERR_MEM;
	    		}
	    		
	    		// 为这个新建的tcp_pcb填充成员
			    ip_addr_copy(npcb->local_ip, current_iphdr_dest);
			    npcb->local_port = pcb->local_port;
			    ip_addr_copy(npcb->remote_ip, current_iphdr_src);
			    npcb->remote_port = tcphdr->src;
			    npcb->state = SYN_RCVD;								// 进入SYN_RCVD状态
			    npcb->rcv_nxt = seqno + 1;							// 期望接收到的下一个序号，注意加1
			    npcb->rcv_ann_right_edge = npcb->rcv_nxt;			// 初始化右侧通告窗口
			    npcb->snd_wnd = tcphdr->wnd;						// 根据TCP头中对方可接收数据长度，初始化本地发送窗口大小
			    npcb->ssthresh = npcb->snd_wnd;						// 拥塞算法相关，暂略
			    npcb->snd_wl1 = seqno - 1;							// 初始化上次窗口更新时收到的序号
			    npcb->callback_arg = pcb->callback_arg;				// 初始化用户自定义数据
			    npcb->accept = pcb->accept;							// 初始化连接accept时的回调函数	
			    npcb->so_options = pcb->so_options & SOF_INHERITED;	// 继承socket选项
			    
			    TCP_REG(&tcp_active_pcbs, npcb);					// 将这个设置好的tcp_pcb注册到tcp_active_pcbs链表中去
			    tcp_parseopt(npcb);									// 从收到的SYN握手包中提取TCP头中选项字段的值，并设置到自己的tcp_pcb
			    npcb->mss = tcp_eff_send_mss(npcb->mss, &(npcb->remote_ip));	// 初始化mss
			    
			    // 回复带有SYN和ACK标志的握手数据包
			    rc = tcp_enqueue_flags(npcb, TCP_SYN | TCP_ACK);
			    if (rc != ERR_OK) 
			    {
			      	tcp_abandon(npcb, 0);
			      	return rc;
			    }
			    
			    // TCP层的总输出函数，详见下文
	   			return tcp_output(npcb);
			}
			return ERR_OK;
		}
		
		******************************************************************************************************************************************************************************************************
		// 本函数是处于TIMEWAIT状态的控制块处理输入报文的函数
		err_t tcp_timewait_input(struct tcp_pcb *pcb)
		{
			// 如果报文中含RST标志，直接丢弃
			if (flags & TCP_RST)  
			{
				return ERR_OK;	
			}	
			
			// 如果报文中含SYN标志
			if (flags & TCP_SYN) 
			{
				// 如果SYN的序号在接收窗口内，返回一个RST报文
				if (TCP_SEQ_BETWEEN(seqno, pcb->rcv_nxt, pcb->rcv_nxt+pcb->rcv_wnd)) 
				{
					tcp_rst(ackno, seqno + tcplen, ip_current_dest_addr(), ip_current_src_addr(),tcphdr->dest, tcphdr->src);
	      			return ERR_OK;	
				}
			}
			// 如果报文中含FIN标志
			else if(flags & TCP_FIN)
			{
				pcb->tmr = tcp_ticks;
			}
			
			// 如果TCP报文中有数据
			if(tcp_len > 0)
			{
				pcb->flags |= TF_ACK_NOW;	// 将当前控制块设为TF_ACK_NOW状态
				
				// TCP层的总输出函数，详见下文
    			return tcp_output(pcb);		
			}
			
			return ERR_OK;
		}
		
		******************************************************************************************************************************************************************************************************	
		// 除了处于LISTEN、TIME_WAIT状态的其余所有状态的pcb控制块，其报文的输入处理都在这里，该函数主要实现了TCP状态转换功能
		err_t tcp_process(struct tcp_pcb *pcb)
		{
			struct 	tcp_seg *rseg;
			u8_t	acceptable = 0;
			err_t	err;
			
			err	= ERR_OK;
			
			// 首先判断该报文是不是一个RST报文
			if(flags & TCP_RST)
			{
				// 判断该RST报文是否合法
				if (pcb->state == SYN_SENT) 	// 第一种情况，连接处于SYN_SENT状态
				{
					if (ackno == pcb->snd_nxt) 	// 且输入报文中的确认号就是控制块中想要发送的下一个序号
					{
						acceptable = 1;	
					}
				}
				else							// 第二种情况，其他状态下，输入报文中的序号在接收窗口内
				{
					if (TCP_SEQ_BETWEEN(seqno, pcb->rcv_nxt,pcb->rcv_nxt+pcb->rcv_wnd)) 
					{
						acceptable = 1;
					}	
				}
				
				// 如果RST报文合法，则需要复位当前连接的控制块，非法则直接返回不做处理
				if (acceptable)
				{
					recv_flags |= TF_RESET;			// 表明该输入报文的处理结果中包含TF_RESET
		  			pcb->flags &= ~TF_ACK_DELAY;	// 因为输入是RST报文，意味当前控制块必然不处于TF_ACK_DELAY状态
		  			return ERR_RST;	
				}
				else
				{
					return ERR_OK;
				}
			}
			
			// 然后处理握手报文SYN，在连接已经建立情况下，但还是接收到对方的握手包，说明这可能是一个超时重发的握手包，直接向对方返回一个ACK即可
			if ((flags & TCP_SYN) && (pcb->state != SYN_SENT && pcb->state != SYN_RCVD)) 
			{
				tcp_ack_now(pcb);					// #define tcp_ack_now(pcb) 	pcb->flags |= TF_ACK_NOW	- 将当前控制块设为TF_ACK_NOW状态
				return ERR_OK;	
			}
			
			// TCP连接不处于半关闭前提下，更新控制块的活动计数器
			if ((pcb->flags & TF_RXCLOSED) == 0) 
			{
				pcb->tmr = tcp_ticks;
			}
			
			// 保活报文计数器清0
  			pcb->keep_cnt_sent = 0;
  			
  			// 处理报文首部中的选项字段（暂略）
  			tcp_parseopt(pcb);
  			
  			// 根据当前所处的不同的TCP状态执行相应动作
  			switch (pcb->state) 
  			{
  				case SYN_SENT:	// 客户端发出SYN后，就处于该状态等待服务器返回SYN+ACK
  					// 如果收到的是SYN+ACK，且输入报文中的确认号，就是控制块中已发送，但尚未收到应答报文段中的序号+1
  					if ((flags & TCP_ACK) && (flags & TCP_SYN) && ackno == ntohl(pcb->unacked->tcphdr->seqno) + 1)
  					{
  						pcb->snd_buf++;							// 发出SYN被返回的ACK确认，释放1字节空间，所以可用的发送空间加1字节	
  						pcb->rcv_nxt = seqno + 1;				// 期望接收的下一个序号，即接收端向发送端ACK报文中的确认号
  						pcb->rcv_ann_right_edge = pcb->rcv_nxt;	// 初始化通告窗口的右边界值（略存疑问）
  						pcb->lastack = ackno;					// 更新接收到的最大确认号字段，也就是更新上一个确认号字段
  						pcb->snd_wnd = tcphdr->wnd;				// 发送窗口设置为接收窗口大小，实现流量控制
  						pcb->snd_wl1 = seqno - 1; 				// 上次窗口更新时收到的数据序号
  						pcb->state = ESTABLISHED;				// 进入ESTABLISHED状态
  						
  						pcb->mss = tcp_eff_send_mss(pcb->mss, &(pcb->remote_ip));	// 计算并设置最大报文段
  						pcb->ssthresh = pcb->mss * 10;								// 重设mss后，ssthresh值也要相应修改
  						pcb->cwnd = ((pcb->cwnd == 1) ? (pcb->mss * 2) : pcb->mss);	// 初始化阻塞窗口
  							
  						--pcb->snd_queuelen;			// SYN被返回的ACK确认，所以占用的pbuf个数减1	
  						
  						rseg = pcb->unacked;			// 从发送了未收到确认的数据段队列中取出SYN报文，相当于删除
  						pcb->unacked = rseg->next;		// 指向下一个发送了未收到确认的数据段
  						if(pcb->unacked == NULL)		// 如果未确认的数据段为空，则停止重传定时器
							pcb->rtime = -1;
						else 							// 如果队列中还有报文，则复位重传定时器和重传次数
						{
							pcb->rtime = 0;
							pcb->nrtx = 0;
						}
						
						tcp_seg_free(rseg);				// 释放取下的SYN报文段内存空间
						
						TCP_EVENT_CONNECTED(pcb, ERR_OK, err);	// 回调用户的connect函数（详解见下文）
						if (err == ERR_ABRT) 
						{
							return ERR_ABRT;
						}
						
						tcp_ack_now(pcb);				// 向服务器返回ACK，三次握手结束，具体含义见L753
  					}
  					// 如果只收到对方的ACK却没有SYN，则向对方返回RST报文
  					else if(flag & TCP_ACK)
  					{
  						tcp_rst(ackno, seqno + tcplen, ip_current_dest_addr(), ip_current_src_addr(),tcphdr->dest, tcphdr->src);
  					}
  					break;
  				case SYN_RCVD:	// 服务器发送SYN+ACK后，就处于该状态，等待客户端返回ACK
  					// 如果收到ACK，也就是三次握手的最后一个报文
  					if(flags & TCP_ACK)
  					{
  						// 如果ACK合法
  						if (TCP_SEQ_BETWEEN(ackno, pcb->lastack+1, pcb->snd_nxt)) 
  						{
  							if (TCP_SEQ_BETWEEN(ackno, pcb->lastack+1, pcb->snd_nxt)) 
  							{
  								u16_t old_cwnd;
  								
  								pcb->state = ESTABLISHED;	// 进入ESTABLISHED状态
  								
  								TCP_EVENT_ACCEPT(pcb, ERR_OK, err);		// 回调用户的accept函数
  								if (err != ERR_OK) 						// 如果accept函数返回错误，则关闭当前连接
  								{
  									if (err != ERR_ABRT) 
  									{
									    tcp_abort(pcb);
									}
									return ERR_ABRT;
  								}
  								
  								old_cwnd = pcb->cwnd;		// 保存旧的阻塞窗口
  								
  								tcp_receive(pcb);			// 如果该ACK报文中还携带了数据，则调用tcp_receive处理报文中的数据（解析见下文） 
  								
  								// 调整本地未被确认的字节数，因为SYN报文占用1个字节，所以减1
  								if (pcb->acked != 0) 		
								{
								  	pcb->acked--;				
								}
								
								pcb->cwnd = ((old_cwnd == 1) ? (pcb->mss * 2) : pcb->mss);	// 初始化阻塞窗口
								
								// 如果在上面的tcp_receive处理结果中包含FIN标志
						        if (recv_flags & TF_GOT_FIN) 
								{
						          	tcp_ack_now(pcb);			// 回复ACK，响应对方的FIN握手标志
						          	pcb->state = CLOSE_WAIT;	// 进入CLOSE_WAIT状态
						        }
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
		      			tcp_rexmit(pcb);
		    		}
		    		break;
		    	case CLOSE_WAIT:	// 服务器处于接收关闭的半连接状态，会一直等待上层应用执行关闭指令，发出FIN，并将状态变为LASK_ACK
		    	case ESTABLISHED:	// 连接双方都处于稳定状态
		    		tcp_receive(pcb);				// 调用函数处理报文中的数据
		    		
		    		// 如果在上面的tcp_receive处理结果中包含FIN标志
		    		if (recv_flags & TF_GOT_FIN) 
		    		{
		    			tcp_ack_now(pcb);			// 回复ACK，响应对方的FIN握手标志
		    			pcb->state = CLOSE_WAIT;	// 进入CLOSE_WAIT状态
		    		}
		    		break;
		    	case FIN_WAIT_1:	// 上层应用主动执行关闭指令，发送FIN后处于该状态(通常对于客户端来讲)
		    		tcp_receive(pcb);						// 调用函数处理报文中的数据
		    		
		    		// 如果在上面的tcp_receive处理结果中包含FIN标志
		    		if (recv_flags & TF_GOT_FIN) 
		    		{
		    			// 如果该报文同时包含一个合法ACK,意味着本地端将直接跳过FIN_WAIT_2进入TIME_WAIT状态
	      				if ((flags & TCP_ACK) && (ackno == pcb->snd_nxt)) 
	      				{
	      					tcp_ack_now(pcb);				// 回复ACK
					        tcp_pcb_purge(pcb);				// 清除该连接中的所有现存数据
					        TCP_RMV(&tcp_active_pcbs, pcb);	// 从tcp_active_pcbs链表中删除该控制块
					        pcb->state = TIME_WAIT;			// 跳过FIN_WAIT_2状态，直接进入TIME_WAIT状态
					        TCP_REG(&tcp_tw_pcbs, pcb);		// 将该控制块加入tcp_tw_pcbs链表	
	      				}
	      				// 如果该报文不含ACK，即表示双方同时执行了关闭连接操作
	      				else
	      				{
	      					tcp_ack_now(pcb);				// 返回ACK
	        				pcb->state = CLOSING;			// 进入CLOSING状态
	      				}
		    		}
		    		// 如果只收到有效的ACK
					else if ((flags & TCP_ACK) && (ackno == pcb->snd_nxt)) 
					{
		      			pcb->state = FIN_WAIT_2;			// 进入FIN_WAIT_2状态
				    }
				    break;	
				case FIN_WAIT_2:	// 主动关闭，发送FIN握手且收到ACK后处于该状态		
					tcp_receive(pcb);						// 调用函数处理报文中的数据
					
					// 如果在上面的tcp_receive处理结果中包含FIN标志
		    		if (recv_flags & TF_GOT_FIN) 
		    		{
		    			tcp_ack_now(pcb);					// 回复ACK
						tcp_pcb_purge(pcb);					// 清除该连接中的所有现存数据
						TCP_RMV(&tcp_active_pcbs, pcb);		// 从tcp_active_pcbs链表中删除该控制块
						pcb->state = TIME_WAIT;				// 进入TIME_WAIT状态
						TCP_REG(&tcp_tw_pcbs, pcb);			// 将该控制块加入tcp_tw_pcbs链表
		    		}
		    		break;
		    	case CLOSING:		// 双方同时执行主动关闭，处于该状态(特殊情况)
		    		tcp_receive(pcb);						// 调用函数处理报文中的数据
					
		    		// 如果收到合法ACK
					if (flags & TCP_ACK && ackno == pcb->snd_nxt) 
					{
						tcp_pcb_purge(pcb);					// 清除该连接中的所有现存数据
						TCP_RMV(&tcp_active_pcbs, pcb);		// 从tcp_active_pcbs链表中删除该控制块
						pcb->state = TIME_WAIT;				// 进入TIME_WAIT状态
						TCP_REG(&tcp_tw_pcbs, pcb);			// 将该控制块加入tcp_tw_pcbs链表
					}
		    		break;
		    	case LAST_ACK:		// 服务器在执行被动关闭时，发送完FIN，等待ACK时处于该状态
		    		tcp_receive(pcb);						// 调用函数处理报文中的数据
		    		
		    		// 如果收到合法ACK
					if (flags & TCP_ACK && ackno == pcb->snd_nxt) 
					{
						recv_flags |= TF_CLOSED;			// recv_flags设置为TF_CLOSED，由tcp_input函数对该控制块进行释放和清除
					}
					break;
				default:
					break;
  			}
  			return ERR_OK;
		}
		
		******************************************************************************************************************************************************************************************************	
		// 只会被tcp_process函数调用，用于进一步完成对输入报文的处理，具体来说，该函数主要是完成输入报文的冗余截断，管理unacked、unsent、ooseq三张链表
		void tcp_receive(struct tcp_pcb *pcb)
		{
			struct tcp_seg *next;
			struct tcp_seg *prev, *cseg;
			struct pbuf *p;
			s32_t off;
			s16_t m;
			u32_t right_wnd_edge;	// 本地发送窗口右边界
			u16_t new_tot_len;
			int found_dupack = 0;	// 重复ack标志，置1表示是重复ack	
			
			// 首先检测报文是否包含ACK标志
			if (flags & TCP_ACK) 
			{
				right_wnd_edge = pcb->snd_wl2 + pcb->snd_wnd;	// 获取本地发送窗口右边界
				
				// 有3种情况可以导致本地发送窗口更新
				if (TCP_SEQ_LT(pcb->snd_wl1, seqno)||								// snd_wl1小于新seqno，说明对方有发来数据
					(pcb->snd_wl1 == seqno && TCP_SEQ_LT(pcb->snd_wl2, ackno))||	// snd_wl1等于新seqno且snd_wl2小于新ackno，说明对方没有发送数据，只是在收到数据后发送一个确认
					(pcb->snd_wl2 == ackno && tcphdr->wnd > pcb->snd_wnd)) 			// snd_wl2等于新ackno且snd_wnd小于报文首部的窗口通告wnd，说明我方没有发数据过去，但被对方告知接收窗口变大	
				{
					pcb->snd_wnd = tcphdr->wnd;		// 更新本地发送窗口大小	，跟对方发来的接收窗口通告匹配
					pcb->snd_wl1 = seqno;			// 更新接收到的序号
					pcb->snd_wl2 = ackno;			// 更新接收到的确认号
					
					// 如果发送窗口非0，且探察开启
					if (pcb->snd_wnd > 0 && pcb->persist_backoff > 0) 
					{
						pcb->persist_backoff = 0;	// 停止窗口探察
					}
				}
				
				// 判断是否是一个重复的ACK，需要满足5个条件
				// 1.如果ackno小于等于lastack，即没有确认新数据
				if (TCP_SEQ_LEQ(ackno, pcb->lastack)) 						
				{
					pcb->acked = 0;		// 没有确认新数据，那么acked为0
					
					// 2.如果报文段中没有数据
					if (tcplen == 0) 
					{
						// 3.本地发送窗口没有更新
						if (pcb->snd_wl2 + pcb->snd_wnd == right_wnd_edge)
						{
							// 4.如果重传定时器正在运行，即本地有数据正等待被确认
							if (pcb->rtime >= 0) 
							{
								// 5.如果ackno等于lastack
								if (pcb->lastack == ackno)
								{
									// 此时可以确定这是一个重复的ack，说明报文发生了丢失
									found_dupack = 1;
									// 该ack被重复收到的次数自增
									if (pcb->dupacks + 1 > pcb->dupacks)
										++pcb->dupacks;
									// 如果该ack重复收到超过3次，说明发生了拥塞
									if (pcb->dupacks > 3) 
									{
										if ((u16_t)(pcb->cwnd + pcb->mss) > pcb->cwnd) 
										{
											pcb->cwnd += pcb->mss;
										}
									}
									// 如果该ack重复第3次收到，执行快速重传算法
									else if (pcb->dupacks == 3)
									{
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
					// 如果控制块处于快速重传状态	，则关闭重传状态、拥塞功能	
					if (pcb->flags & TF_INFR) 
					{
						pcb->flags &= ~TF_INFR;
						pcb->cwnd = pcb->ssthresh;
					}
					
					pcb->nrtx = 0;								// 重传次数清0
					pcb->rto = (pcb->sa >> 3) + pcb->sv;		// 复位重传超时时间
					pcb->acked = (u16_t)(ackno - pcb->lastack);	// 更新acked字段为被确认的已发送数据长度
					pcb->snd_buf += pcb->acked;					// 更新可用的发送空间
					pcb->dupacks = 0;							// 将ack重复收到的次数清0
					pcb->lastack = ackno;						// 更新接收到的ackno
					
					// 如果处于TCP连接已经建立状态，调整拥塞算法功能模块
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
					
					// 遍历unacked队列，将所有数据编号小于等于ackno的报文段移除
					while (pcb->unacked != NULL && TCP_SEQ_LEQ(ntohl(pcb->unacked->tcphdr->seqno) + TCP_TCPLEN(pcb->unacked), ackno)) 
					{  	
						// 将满足要求的报文从unacked链表取出
						next = pcb->unacked;
						pcb->unacked = pcb->unacked->next;
						
						// 如果该报文包含FIN标志，意味着当前收到的ACK对FIN做了确认，则acked字段减1，即不需要提交上层使知道FIN被对方成功接收
						if ((pcb->acked != 0) && ((TCPH_FLAGS(next->tcphdr) & TCP_FIN) != 0)) 
						{
						  	pcb->acked--;
						}
						
						pcb->snd_queuelen -= pbuf_clen(next->p);		// 释放被该报文占用的发送空间
						tcp_seg_free(next);								// 释放被该报文占用的tcp报文段
					}
					
					// 当所有满足要求的报文段移除成功后，判断unacked队列是否为空
					if(pcb->unacked == NULL)
						pcb->rtime = -1;	// 若为空，关闭重传定时器
					else
						pcb->rtime = 0;		// 否则复位重传定时器
		
					pcb->polltmr = 0;		// 复位轮询定时器
				}		
				// 如果该ACK既不是重复ACK，又不是正常ACK，则acked字段清0，即该ACK不确认任何已发送数据
				else 
				{
					pcb->acked = 0;
    			}
    			
    			// 遍历unsent队列，将所有数据编号小于等于ackno的报文段移除
			    // 这是因为对于需要重传的报文段，lwip直接将它们挂在unsent队列上，所以收到的ACK可能是对已超时报文段的确认
			    while (pcb->unsent != NULL && TCP_SEQ_BETWEEN(ackno, ntohl(pcb->unsent->tcphdr->seqno) + TCP_TCPLEN(pcb->unsent), pcb->snd_nxt)) 
			    {
					// 将满足要求的报文从unsent链表取出
					next = pcb->unsent;
					pcb->unsent = pcb->unsent->next;
					
					// 如果该报文包含FIN标志，意味着当前收到的ACK对FIN做了确认，则acked字段减1，即不需要提交上层使知道FIN被对方成功接收
					if ((pcb->acked != 0) && ((TCPH_FLAGS(next->tcphdr) & TCP_FIN) != 0)) 
					{
						pcb->acked--;
					}
					
					pcb->snd_queuelen -= pbuf_clen(next->p);		// 释放被该报文占用的发送空间
					tcp_seg_free(next);								// 释放被该报文占用的tcp报文段
				}
				
				// RTT计算，暂略
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
			
			// 如果该输入报文还包含了数据，则要继续对数据进行处理
			if (tcplen > 0) 
			{
				// 如果seqno + 1 <= rcv_nxt <= seqno + tcplen - 1，意味着收到的数据区域头部有无效数据（收到的数据有部分处于本地左侧接收窗口外），需要截断数据头
				if (TCP_SEQ_BETWEEN(pcb->rcv_nxt, seqno + 1, seqno + tcplen - 1))
				{
					off = pcb->rcv_nxt - seqno;							// 需要截掉的数据长度
					p = inseg.p;										// 获取收到的报文段的pbuf链表头
					
					// 判断需要截断的长度是否超出了第一个pbuf中存储的数据长度
					if (inseg.p->len < off) 
					{
						new_tot_len = (u16_t)(inseg.p->tot_len - off);	// 截断重复数据后的有效数据长度

						// 如果超出，则需要遍历pbuf链表，依次摘除数据，直到最后一个包含摘除数据的pbuf
						while (p->len < off) 
						{
							off -= p->len;								// 剩余摘除长度
							p->tot_len = new_tot_len;					// 更新当前pbuf中的数据总长，
							p->len = 0;									// 因为数据被摘除，所以当前pbuf中的数据分长清0
							p = p->next;								// 指向下一个pbuf
						}
						
						// 处理最后一个包含摘除数据的pbuf，就是调整数据指针略过摘除数据
						pbuf_header(p, (s16_t)-off);
					}
					else 
					{
						// 如果未超出，则调整第一个pbuf中的数据指针略过摘除数据
						pbuf_header(inseg.p, (s16_t)-off);
					}
					
					inseg.len -= (u16_t)(pcb->rcv_nxt - seqno);	// 更新TCP报文段数据总长
					inseg.tcphdr->seqno = seqno = pcb->rcv_nxt;	// 更新TCP头中的seqno，指向接收窗口头位置
				}
				else 
				{
					// 如果seqno < rcv_nxt，意味着seqno+tcplen-1 < rcv_nxt，说明这是个完全重复的报文段
					if (TCP_SEQ_LT(seqno, pcb->rcv_nxt))
					{
						tcp_ack_now(pcb);		// 只回复一个ACK给对方(这里是否应该直接返回不再运行下去)
					}
				}
				
				// 如果数据起始编号在接收窗口内
			    if (TCP_SEQ_BETWEEN(seqno, pcb->rcv_nxt, pcb->rcv_nxt + pcb->rcv_wnd - 1))
		        {
		        	// 如果该报文数据处于接收起始位置，意味着该报文是连续到来的
					if (pcb->rcv_nxt == seqno) 
					{
						tcplen = TCP_TCPLEN(&inseg);		// 更新该报文的总数据长度

						// 如果总长大于接收窗口大小，就需要做尾部截断处理，这里包含对FIN和SYN两种标志的不同处理结果，注意体会
						if (tcplen > pcb->rcv_wnd) 
						{
							// 如果TCP头中带FIN标志，清除FIN标志，因为对方还有数据要发过来
							if (TCPH_FLAGS(inseg.tcphdr) & TCP_FIN) 
							{
								TCPH_FLAGS_SET(inseg.tcphdr, TCPH_FLAGS(inseg.tcphdr) &~ TCP_FIN);
							}
							
							inseg.len = pcb->rcv_wnd;		// 根据接收窗口调整数据长度

							// 如果TCP头中带SYN标志，报文段数据长度减1
							if (TCPH_FLAGS(inseg.tcphdr) & TCP_SYN) 
							{
								inseg.len -= 1;
							}
							
							pbuf_realloc(inseg.p, inseg.len);	//  因为数据被截断，pbuf中的参数需要相应调整
							tcplen = TCP_TCPLEN(&inseg);		// 再次更新该报文的总数据长度
						}
						
						// 如果无序报文段队列ooseq上存在报文段
				        if (pcb->ooseq != NULL) 
						{
							// 判断当前有序报文段的TCP头中是否带FIN标志
							if (TCPH_FLAGS(inseg.tcphdr) & TCP_FIN) 
							{
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
								// 遍历ooseq链表，删除序号被当前有序报文段完全覆盖的报文段
								while (next && TCP_SEQ_GEQ(seqno + tcplen,next->tcphdr->seqno + next->len)) 
								{
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
								
								// 如果当前有序报文段尾部与ooseq中的报文段存在部分重叠	
								if (next && TCP_SEQ_GT(seqno + tcplen,next->tcphdr->seqno)) 
								{
									inseg.len = (u16_t)(next->tcphdr->seqno - seqno);	// 截断当前有序报文段尾部的重叠部分，得到有效部分长度
									
									// 如果当前有序报文段TCP头中带SYN标志，报文段数据长度减1
									if (TCPH_FLAGS(inseg.tcphdr) & TCP_SYN) 
									{
										inseg.len -= 1;
									}
									
									pbuf_realloc(inseg.p, inseg.len);			//  因为数据被截断，pbuf中的参数需要相应调整
									tcplen = TCP_TCPLEN(&inseg);				// 再次更新该报文的总数据长度
								}
								pcb->ooseq = next;
							}
						}
						
						pcb->rcv_nxt = seqno + tcplen;	// 更新下一个期望接收到的序号，也就是接收窗口左边界
						pcb->rcv_wnd -= tcplen;			// 更新当前可用接收窗口

		        		tcp_update_rcv_ann_wnd(pcb);	// 更新公告窗口
		        		
		        		// 如果该有序报文段中存在数据
				        if (inseg.p->tot_len > 0) 
						{
							recv_data = inseg.p;		// 将全局指针recv_data指向报文段中的数据pbuf
							inseg.p = NULL;
						}
						
						// 如果该有序报文段的TCP头中带FIN标志
				        if (TCPH_FLAGS(inseg.tcphdr) & TCP_FIN) 
						{
				          recv_flags |= TF_GOT_FIN;		// 则在报文处理结果变量recv_flags添加TF_GOT_FIN标志
				        }
				        
				        // 遍历ooseq队列，取出所有有序的报文段
				        // (通过比较ooseq队列中报文段的seqno和当前TCP控制块中保存的rcv_nxt来判定该报文段是否有序)
				        while (pcb->ooseq != NULL && pcb->ooseq->tcphdr->seqno == pcb->rcv_nxt) 
						{
							cseg = pcb->ooseq;
							seqno = pcb->ooseq->tcphdr->seqno;	// 更新序号
							pcb->rcv_nxt += TCP_TCPLEN(cseg);	// 更新下一个期望接收到的序号
							pcb->rcv_wnd -= TCP_TCPLEN(cseg);	// 更新当前可用接收窗口
							tcp_update_rcv_ann_wnd(pcb);		// 更新公告窗口
							
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
								recv_flags |= TF_GOT_FIN;		// 则全局变量recv_flags添加TF_GOT_FIN标志

								// 如果当前TCP处于ESTABLISHED状态，则变成CLOSE_WAIT状态
								if (pcb->state == ESTABLISHED) 
								{ 
									pcb->state = CLOSE_WAIT;
								}
							}
							
							pcb->ooseq = cseg->next;
							tcp_seg_free(cseg);
						}
						
						// 以上都执行完毕后，向源端返回一个ACK，此处其实只是先在TCP控制块中添加ACK标志
						tcp_ack(pcb);
					}
					// 如果该报文数据不处于接收起始位置，意味着该报文不是有序的
					else 
					{
						// 首先向源端返回一个立即ACK
		        		tcp_send_empty_ack(pcb);
		        		
		        		// 然后将该报文段放入ooseq队列
				        if (pcb->ooseq == NULL) 
						{
							// 如果ooseq为空，则拷贝该报文段到新开辟的报文段空间，并将新开辟报文段作为ooseq起始单元
				          	pcb->ooseq = tcp_seg_copy(&inseg);
				        } 
						else 
						{
							prev = NULL;	// 定义为ooseq链表中上一个报文段，这里首先清空
							// 遍历ooseq队列，选择合适位置插入该报文段
							for(next = pcb->ooseq; next != NULL; next = next->next) 
							{
								// 依次比较两个报文段的起始序号seqno，如果相等
								if (seqno == next->tcphdr->seqno) 
								{
									// 继续比较两个报文段的数据长度
									if (inseg.len > next->len) 
									{
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
											
											tcp_oos_insert_segment(cseg, next);	// 处理好插入后与原有的下一个报文段的影响，简单来说，就是切掉冗余，释放内存
										}
										break;	// 退出循环							
									}
									else
									{
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
											cseg = tcp_seg_copy(&inseg);			// 拷贝要插入的报文段到新开辟的报文段空间
											if (cseg != NULL) 
											{
												pcb->ooseq = cseg;					// 将新报文段插到ooseq第一个位置
												tcp_oos_insert_segment(cseg, next);	// 处理好插入后与原有的第一个报文段的影响
											}
											break;		// 退出循环
										}
									}
									// 如果不是第一个
									else 
									{
										// 如果待插入报文段起始序号在前一个和后一个报文段起始序号之间
										if (TCP_SEQ_BETWEEN(seqno, prev->tcphdr->seqno+1, next->tcphdr->seqno-1)) 
										{
											cseg = tcp_seg_copy(&inseg);	// 拷贝要插入的报文段到新开辟的报文段空间
											if (cseg != NULL) 
											{
												// 如果与前一个报文段有数据重合
												if (TCP_SEQ_GT(prev->tcphdr->seqno + prev->len, seqno)) 
												{
													prev->len = (u16_t)(seqno - prev->tcphdr->seqno);	// 截断前一个报文段尾部
													pbuf_realloc(prev->p, prev->len);					// 因为数据被截断，pbuf中的参数需要相应调整
												}
												
												prev->next = cseg;					// 将新报文段插入前一个报文段之后
												tcp_oos_insert_segment(cseg, next);	// 处理好插入后与原有的下一个报文段的影响
											}
											break;
										}
									}
									
									// 如果已经是ooseq上的最后一个报文段
									// 且待插入的报文段起始序号大于该报文起始序号(其实函数运行到这里该条件必然成立)
									if (next->next == NULL && TCP_SEQ_GT(seqno, next->tcphdr->seqno)) 
									{
										// 如果该报文的TCP头中有FIN标志，则直接丢弃待插入的报文段，退出循环
										if (TCPH_FLAGS(next->tcphdr) & TCP_FIN) 
										{
											break;
										}
										
										next->next = tcp_seg_copy(&inseg);	// 拷贝要插入的报文段到新开辟的报文段空间，并插在队列尾部

										// 如果新插入的报文段不为空
										if (next->next != NULL) 
										{
											// 如果与前一个报文段有数据重合
											if (TCP_SEQ_GT(next->tcphdr->seqno + next->len, seqno)) 
											{
												next->len = (u16_t)(seqno - next->tcphdr->seqno);	// 截断前一个报文段尾部
												pbuf_realloc(next->p, next->len);					// 因为数据被截断，pbuf中的参数需要相应调整
											}
											
											// 如果新插入的报文段数据长度超出了当前接收窗口大小
											if ((u32_t)tcplen + seqno > pcb->rcv_nxt + (u32_t)pcb->rcv_wnd) 
											{
												// 如果新插入的报文段的TCP头中有FIN标志
												if (TCPH_FLAGS(next->next->tcphdr) & TCP_FIN) 
												{
													TCPH_FLAGS_SET(next->next->tcphdr, TCPH_FLAGS(next->next->tcphdr) &~ TCP_FIN);	// 去掉TCP头中的FIN标志
												}
												
												next->next->len = pcb->rcv_nxt + pcb->rcv_wnd - seqno;	// 根据接收窗口大小调制新插入的报文段数据长度
												pbuf_realloc(next->next->p, next->next->len);			// 因为数据被截断，pbuf中的参数需要相应调整
												tcplen = TCP_TCPLEN(next->next);						// 再次更新该报文的总数据长度
											}
											
										}
										break;
									}
								}
								
								prev = next;	// 以上都不满足，则遍历ooseq链表中下一个
							}
						}
					}
		        }
		        // 如果数据不在接收范围内
				else 
				{
					tcp_send_empty_ack(pcb);	// 直接向源端返回一个立即确认ACK
				}
			}
			// 如果输入的报文段中不包含数据
			else 
			{
				// 且序号位于接收窗口之内
			    if(!TCP_SEQ_BETWEEN(seqno, pcb->rcv_nxt, pcb->rcv_nxt + pcb->rcv_wnd-1))
			    {
			      	tcp_ack_now(pcb);		// 回一个ACK
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
		
		
		
		