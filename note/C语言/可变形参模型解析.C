原理：根据参数入栈的特点从最靠近第一个可变参数的固定参数开始，依次获取每个可变参数的地址。
例：
#include "stdio.h"
#include "stdlib.h"

void myprintf(char *fmt,...)							//一个类printf函数的实现
{
	char *ptr=NULL;										//(void*)NULL在stdio.h文件声明
	
	ptr = (char *)&fmt;									//将存放固定参数的内存地址(仅仅是首地址)推入栈，再将这个栈的内存地址强制转换成char*
	ptr += 4;											//存放固定参数的内存地址(2^32，仅仅是首地址)为4byte的倍数，首地址长度为4
														//将栈指针指向下一个地址，即存放第一个可变形参的地址处，与前一个地址不同的是，此处栈地址里存放的不是地址值，而是真实的可变形参值
	do
	{
		if(*fmt != '%')									//判断固定参数中的每个字符是否等于‘%’
			putchar(*fmt);								//直接输出字符
		else
		{
			switch(*(++fmt))	
			{
				case 'd':							 	//如果字符‘%’后接‘d’，则栈指针指向的可变形参十进制输出 
					printf("%d",*(int*)ptr);			//在这种myprintf写法中，此处的*(int*)ptr写法不可省略，否则会出错，详见底部
					break;
				case 'x':
					printf("%#x",*(int*)ptr);
					break;
				default:
					break;	
			}
			ptr +=4;									//栈指针移动一个内存地址								
		}
		++fmt;											//指针作形参时可以写fmt++形式，否则禁止
	}while(*fmt != '\0');								//判断固定参数是否结尾
	ptr = NULL;											//清空栈指针
	return;
}

myprintf另一种写法：
void myprintf(char *fmt,...)							
{
	int *ptr=NULL;
	
	ptr = (int*)&fmt;									//将存放固定参数的内存空间推入栈中，栈地址必须强制转换成与左边一样的类型才能做赋值运算						
	ptr += 1;											
	do
	{
		if(*fmt != '%')									
			putchar(*fmt);								
		else
		{
			switch(*(++fmt))	
			{
				case 'd':
					printf("%d",*ptr);
					break;
				case 'x':
					printf("%#x",*ptr);
					break;
				default:
					break;	
			}
			ptr +=1;																	
		}
		++fmt;
	}while(*fmt != '\0');
	ptr = NULL;
	return;
}

void main(void)
{
	int a=12,b=13;
	myprintf("the output is :%d\n%d\n%x\n",a,b,0xabcd);
	return;
}

//------------------------------------------------------------------------------------------------------
错误案例：
void myprintf(char *fmt,...)							
{
	char *ptr=NULL;
	
	ptr = (char*)&fmt;													
	ptr += 4;											
	do
	{
		if(*fmt != '%')									
			putchar(*fmt);								
		else
		{
			switch(*(++fmt))	
			{
				case 'd':
					printf("%d",*ptr);			//此处应该是*(int*)ptr,因为栈ptr的每个元素在开始处被定义为了char型，当推入栈的可变形参处于char范围内时(如b=13)，main函数结果正常；
												//而当可变形参超出char的范围时(a=12111)，a放入ptr时还是完整的4byte数据，但当从这里取出来时，只截取了2byte数据，所以导致不全
												//解决方法就是将栈空间里的数据还原成int型，从而以4byte为单位取
					break;
				case 'x':
					printf("%#x",*ptr);
					break;
				default:
					break;	
			}
			ptr +=4;																	
		}
		++fmt;
	}while(*fmt != '\0');
	ptr = NULL;
	return;
}

void main(void)
{
	int a=12111,b=13;
	myprintf("the output is :%d\n%d\n%x\n",a,b,0xabcd);
	return;
}