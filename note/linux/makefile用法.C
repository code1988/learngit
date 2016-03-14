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

										makefile中的变量
/***************************************************************************************************
makefile中的变量大小写敏感，分为用户自定义变量、预定义变量、自动变量、环境变量

1. 用户自定义变量通常使用小写字母作为变量名

2. 常见的预定义变量
	CC			-	C编译器的名称，默认值cc
	CPP			-	C预编译器的名称，默认值$(CC) -E
	CXX			-	C++编译器的名称，默认值g++
	RM			- 	文件删除程序的名称，默认值rm -f
	CFLAGS		-	C编译器的选项，无默认值
	CPPFLAGS	- 	C预编译的选项，无默认值
	CXXFLAGS	-	C++编译器的选项，无默认值
	
3. 常见的自动变量
	$@	-	目标文件的完整名称
	$<	-	第一个依赖文件的名称
	$^	-	所有不重复的依赖文件，以空格分开

4. makefile在启动时会自动读取当前系统中的环境变量		

5. makefile标准的变量引用是用$()，但是makefile的执行离不开shell环境，因此使用${}也可以访问
***************************************************************************************************/

										makefile中的额外规则
/***************************************************************************************************
1. 隐式规则
	所有“.o”文件都可自动由“.c”文件使用命令“$(CC) $(CPPFLAGS) $(CFLAGS) -c file.c -o file.o”生成
	这条隐式规则只能查找相同文件名的不同后缀名文件，如a.o文件必须由a.c文件生成，这是其局限性

2. 模式规则
	%.o:%.c
		$(CC) $(CFLAGS) -c $< -o $@ 
	这条模式规则表示每个“.o”文件都会由第一个依赖的“.c”文件生成

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

                                        makefile调用shell脚本    
/***************************************************************************************************
1. shell 脚本只有在target里才有效

2. makefile把每一行shell脚本当作一个独立的单元，它们在单独的进程中运行，所以此时要特别注意换行的使用。
   比如调用shell中的流程控制语句if时，整条语句从if到fi要写在同一行里，所以换行书写时必须加连接符 \
***************************************************************************************************/
										生成可执行文件的makefile
/***************************************************************************************************
#source file
# 源文件，自动找所有 .c 和 .cpp 文件，并将目标定义为同名 .o 文件
SOURCE  := $(wildcard *.c) $(wildcard *.cpp)
OBJS    := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))
 
#target you can change test to what you want
# 目标文件名，输入任意你想要的执行文件名
TARGET  := test
 
#compile and lib parameter
# 编译参数
CC      := gcc
LIBS    :=
LDFLAGS:= 
DEFINES:=
INCLUDE:= -I.
CFLAGS  := -g -Wall -O3 $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H
 
 
#i think you should do anything here
# 下面的基本上不需要做任何改动了
.PHONY : everything objs clean veryclean rebuild
 
everything : $(TARGET)
 
all : $(TARGET)
 
objs : $(OBJS)
 
rebuild: veryclean everything
               
clean :
    rm -fr *.so
    rm -fr *.o
   
veryclean : clean
    rm -fr $(TARGET)
 
$(TARGET) : $(OBJS) 
    $(CC) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)
***************************************************************************************************/	

										生成静态链接库的makefile
/***************************************************************************************************
#target you can change test to what you want
# 共享库文件名， lib*.a
TARGET  := libtest.a
 
#compile and lib parameter
# 编译参数
CC      := gcc
AR      = ar
RANLIB  = ranlib
LIBS    :=
LDFLAGS:= 
DEFINES:=
INCLUDE:= -I.
CFLAGS  := -g -Wall -O3 $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H
 
#i think you should do anything here
# 下面的基本上不需要做任何改动了
 
#source file
# 源文件，自动找所有 .c 和 .cpp 文件，并将目标定义为同名 .o 文件
SOURCE  := $(wildcard *.c) $(wildcard *.cpp)
OBJS    := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))
 
.PHONY : everything objs clean veryclean rebuild
 
everything : $(TARGET)
 
all : $(TARGET)
 
objs : $(OBJS)
 
rebuild: veryclean everything
               
clean :
    rm -fr *.o
   
veryclean : clean
    rm -fr $(TARGET)
 
$(TARGET) : $(OBJS) 
    $(AR) cru $(TARGET) $(OBJS)
    $(RANLIB) $(TARGET)
***************************************************************************************************/		

										生成动态链接库的makefile
/***************************************************************************************************
#target you can change test to what you want
# 共享库文件名， lib*.so
TARGET  := libtest.so
 
#compile and lib parameter
# 编译参数
CC      := gcc
LIBS    :=
LDFLAGS:= 
DEFINES:=
INCLUDE:= -I.
CFLAGS  := -g -Wall -O3 $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H
SHARE   := -fPIC -shared -o
 
#i think you should do anything here
# 下面的基本上不需要做任何改动了
 
#source file
# 源文件，自动找所有 .c 和 .cpp 文件，并将目标定义为同名 .o 文件
SOURCE  := $(wildcard *.c) $(wildcard *.cpp)
OBJS    := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))
 
.PHONY : everything objs clean veryclean rebuild
 
everything : $(TARGET)
 
all : $(TARGET)
 
objs : $(OBJS)
 
rebuild: veryclean everything
               
clean :
    rm -fr *.o
   
veryclean : clean
    rm -fr $(TARGET)
 
$(TARGET) : $(OBJS) 
    $(CC) $(CXXFLAGS) $(SHARE) $@ $(OBJS) $(LDFLAGS) $(LIBS)
***************************************************************************************************/												
