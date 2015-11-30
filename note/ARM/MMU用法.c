
ARM CPU地址转换涉及三种地址：虚拟地址VA，变换后的虚拟地址MVA，物理地址PA.
启动MMU后
	CPU核心看到和用到的只是虚拟地址VA
	Cashes和MMU看不到VA,只能看到和使用MVA,Z转换得到PA
	实际设备看不到VA、MVA，使用的是PA

VA和MVA转换关系：
	如果VA<32M ，需要使用进程标识号PID（通过读CP15的C13寄存器获得）来转换为MVA
	if（VA<32M）then
		MVA = VA | (PID<<25)
	else
		MVA = VA
		
		
/**************************************************************************
*
*	以一级页表、段描述符为例，虚拟地址 - 物理地址 转换主要涉及5张图：
*	1. 页表基址寄存器	[31   :   14]  [13 : 0]					//一级页表基址TTB是16K对齐，所以低14位是0
*						 一级页表基址	   0		
*		
*	2. 虚拟地址MVA		[31   :   20]  [19     :     0]			//虚拟地址高12位[31:20]用来索引一级页表，索引范围为2^12 = 4096，即4096个一级页表地址，每个地址上存储一个描述符，每个描述符4字节
*						 一级页表索引	 同物理地址低位 
*
*	3. 一级页表地址		[31	   :   14] [13    :    2][1 : 0]	//一级页表地址按4字节对齐，每个地址上存储一个段描述符
*						 一级页表基址    一级页表索引		0
*
*	4. 段描述符			[31	  :   20] [19  	  : 	 0]			//段基址就是物理地址高位，内存访问权限决定一块内存是否允许读写
*						    段基址		 内存访问权限
*
*	5. 物理地址PA		[31	  :   20] [19     :     0]			//真正的物理地址，按段分，每段1MB,[19:0]正是用于寻址
*						    段基址      同虚拟地址低位
*
**************************************************************************/
void create_page_table(void)
{

/* 
 * 用于段描述符的一些宏定义
 */ 
#define MMU_FULL_ACCESS     (3 << 10)   /* 访问权限 */
#define MMU_DOMAIN          (0 << 5)    /* 属于哪个域 */
#define MMU_SPECIAL         (1 << 4)    /* 必须是1 */
#define MMU_CACHEABLE       (1 << 3)    /* cacheable */
#define MMU_BUFFERABLE      (1 << 2)    /* bufferable */
#define MMU_SECTION         (2)         /* 表示这是段描述符 */
#define MMU_SECDESC         (MMU_FULL_ACCESS | MMU_DOMAIN | MMU_SPECIAL | MMU_SECTION)
#define MMU_SECDESC_WB      (MMU_FULL_ACCESS | MMU_DOMAIN | MMU_SPECIAL | MMU_CACHEABLE | MMU_BUFFERABLE | MMU_SECTION)
#define MMU_SECTION_SIZE    0x00100000

    unsigned long virtuladdr, physicaladdr;
    unsigned long *mmu_tlb_base = (unsigned long *)0x30000000;					//这是定义一级页表地址，将它的值写入协处理器CP15的寄存器C2(页表基址寄存器)即可
    
    /*
     * Steppingstone的起始物理地址为0，第一部分程序的起始运行地址也是0，
     * 为了在开启MMU后仍能运行第一部分的程序，
     * 将0～1M的虚拟地址映射到同样的物理地址
     */
    virtuladdr = 0;
    physicaladdr = 0;															//等号左边是存储段描述符的地址，MMU利用该地址寻找到段描述符，这其实就是个索引的过程（索引范围：0～4096描述符）
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \		//索引方法：[31:14]页表基址（一级页表地址TTB base = 页表基址+[13:0]填充0）+虚拟地址MVA高12位（用于索引页表：2^12=4096）			
                                            MMU_SECDESC_WB;						//等号右边即是段描述符（由2部分组成：[31:20]段基址(也就是1M物理空间的起始地址)，[11:0]访问控制，中间[19:12]8位忽略）
																				//段描述符属于页表描述符，当低2位为0b010时，表明该页表描述符就是段描述符
																				//？？？20140722 等号左边似乎应该是移动18位才对，使得索引一级页表时4字节对齐，这样才能得到4字节的描述符。
    /*
     * 0x56000000是GPIO寄存器的起始物理地址，
     * GPBCON和GPBDAT这两个寄存器的物理地址0x56000050、0x56000054，
     * 为了在第二部分程序中能以地址0xA0000050、0xA0000054来操作GPFCON、GPFDAT，
     * 把从0xA0000000开始的1M虚拟地址空间映射到从0x56000000开始的1M物理地址空间
     */
    virtuladdr = 0xA0000000;
    physicaladdr = 0x56000000;
    *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                            MMU_SECDESC;

    /*
     * SDRAM的物理地址范围是0x30000000～0x33FFFFFF，
     * 将虚拟地址0xB0000000～0xB3FFFFFF映射到物理地址0x30000000～0x33FFFFFF上，
     * 总共64M，涉及64个段描述符
     */
    virtuladdr = 0xB0000000;
    physicaladdr = 0x30000000;
    while (virtuladdr < 0xB4000000)
    {
        *(mmu_tlb_base + (virtuladdr >> 20)) = (physicaladdr & 0xFFF00000) | \
                                                MMU_SECDESC_WB;
        virtuladdr += 0x100000;
        physicaladdr += 0x100000;
    }
}

