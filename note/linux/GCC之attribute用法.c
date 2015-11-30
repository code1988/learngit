//=======================================================================
__attribute__可以设置函数属性、变量属性、类型属性。
__attribute__书写格式：
	__attribute__((参数))		//参数可以是多个

//-----------------------------------------------------------------------
用法1：
	__attribute__((constructor))	//若将某个函数设置为constructor属性，则该函数会在main()函数执行之前被自动执行
	__attribute__((destructor))		//若将某个函数设置为destructor属性，则该函数会在main()函数执行之后被自动执行
	例：
		#include "stdio.h"
		
		void before() __attribute__((constructor));		//在main函数之前执行before函数
		void after() __attribute__((destructor));		//在mian函数之后执行after函数
		
		void before()
		{
			printf("this is %s\n",__func__);			
		}
		void after()
		{
			printf("this is %s\n",__func__);	
		}
		
		int main()
		{
			printf("this is %s\n",__func__);
			return 0;	
		}
		输出信息：	this is before
					this is main
					this is after
	注意点：__attribute__适用于放在函数的声明位置，而不是函数的定义位置(用来设置变量属性的时候比如数组属性，则没这个说法了)
			__attribute__是GCC的关键字，VC中不能使用,keil使用的ARMCC编译器中也能使用
//-------------------------------------------------------------------------
用法2：
	__attribute__((section("段名")))	//将作用的函数或数据放入指定的输入段中
										//输入段/输出段概念：是相对于要生成最终的elf或binary时的link过程说的。
										//link过程中，输入的是由源代码编译生成的 .o 文件，这些 .o 文件包含的段就是输入段
										//link过程中，输出段是可执行文件elf或库，这些输出文件包含的段就是输出段
	例1：
		int var __attribute__((section(".data1"))) = 0;			//这样定义的变量var将被放入名为.data1的输入段
		static int __attribute__((section(".data2"))) func(void)//将函数func放入名为.data2的输入段
	例2（复杂点的，KL25里的）：
		typedef void(*vector_entry)(void);										//定义函数指针  
		__attribute__((section(".vectortable"))) vector_entry rom_vector[]=		//定义了一个数组，数组成员是函数指针；再把整个数组放入.vectortable输入段
		{
			//省略数组成员		
		}
//-------------------------------------------------------------------------
用法3：
	__attribute__((unused))		//如果某个函数或变量附带该属性，则意味着该函数或变量即使不使用，编译器也不会产生警告信息
	__attribute__((used))		//This attribute, attached to a function, means that code must be emitted for the function even if it appears that the function is not referenced. 
								//This is useful, for example, when the function is referenced only in inline assembly. 