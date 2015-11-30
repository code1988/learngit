cache(高速缓冲存储器)：cache是位于CPU和DRAM之间，通常由SRAM构成的规模小存取速度快的存储器
write buffer(写缓冲区)：由于高速CPU和低速外围设备间的执行效率不同步问题，产生了写缓冲区，用于优化向主存储器中的写入操作
cache有3种分类标准：
	1.	data cache 
		instruction cache
	2. 	write through cache:	CPU在执行写操作时，必须同时把数据写入cache和主存储器中
		write back cache:		CPU在执行写操作时，被写的数据只写入cache，不写入主存储器中，仅当需要替换时，才把已经修改的cache块写回到主存储器中
	3. 	read allocate cache:	进行数据写操作时，不使用cache；只有进行读操作时，才使用cache预取
		write allocate cache:	进行数据写操作时，如果cache没命中，将进行cache预取，即从主存中将相应的块读到cache相应位置，并执行写操作，把数据写到cache中，至于主存中如何刷新，则根据写通/写回模式而定

ARM处理器中的cache和write buffer相关操作都是通过协处理器CP15完成，具体如下：
	op1	cn	cm	op2				register				attribute			content（cache相关）
	0	c0	c0	0/4/6/7			主标识符寄存器				RO;privilege
	0	c0	c0	1				cache类型标识符寄存器		   RO;privilege		
	2	c0	c0	0				cache级数选择寄存器			R/W;privilege	L1/L2选择，指令/数据cache选择	
	1	c0	c0	0				cache级数鉴定寄存器			RO;privilege	鉴定选择的cache尺寸
	cache使能位位于CP15的C1寄存器中，操作CP15的C1寄存器指令格式如下：
		MCR P15,0,<rd>,c1,c0,0					
		MRC P15,0,<rd>,c1,c0,0
				
	控制是通过CP15的C7寄存器来实现的，访问CP15的C7寄存器指令格式如下：
		MCR P15,0,<rd>,<c7>,cm,<opcode_2>		//CP15的C7寄存器是一个只写寄存器，所以只有MCR
	
	相关的指令列表如下：
				指令					<rd>中预先填入的数据			含义
		mcr p15,0,rd,c1,c0,0			#0x00001000				写1使能指令cache；清零禁止指令cache
		mcr p15,0,rd,c1,c0,0			#0x00000004				写1使能数据cache；清零禁止数据cache
				
		mcr p15,0,rd,c7,c0,4				#0					等待中断激活
		
		mcr p15,0,rd,c7,c5,0				#0					使整个指令cache无效
		mcr p15,0,rd,c7,c5,1			虚拟地址					使指令cache中某块无效
		mcr p15,0,rd,c7,c5,2			组号/组内序号			使指令cache中某块无效
		
		mcr p15,0,rd,c7,c5,4				#0					清空预取缓冲区
		mcr p15,0,rd,c7,c5,6				#0					清空整个跳转目标cache
		mcr p15,0,rd,c7,c5,7			厂商定义					清空跳转目标cache中某块
		
		mcr p15,0,rd,c7,c6,0				#0					使整个数据cache无效		
		mcr p15,0,rd,c7,c6,1			虚拟地址					使数据cache中某块无效
		mcr p15,0,rd,c7,c6,2			组号/组内序号			使数据cache中某块无效
		
		mcr p15,0,rd,c7,c7,0				#0					使整个指令和数据cache都无效
		mcr p15,0,rd,c7,c7,1			虚拟地址					使整个指令和数据cache中某块无效
		mcr p15,0,rd,c7,c7,2			组号/组内序号			使整个指令和数据cache中某块无效
		
		mcr p15,0,rd,c7,c8,2				#0					等待中断激活
		
		mcr p15,0,rd,c7,c10,1			虚拟地址					清空数据cache中某块
		mcr p15,0,rd,c7,c10,2			组号/组内序号			清空数据cache中某块
		mcr p15,0,rd,c7,c10,4				#0					清空写缓冲区
		mcr p15,0,rd,c7,c11,1			虚拟地址					清空整个指令和数据cache中某块
		mcr p15,0,rd,c7,c11,2			组号/组内序号			清空整个指令和数据cache中某块
		mcr p15,0,rd,c7,c13,1			虚拟地址					预取指令cache中某块
		mcr p15,0,rd,c7,c14,1			虚拟地址					清空并使无效数据cache中某块
		mcr p15,0,rd,c7,c14,2			组号/组内序号			清空并使无效数据cache中某块
		mcr p15,0,rd,c7,c15,1			虚拟地址					清空并使无效整个指令和数据cache中某块
		mcr p15,0,rd,c7,c15,2			组号/组内序号			清空并使无效整个指令和数据cache中某块
