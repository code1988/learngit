/********************************************************************************************************
	目前常用的ARM汇编有2种：ARMASM和GNU ARM ASM,二者略有不同，以下以GNU ARM ASM为主						*
********************************************************************************************************/

1. 汇编系统预定义的段名
		.text	-	代码段：用来存放程序执行代码的一块内存区域，通常是只读的
		.rodata	-	只读数据段：用来存放字符串和#define定义的常量，也是只读的
		.data	-	数据段：用来存放程序中已初始化的全局变量和静态变量，属于静态内存分配区域，可读可写
		.bss	- 	BSS段：用来存放程序中未初始化的全局变量和静态变量，也属于静态内存分配区域，可读可写
		
		备注：	之所以将data和bss区分开，因为系统会为bss段的变量初值清0
	
2. 汇编程序的缺省入口时_start标号，也可以在链接脚本（.lds for arm）中用ENTRY标志指明程序入口位置

3. ARM GNU 汇编伪指令汇总：
		.global symbol	-	定义一个全局符号，通常是为lds链接脚本使用（同ARMASM中的EXPORT）
							例		.global _start
		.word 	value	-	在当前位置放一个32bit的值，这个值就是value（同ARMASM中的DCD）
							例		FIQ_STACK_START:
									.word 	0x0badc0de	// 就是在当前地址，即FIQ_STACK_START处，放入一个值0x0badc0de
		.equ 	symbol,	value	-	类似C中的#define，为一个符号赋值（同ARMASM中的EQU）
							例		.equ	COUNT	0x80000000	// 等同于#define COUNT 0x80000000
						
4. 任何汇编行的基本格式：
		[<标签>：][<指令>]
		备注：GNU ARM ASM中，任何以冒号结尾的都被认为是一个标签，标签可以理解为C中的函数名，实质是一个地址
		
5. ldr指令：不论ARMASM还是GNU ARM ASM中，ldr都有2种用法
	ldr作为加载指令时的格式		ldr{条件} 目的寄存器，<存储器地址>		// 用法：从存储器地址中读取32位的数据到通用寄存器
								ldr			r0,		[r1]				// 将存储器地址为r1的32位数据读入寄存器r0
								ldr			r0,		[r1,r2]				// 将存储器地址为r1+r2的32位数据读入寄存器r0
								ldr			r0,		[r1,#8]				// 将存储器地址为r1+8的32位数据读入寄存器r0
								ldr			r0,		[r1,r2]!			// 将存储器地址为r1+r2的32位数据读入寄存器r0，并将新地址r1+r2写入r1
								ldr			r0,		[r1,r2,lsl#2]!		// 将存储器地址为r1+r2*4上的32位数据读入寄存器r0，并将新地址r1+r2*4写入r1
                                ldr         r0,     [r1],   #8          // 将存储器地址为r1的32位数据读入寄存器r0，并将新地址r1+8写入r1
	ldr作为伪指令时的格式		ldr			Rn,		=expr				// 将常量expr读入寄存器Rn
	
	备注：	汇编中的大括号{}通常表示里面的选项是可选的
			汇编中的中括号[]通常表示取里面地址中的数据，类似于C语言指针的作用
			汇编中的叹号!通常表示操作完成后，新地址要被写入前一个寄存器
