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
					
*****************************************************************************************************************/
											