/********************************************************************************
*
*	TLB:转译查找缓存
*		这是一个用于存储最近用过的描述符的高速、小容量存储器，用来帮助快速进行地址转换，避免每次地址转换都去主存中查找
*		使用TLB时要保证TLB中的内容与页表一致，所以当页表内容变化时尤其要注意
*		启动MMU前必须先使TLB无效
*
*	对MMU、TLB、Cache等的操作涉及到协处理器，指令写法如下：
*	<MCR|MRC>{条件}  协处理器编码  协处理器操作码1  目的寄存器  源寄存器1  源寄存器2  协处理器操作码2
*	<MCR|MRC>{cond}		p#,		  <expression1>,     Rd,		cn,		   cm	  {,<expression2>}		//{}表示可选
*	MCR/MRC			// 数据从核心寄存器传给协处理器	/	数据从协处理器传给核心寄存器
*	{cond}			// 执行条件，省略时表示无条件执行
*	p#				// 协处理器序号
* 	<expression1/2>	// 一个常数
*	Rd				// 核心寄存器
*	cn&cm			// 协处理器中的寄存器
*
*
********************************************************************************/


/*
 * 启动MMU
 */
void mmu_init(void)
{
    unsigned long ttb = 0x30000000;

__asm__(
    "mov    r0, #0\n"
    "mcr    p15, 0, r0, c7, c7, 0\n"    /* 使无效ICaches和DCaches */
    
    "mcr    p15, 0, r0, c7, c10, 4\n"   /* drain write buffer on v4 */
    "mcr    p15, 0, r0, c8, c7, 0\n"    /* 使无效指令、数据TLB */
    
    "mov    r4, %0\n"                   /* r4 = 页表基址 */
    "mcr    p15, 0, r4, c2, c0, 0\n"    /* 设置页表基址寄存器 */
    
    "mvn    r0, #0\n"                   
    "mcr    p15, 0, r0, c3, c0, 0\n"    /* 域访问控制寄存器设为0xFFFFFFFF，
                                         * 不进行权限检查 
                                         */    
    /* 
     * 对于控制寄存器，先读出其值，在这基础上修改感兴趣的位，
     * 然后再写入
     */
    "mrc    p15, 0, r0, c1, c0, 0\n"    /* 读出控制寄存器的值 */
    
    /* 控制寄存器的低16位含义为：.RVI ..RS B... .CAM
     * R : 表示换出Cache中的条目时使用的算法，
     *     0 = Random replacement；1 = Round robin replacement
     * V : 表示异常向量表所在的位置，
     *     0 = Low addresses = 0x00000000；1 = High addresses = 0xFFFF0000
     * I : 0 = 关闭ICaches；1 = 开启ICaches
     * R、S : 用来与页表中的描述符一起确定内存的访问权限
     * B : 0 = CPU为小字节序；1 = CPU为大字节序
     * C : 0 = 关闭DCaches；1 = 开启DCaches
     * A : 0 = 数据访问时不进行地址对齐检查；1 = 数据访问时进行地址对齐检查
     * M : 0 = 关闭MMU；1 = 开启MMU
     */
    
    /*  
     * 先清除不需要的位，往下若需要则重新设置它们    
     */
                                        /* .RVI ..RS B... .CAM */ 
    "bic    r0, r0, #0x3000\n"          /* ..11 .... .... .... 清除V、I位 */
    "bic    r0, r0, #0x0300\n"          /* .... ..11 .... .... 清除R、S位 */
    "bic    r0, r0, #0x0087\n"          /* .... .... 1... .111 清除B/C/A/M */

    /*
     * 设置需要的位
     */
    "orr    r0, r0, #0x0002\n"          /* .... .... .... ..1. 开启对齐检查 */
    "orr    r0, r0, #0x0004\n"          /* .... .... .... .1.. 开启DCaches */
    "orr    r0, r0, #0x1000\n"          /* ...1 .... .... .... 开启ICaches */
    "orr    r0, r0, #0x0001\n"          /* .... .... .... ...1 使能MMU */
    
    "mcr    p15, 0, r0, c1, c0, 0\n"    /* 将修改的值写入控制寄存器 */
    : /* 无输出 */
    : "r" (ttb) );
}