									网络接口结构
/*****************************************************************************************************************
网络接口管理属于链路层范畴，lwip使用一个名为netif的网络接口结构来描述各种网络设备，定义如下
	struct netif{
		struct netif *next;			// 指向下一个netif结构体指针，也就是下一个网络设备，如双网卡这里就需要指向另一块网卡的netif结构体
		struct ip_addr ip_addr;		// IP
		struct ip_addr netmask;		// 子网掩码
		struct ip_addr gw;			// 网关
		err_t (*input)(struct pbuf *p,struct netif *inp);							// 链路层调用函数指针input完成向IP层发送数据包
		err_t (*output)(struct netif *netif,struct pbuf *p,struct ip_addr *ipaddr);	// IP层调用函数指针output完成向链路层发送数据包
		err_t (*linkoutput)(struct netif *netif,struct pbuf *p);					// 链路层最终调用函数指针linkoutput完成向网卡发送数据包
		void *status;						// 用户自定义指针，现用于指向cpswportif结构体，保存了AM3352的CPSW模块号（只有一个0号模块）、网口号（有1号、2号两个网口）、MAC地址
		u8_t hwaddr_len;					// MAC地址长度，固定6字节
		u8_t hwaddr[NETIF_MAX_HWADDR_LEN];	// MAC地址
		u16_t mtu;							// 网络传输最大单元，一般是固定值1500
		u8_t flags;							// 网络接口的状态、属性字段，包括NETIF_FLAG_BROADCAST - 网卡广播使能，NETIF_FLAG_ETHARP - ARP功能使能，NETIF_FLAG_LINK_UP - 硬件链路连接使能，NETIF_FLAG_UP - 网络接口使能
		char name[2];						// 网络设备驱动类型,暂时未用到
		u8_t num;							// 每个网络接口对应一个编号，从0递增
	}
*****************************************************************************************************************/

									注册一个网络接口设备
/*****************************************************************************************************************
参数*state 	- (网卡设备)传入cpswportif地址								(环回接口)传入NULL									
参数*init 	- (网卡设备)传入cpswif_init，底层接口驱动初始化函数			(环回接口)传入netif_loopif_init
参数*input	- 传入tcpip_input，底层网卡向IP层提交数据包的函数
struct netif *netif_add(struct netif *netif,ip_addr *ipaddr,ip_addr *netmask,ip_addr *gw,void *state,err_t (* init)(struct netif *netif),err_t (*input)(struct pbuf *p,struct netif *inp))
{
	static u8_t netifnum = 0;

    // 复位netif结构的各字段
    ip_addr_set_zero(&netif->ip_addr);
    ip_addr_set_zero(&netif->netmask);
    ip_addr_set_zero(&netif->gw);
    netif->flags = 0;
    netif->loop_first = NULL;
    netif->loop_last = NULL;
    
    // 填充netif结构的各字段
    netif->state = state;
    netif->num = netifnum++;
    netif->input = input;
    netif_set_addr(netif, ipaddr, netmask, gw);
    
    // 调用网络接口初始化函数
    if (init(netif) != ERR_OK)
        return NULL;
    
    // 将初始化成功的netif结构插入链表头
    netif->next = netif_list;
    // 更新表头
    netif_list = netif;   
    
    // 返回netif指针
    return netif;  
}

网卡设备的网络接口初始化函数如下：（实际使用的用户自定义初始化函数封装与标准lwip略有不同）
err_t cpswif_init(struct netif *netif)
{
	struct cpswportif *cpswif = (struct cpswportif*)(netif->state);
	static u32_t inst_init_flag = 0;
	u32_t inst_num = cpswif->inst_num;
	
#ifdef CPSW_DUAL_MAC_MODE
	netif->num = (u8_t)(((cpswif->inst_num * MAX_SLAVEPORT_PER_INST) + cpswif->port_num - 1) & 0xFF);
#else
	netif->num = (u8_t)(cpswif->inst_num);
#endif
	netif->output = etharp_output;		//注册IP层数据包输出函数
	netif->linkoutput = cpswif_output;	//注册链路层数据包输出函数
	
	// 以下就是网卡底层初始化部分
	if(((inst_init_flag >> inst_num) & 0x01) == 0)
    {
    	// cpsw模块地址初始化
    	cpswif_inst_config(cpswif);
    	// cpsw模块初始化
    	cpswif_inst_init(cpswif);
    	inst_init_flag |= (1 << inst_num);
	}
	// cpsw与phy相关初始化
	if(cpswif_port_init(netif) != ERR_OK)
	    return ERR_CONN;   
	
	return ERR_OK;
}

环回接口的网络接口初始化函数如下：(最重要的操作就是注册IP层数据包输出函数netif_loop_output)
err_t netif_loopif_init(struct netif *netif)
{
  netif->name[0] = 'l';
  netif->name[1] = 'o';
  netif->output = netif_loop_output;
  return ERR_OK;
}

完成初始化后的net_if结构体会被链入链表，协议栈使用2个全局变量管理这张链表：netif_list指向链表表头，netif_default指向缺省的net_if结构体
*****************************************************************************************************************/
