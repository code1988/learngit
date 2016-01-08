										makefile基本格式：
/***************************************************************************************************
	目标文件：依赖文件
		命令				//tab键不可少
***************************************************************************************************/

										"="、":="、"?="三种赋值符号的用法
/***************************************************************************************************
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
***************************************************************************************************/

										makefile常用通配符
/***************************************************************************************************
makefile里有3个常用变量
	$@	目标文件
	$^	所有依赖文件
	$<	第一个依赖文件
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
***************************************************************************************************/
	
										makefile函数
/***************************************************************************************************
makefile里的函数使用，和使用变量类似，也是“$()”格式，括号里面是函数名和形参，形参之间一般用“,”隔开

1. wildcard
	用法：	SRC = $(wildcard *.c ./sub/*.c)
	释义：	搜索当前目录和./sub目录下所有以.c结尾的文件，生成一个以空格间隔的文件名列表，并赋值给SRC。当前目录文件只有文件名，子目录下文件名包含相对路径
	
2. notdir
	用法：	DIR = $(notdir $(SRC))
	释义： 	去除所有的路径信息
	
3. patsubst
	用法：	OBJ = $(patsubst %c,%o,$(DIR))
	释义：	将DIR中所有结尾是c字符的变量替换成o字符，这里还有另外一种替换方法，其格式是“$(var:a=b)/${var:a=b}”，含义同patsubst
***************************************************************************************************/	

										传说中的万能makefile
/***************************************************************************************************
#　如果需要，調整下面的東西。　EXECUTABLE　是目標的可執行文件名，　LIBS 
#　是一個需要連接的程序包列表（例如　alleg,　stdcx,　iostr　等等）。當然你 
#　可以在　make　的命令行覆蓋它們，你願意就沒問題。 
#　 

EXECUTABLE　:=　mushroom.exe 
LIBS　:=　alleg 

#　Now　alter　any　implicit　rules'　variables　if　you　like,　e.g.: 
# 
#　現在來改變任何你想改動的隱含規則中的變量，例如 

CFLAGS　:=　-g　-Wall　-O3　-m486 
CXXFLAGS　:=　$(CFLAGS) 

#　下面先檢查你的　djgpp　命令目錄下有沒有　rm　命令，如果沒有，我們使用 
#　del　命令來代替，但有可能給我們　'File　not　found'　這個錯誤信息，這沒 
#　什大礙。如果你不是用　DOS　，把它設定成一個刪文件而不廢話的命令。 
#　（其實這一步在　UNIX　類的系統上是多余的，只是方便　DOS　用戶。　UNIX 
#　用戶可以刪除這５行命令。） 

ifneq　($(wildcard　$(DJDIR)/bin/rm.exe),) 
RM-F　:=　rm　-f 
else 
RM-F　:=　del 
endif 

#　You　shouldn't　need　to　change　anything　below　this　point. 
# 
#　從這裡開始，你應該不需要改動任何東西。（我是不太相信，太ＮＢ了！） 

SOURCE　:=　$(wildcard　*.c)　$(wildcard　*.cc) 
OBJS　:=　$(patsubst　%.c,%.o,$(patsubst　%.cc,%.o,$(SOURCE))) 
DEPS　:=　$(patsubst　%.o,%.d,$(OBJS)) 
MISSING_DEPS　:=　$(filter-out　$(wildcard　$(DEPS)),$(DEPS)) 
MISSING_DEPS_SOURCES　:=　$(wildcard　$(patsubst　%.d,%.c,$(MISSING_DEPS))　\ 
$(patsubst　%.d,%.cc,$(MISSING_DEPS))) 
CPPFLAGS　+=　-MD 

.PHONY　:　everything　deps　objs　clean　veryclean　rebuild 

everything　:　$(EXECUTABLE) 

deps　:　$(DEPS) 

objs　:　$(OBJS) 

clean　: 
　　@$(RM-F)　*.o 
　　@$(RM-F)　*.d 

veryclean:　clean 
　　@$(RM-F)　$(EXECUTABLE) 

rebuild:　veryclean　everything 

ifneq　($(MISSING_DEPS),) 
$(MISSING_DEPS)　: 
　　@$(RM-F)　$(patsubst　%.d,%.o,$@) 
endif 

-include　$(DEPS) 

$(EXECUTABLE)　:　$(OBJS) 
　　gcc　-o　$(EXECUTABLE)　$(OBJS)　$(addprefix　-l,$(LIBS)) 
###
***************************************************************************************************/	
												