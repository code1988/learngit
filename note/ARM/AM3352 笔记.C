TIMER:
	1. 定时器2被用来做OS时钟节拍
	2. 定时器7被用于USB，其他任务尽量不用
	3. 2、3、4、7类似
UART:
	1. 波特率范围：300bps—— 3.6864Mbps
	2. UART1~5 不支持wake-up功能，只有UART0支持；UART1支持全调制功能
	3. UART0 时钟来源: pd_wkup_L4_gclk; pd_wkup_uart0_gfclk
	   UART1~5 时钟来源：pd_per_L4LS_;pd_per_uart_gfclk
	4. UART模块支持7个中断源，每个中断的开关位于UART_IER寄存器中，中断产生时，UART_IIR寄存器中的相应标志位都会被置位
	   7个中断存在固定的6级优先级：	第一级：接收线状态中断		//OE FE PE BI错误发生在RX FIFO
	   								第二级：RX超时				//RX FIFO中接收不到新数据
	   									    RX接收中断			//数据满（FIFO未使能）或者RX FIFO超过触发深度
	   								第三级：TX发送中断			//数据空（FIFO未使能）或者TX FIFO低于触发线
	   								第四级：模式状态中断
	   								第五级：XOFF中断
	   								第六级：CTS/RTS/DSR中断	
	实际使用情况：
		1. UART0：用于马达控制
				TX--E15;RX--E16						 	//UART0模块只有一组PIN可选
				管脚复用寄存器：	conf_uart0_rxd 		//CONTROL MODULE基地址+0x970
								 	conf_uart0_txd	 	//CONTROL MODLUE基地址+0x974
							   	 	都是默认0号功能
		2. UART2: 用于LCD控制
				TX--L18;RX--K18						 	//UART2模块有多组PIN可选
				管脚复用寄存器：	conf_mii1_tx_clk	//CONTROL MODULE基地址+0x92c
								 	conf_mii1_rx_clk	//CONTROL MODULE基地址+0x930
								 	都是1号功能
		

外设模块配置套路：
	1. 0号xxxx模块配置：						//xxxx：UART/IIC
		xxxx0ModuleClkConfig:
			1.使能L3_PER时钟					//外设时钟模块CM_PER中5个寄存器，各个外设配置都相同
												//确认5个寄存器中相应状态位，各个外设相同
			2.使能L4_WKUP时钟					//唤醒时钟模块CM_WKUP中4个寄存器，各个外设配置区别在于最后一个
												//确认7个寄存器中的状态位，各个外设区别在于最后两个 
												
	2. 1～n号xxxx模块配置：						//xxxx：UART/IIC
		xxxxnModuleClkConfig：
			1.使能L3/L4_PER时钟					//外设时钟模块CM_PER中8个寄存器，相比0号模块多出后3个寄存器操作；各个外设配置区别在于最后一个
												//确认4个寄存器中的状态位，各个外设区别在于最后一个			
												

SPI:
	1. 每个SPI模块可以接入最多4个从设备或1个主设备;最多拥有4个通道（实际只有2路CS0、CS1）
	   只有通道0可以用于从模式
	2. 如果1个SPI模块内有超过1个通道开启了FIFO功能，那么该功能反而不会在任何一个通道上生效，也就是说，FIFO是每个SPI模块共用的
	3. SPI的配置参数只有在该通道关闭的情况下才能被配置成功
	3. 当SPI处于从模式时，如果在连续传送之间SPIEN线没有解除认定，那么发送寄存器中的内容将不被发送，而是最后接收到的数据被发送
	4. 主模式时：
		中断：4个 —— 	TX_empty		使能SPI某个通道后就会自动触发该中断（主接收模式除外）
										如果FIFO被使能，当FIFO中有足够的空间写入n字节（n可由MCSPI_XFERLEVEL[AEL]配置）时，该中断被触发
						TX_underflow	该中断对于主模式来说只是一个无害的提醒
						RX_full			如果FIFO被使能，当FIFO中接收到的字节数达到n时（n可由MCSPI_XFERLEVEL[AFL]配置），该中断被触发
						EOW				前提是通道和FIFO都已经使能，当发送完指定的n字节后（n可由MCSPI_XFERLEVEL[WCNT]配置），该中断被触发；n=0时该中断不会触发
	   从模式时：
	    中断：5个 ——	TX_empty		同主模式
	    				TX_underflow	当外部主机进行SPI收发时，从机发送FIFO中无数据，则触发该中断
	    								从模式下，该中断意味着传输数据存在丢失现象
	    				RX_full			同主模式
	    				RX0_overflow	因为只有0通道可配置成从模式，所以只有RX0_overflow一个中断
	    								当外部主机传来一个新的SPI数据时，从机接收FIFO已满，则触发该中断
	    								FIFO中已有的数据会被覆盖，也就意味着遭到破坏，所以不能让该事件出现在从模式下
	    				EOW				同主模式
	    				
	    				
