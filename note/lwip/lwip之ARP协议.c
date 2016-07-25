											ARP缓存表
/*****************************************************************************************************************
ARP协议的核心是ARP缓存表，ARP的实质就是对缓存表的建立、更新、查询等操作
lwip通过数组的方式来创建缓存表	struct etharp_entry arp_table[ARP_TABLE_SIZE]
数组arp_table的基本元素为缓存表项，定义如下：
	struct etharp_entry{
	  	struct pbuf *q;			// 单个IP数据包缓冲指针
		ip_addr_t ipaddr;		// 存储一对IP地址和MAC地址，这是ARP缓存表项的核心部分
		struct eth_addr ethaddr;
#if LWIP_SNMP
  		struct netif *netif;
#endif 
		u8_t state;				// 描述该缓存表项的状态，一共3种：ETHARP_STATE_EMPTY,ETHARP_STATE_PENDING,ETHARP_STATE_STABLE
		u8_t ctime;				// 该缓存表项的计数器，判断ARP超时用
	}
	
#define ARP_MAXAGE 		240	// 处于稳定状态下的ARP缓存表项最大生存时间计数值
#define ARP_MAXPENDING 	2	// 处于pending状态下的ARP缓存表项最大生存时间计数值
lwip通过定时函数etharp_tmr来删除那些超时的ARP缓存表项，该函数以5s为周期被调用
void etharp_tmr(void)	
{
	u8_t i;
	
	// 遍历整个ARP缓存表，删除超时的ARP缓存表项
    for (i = 0; i < ARP_TABLE_SIZE; ++i) 
    {
    	u8_t state = arp_table[i].state;
    	
    	if (state != ETHARP_STATE_EMPTY)
    	{
    		// 先将每个缓存表项的计数值加1
    		arp_table[i].ctime++;
    		
    		// 判断是否超时
            if ((arp_table[i].ctime >= ARP_MAXAGE) ||
          	((state == ETHARP_STATE_PENDING)  && (arp_table[i].ctime >= ARP_MAXPENDING))) 
          	{
          		// 判断要删除的缓存表项上是否挂有数据
          		if (arp_table[i].q != NULL) 
          		{
          			// 释放挂在缓存表项里的数据空间
          			free_etharp_q(arp_table[i].q);
    				arp_table[i].q = NULL;
          		}
          		
          		// 将该缓存表项标记为未用状态
          		arp_table[i].state = ETHARP_STATE_EMPTY;
          	}
    	}
    }
}
*****************************************************************************************************************/

											ARP报文
/*****************************************************************************************************************
lwip使用一个eth_hdr的结构体来描述以太网数据帧首部的14个字节：
															struct eth_hdr{
																struct eth_addr dest;	// 目的MAC地址
																struct eth_addr src;	// 源MAC地址
																u16_t type;				// 类型，主要用到2种：0x0800 - IP	0x0806 - ARP
															}
以太网数据包最大帧长1518字节，最小帧长64字节，当无法满足最小64字节后，末尾用trailer字段来填充补足64字节

ARP和IP是两个独立的协议，都属于网络层，都依赖以太网数据帧来传输自身的协议数据															
lwip使用一个etharp_hdr的结构体来描述ARP协议包：
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
*****************************************************************************************************************/

											ARP层数据包输入
/*****************************************************************************************************************
ARP层数据包的输入由2部分组成：
1. 如果输入的是IP包，对于同网段的IP/MAC地址对，需要更新ARP缓存表
void etharp_ip_input(struct netif *netif, struct pbuf *p)
{
	struct eth_hdr *ethhdr;
  	struct ip_hdr *iphdr;
  	ip_addr_t iphdr_src;
  	
  	ethhdr = (struct eth_hdr *)p->payload;						// 获取以太网协议首部
  	iphdr = (struct ip_hdr *)((u8_t*)ethhdr + SIZEOF_ETH_HDR);	// 获取IP协议首部
  	
  	// 如果对方IP与本地IP在不同网段，则直接返回
  	ip_addr_copy(iphdr_src, iphdr->src);
	if (!ip_addr_netcmp(&iphdr_src, &(netif->ip_addr), &(netif->netmask)))
		return;
	
	// 只有与本地IP在同网段的IP才会更新ARP缓存表
    update_arp_entry(netif, &iphdr_src, &(ethhdr->src), ETHARP_FLAG_FIND_ONLY);
}

2. 如果输入的是ARP包，则处理ARP数据包，更新ARP缓存表，对ARP请求进行应答
static void etharp_arp_input(struct netif *netif, struct eth_addr *ethaddr, struct pbuf *p)
{
	struct etharp_hdr *hdr;
	struct eth_hdr *ethhdr;
	ip_addr_t sipaddr, dipaddr;
  	u8_t for_us;
	
}
*****************************************************************************************************************/
																						
