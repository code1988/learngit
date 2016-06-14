//typedef用法总结
//-------------------------------------------------------------------------------------------
用途一：
定义一种类型的别名，而不是简单的宏替换，可以用于同时声明指针型的多个变量
例：
	char* pa,nb;		//它其实只声明了一个指向字符变量的指针pa，和一个字符变量nb
	
	typedef char* putchar;
	putchar pa,nb;		//它才是定义了两个指向字符变量的指针pa,nb

用途二：
C代码在声明struct新对象时，形式为：struct 结构名 结构变量名
而在C++中，可以直接这样写：结构名 结构变量名
即，同样代码：
	struct A
	{
		char b;	
	}C;
	void main(void)
	{
		A d,e;	
	}
在gcc编译器中就会报错，而在g++编译器中则不会。
但如果这样写：
	typedef struct A
	{
		char b;	
	}C;
	void main(void)
	{
		C d,e;	
	}
在以上两种编译环境中就都不会报错了。

用途三：
可以用typedef定义与平台无关无关的数据类型，比如一个叫REAL的浮点类型，
在目标平台上：
	typedef long double REAL;
在不支持long double的平台上，可做如下修改：
	typedef float REAL;
也就是说，当跨平台时，只要改一下typedef本身就行，不用对其它源码作任何修改。

用途四：
用于定义函数指针，用法：
	typedef 返回类型 (*新类型)(参数)
例：
	#include "stdio.h"
	#include "assert.h"
			
	typedef int (*FP_CALC)(int,int);			//定义一种名为FP_CALC的函数指针类型，该指针指向某种函数：
												//有2个int型的参数，返回1个int类型
	int add(int a,int b)
	{
		return (a+b);
	}
	int sub(int a,int b)
	{
		return (a-b);
	}
	int mul(int a,int b)
	{
		return (a*b);
	}
	int div(int a,int b)
	{
		return (a/b);
	}
	
	FP_CALC calc_func(char op)					//定义一个函数calc_func,参数为字符变量op，返回一个指针，该指针的类型为有2个int参数，返回类型为int的函数指针
	{											//在代码里的功能是：通过判断运算符，返回相应的运算函数地址
		switch(op)
		{
			case '+':
				return add;
			case '-':
				return sub;
			case '*':
				return mul;
			case '/':
				return div;
			default:
				return NULL;
		}
	}
	
	int calc(int a,int b,char op)				//最终用户直接调用的函数，实现功能：接收2个int整数和1个运算符，返回两数的运算结果
	{
		FP_CALC fp = calc_func(op);				//注意这里函数返回值其实是一个地址，指向FP_CALC类型的函数。这里只是把地址赋值给fp，尚未执行
		if(fp != NULL)
			return fp(a,b);						//这里才是执行地址上的函数
		else
			return -1;
	}
	
	int main()
	{
		int a = 100;
		int b = 20;
		printf("%d %c %d = %d\n",a,'+',b,calc(a,b,'+'));
		printf("%d %c %d = %d\n",a,'-',b,calc(a,b,'-'));
		printf("%d %c %d = %d\n",a,'*',b,calc(a,b,'*'));
		printf("%d %c %d = %d\n",a,'/',b,calc(a,b,'/'));
		while(1);
		return 0;
	}