GPMC:
	1. 支持的设备：
				16位NOR
				8位或16位NAND
				16位pSRAM
	2. NAND:
		接口传输协议：					片选-CEn		命令锁存器-CLE		地址锁存器-ALE		写使能-WEn		读使能-REn		写保护-WPn
			待机状态（standby）			H			 	X					X					X				X				H/L
			写保护（write protect）		X				X					X					X				X				L
			写命令（cmd input）			L				H					L				  上升沿			H				H
			写地址（adr input）			L				L					H				  上升沿			H				H
			写数据（data input）		L				L					L				  上升沿			H				H
			读数据（data output）		L				L					L					H			 下降沿				X
	
			
			
BOOT:
	1.存储区域分布：
		片上ROM，分为2块：
			(1)	0x20000		execption vectors														0x40020000	(2)
							(整张异常向量表，除Reset外，全部重映射至RAM中，具体起始地址为0x4030CE04)
				0x20020		CRC																		0x40020020
							(32BIT值，计算范围0x20000-0x2BFFF)
				0x20080		Dead loops																0x40020080
							(这片固定区域定义了缺省的异常服务函数，即死循环；当然另有几个特殊的)
				0x20100		CODE																	0x40020100
							(存储芯片出厂时就已经固化的代码和常量，主要用于完成上电初始化：配置系统时钟、配置3分钟看门狗、跳转到boot程序)
				0x2BFFC		VERSION																	0x4002BFFC
							(内部存储器相关版本号)
				0x2BFFF																				0x4002BFFF
			以上2个ROM区域存储内容完全相同
		
		 片上RAM:
		 	0x402F0400		
		 					可以用来加载boot image，加载源可以是SD卡、nand、nor，根据硬件SYSBOOT[4:0]位决定，image最大可至109KB；
		 					也可以当普通的IRAM使用
		 	0x4030B800		
		 					栈空间，标称6k，实际如果image小，可以向低地址扩
		 	0x4030CE00		
		 					1. 	重映射的异常向量表，每条异常向量缺省指向偏移 #0x18地址。
		 						所以想要重定向到自定义的异常服务函数，2种方法：
		 															(1)	在偏移地址上写入自定义函数入口
		 															(2)	直接将自定义函数入口覆盖到 0x4030CE04 - 0x4030CE1C 上的对应位置
		 						注意点，由于Reset异常并未重映射过来，所以0x4030CE00地址其实是无用的
		 					2.	Tracing Data
		 					3.	静态变量
		 	0x4030FFFF

EDMA:
	1.EDMA3控制器包含2个主要模块：
		1)通道控制器（EDMA3CC）----- 内包含一块参数设置内存（PaRAM）、通道控制寄存器族、中断控制寄存器族，EDMA3CC主要用于区分传入的事件请求的优先顺序，然后再将传输请求提交给EDMA3TC	
			64个DMA通道
				- 触发方式：	外设事件触发；
							手动触发（写SER）；
							链触发（前一个发送链完成）
				- 64个外设事件传入
			8个QDMA通道
			256个PaRAM设置块
				- 可用于设置DMA、QDMA、链
			4条事件队列
				- 每条事件队列可以压入16个事件
			支持3D发送
				- A模式
				- B模式
			
														
		2)发送控制器（EDMA3TC）-----	EDMA3TC是EDMA3CC的附属设备，主要用于数据搬运
		
ETHERNET:
	支持3种网络接口：GMII、RGMII、RMII(前二种千兆，最后一种百兆) ;另有MDIO接口用于配置PHY
	时钟特性：MDIO接口的时钟必须确保小于2.5MHz（针对LAN8720A）；
			  RMII接口的50Mhz时钟不能由A8提供，因为通过ADPLLS CORE PLL锁相环得到的时钟抖动过大，无法满足绝大部分PHY的使用要求，所以该时钟必须由PHY提供，
			  这一点在管脚配置时需要特别注意，就是RMIIn_REFCLK必须配置成输入模式，而且GMII_SELL寄存器内需要特别注意配置时钟源
			  	 	