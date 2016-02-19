									网络接口结构
/*****************************************************************************************************************
网络接口管理属于链路层范畴，lwip使用一个名为netif的网络接口结构来描述各种网络设备，定义如下
	struct netif{
		struct netif *next;			// 指向下一个netif结构体指针，也就是下一个网络设备，如双网卡这里就需要指向另一块网卡的netif结构体
		struct ip_addr ip_addr;		// IP
		struct ip_addr netmask;		// 子网掩码
		struct ip_addr gw;			// 网关
		err_t (*input)(struct pbuf *p,struct netif *inp);							// 链路层调用函数指针input完成向IP层输入数据包
		err_t (*output)(struct netif *netif,struct pbuf *p,struct ip_addr *ipaddr);	// IP层调用函数指针output完成向链路层输出数据包
		err_t (*linkoutput)(struct netif *netif,struct pbuf *p);					// ARP模块调用linkoutput函数可以向网卡发送一个数据包,output函数最终也是调用linkoutput发数据包的
		void *status;						// 用户自定义指针，现用于指向cpswportif结构体，保存了AM3352的CPSW模块号（只有一个0号模块）、网口号（有1号、2号两个网口）、MAC地址
		u8_t hwaddr_len;					// MAC地址长度，固定6字节
		u8_t hwaddr[NETIF_MAX_HWADDR_LEN];	// MAC地址
		u16_t mtu;							// 网络传输最大单元，一般是固定值1500
		u8_t flags;							// 网卡状态控制字段，包括NETIF_FLAG_BROADCAST - 网卡广播使能，NETIF_FLAG_ETHARP - ARP功能使能，NETIF_FLAG_LINK_UP - 硬件链路连接使能，NETIF_FLAG_UP - 网卡功能使能
		char name[2];						// 网络设备驱动类型,暂时未用到
		u8_t num;							// netif的编号，双网卡的话就是1、2
	}

*****************************************************************************************************************/
