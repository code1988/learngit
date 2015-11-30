//--------------------------------------------------------------------------------------
makefile基本格式：
	目标文件：依赖文件
		命令				//tab键不可少
//-------------------------------------------------------------------------------------		
"="用法：会将整个Makefile展开后，再决定变量的值，即变量的值将会是整个makefie中最后被指定的值。
例：
	x = foo
	y = $(x)bar
	x = xyz
	此例中，y的值将会是xyzbar,而不是foobar
	
":="用法：表示变量的值决定于它在makefile中的位置，而不是整个makefile展开后的最终值。
例：
	x := foo
	y := $(x)bar
	x := xyz
	此例中，y的值将会是foobar，而不是xyzbar

"?="用法，表示如果没有被赋值过就赋予等号后面的值，即只有当变量还没有被定义过的时候，才会将右边的值赋给变量
例：
	x := foo
	x ?=xyz
	此时 x的值为foo
	x :=foo
	y ?= $(x)bar
	此时 y的值为foobar
	x :=
	x ?= foo
	此时x的值为空
	
//--------------------------------------------------------------------------------------
makefile里有3个常用变量
	$@	目标文件
	$^	所有依赖文件
	$<	第一个依赖文件
/*----------------------------------------------------------------------------------------
makefile模板（mmu/leds）：
	objs := head.o init.o leds.o						#变量objs赋值
	
	mmu.bin : $(objs)									#目标文件：二进制烧写文件，依赖文件：$(objs)
		arm-linux-ld -Tmmu.lds -o mmu_elf $^			#arm-linux-ld负责将多个*.o的目标文件链接成elf可执行文件。-T*.lds/-T*格式用于添加链接脚本/直接指定代码段、数据段、BSS段起始地址，链接脚本示例：L57
														#arm-linux-ld -Ttext=0x30000000 led.o -o led_elf  其中text代码段在led.s启动文件里定义
		arm-linux-objcopy -O binary -S mmu_elf $@		#arm-linux-objcopy负责将elf转换为bin文件，-O指明输出格式：binary
		arm-linux-objdump -D -m arm mmu_elf > mmu.dis	#arm-linux-objdump负责将elf反汇编，-D显示汇编信息，-m arm指明arm架构，>反汇编文件重定向到指定文件
		
	%.o:%.c
		arm-linux-gcc -Wall -O2 -c -o $@ $<				#-c表示只编译不链接，生产对应文件名的.o目标文件，-O2指明编译器使能2级编译优化选项
	
	%.o:%.S
		arm-linux-gcc -Wall -O2 -c -o $@ $<
	
	clean:
		rm -f mmu.bin mmu_elf mmu.dis *.o		
	

mmu.lds内容：
	SECTION{										//以下地址在这个工程里都是虚拟地址
		first  0x00000000 : {head.o init.o}			//将head.o和init.o两个目标文件指定起始地址为0x00000000
		second 0xB0004000 : AT(2048){leds.o}				//将leds.o目标文件指定起始地址为0B0004000
	}		