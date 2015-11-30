/*
************ 	C标准库预定义宏
*/
__FILE__	//当前源代码文件名 包含绝对路径
__func__	//当前所在函数名
__DATE__	//日期
__TIME__	//时间
__LINE__	//当前行号

/*
************	assert()函数用法
*/
assert定义在assert.h中，作用是如果它的返回条件错误，则终止程序执行。
函数原型：	assert(int expression)		//计算表达式expression,如果为0，则打印一条错误信息，然后终止程序。

例：
	#include "stdio.h"
	#define NDEBUGE								//添加这条宏可以禁用assert函数调用
	#include "assert.h"
	
	char buf[4]="abc";
	void __attribute__((instruction)) before();
	void __attribute__((destruction)) after();
	
	int mian()
	{
		assert(buf[3] == '\0');					//判断数组第四位是否是 '\0',如果不是则程序终止
		printf("this is main, %s\n",buf);	
		return 0;
	}
	void before(void)
	{
		printf("this is before, %s\n",__func__);	
	}
	void after(void)
	{
		printf("this is after, %s\n",__DATE__);	
	}
	输出结果：
		this is before, before 
		this is main, abc
		this is after, May 19 2014
频繁调用assert会影响程序性能，增加额外开销。可以通过在	#include "assert.h"	语句之前插入	#define NDEBUG	来禁用assert调用

/*
************	随机数rand()函数用法
*/
rand定义在stdlib.h头文件中，原型为：
int rand(void);				//返回一个[0,RAND_MAX]间的随机整数
							//RAND_MAX是个标准库里的宏定义，通常为32767
					
rand()函数需要配合srand()函数一起使用，系统在调用rand()函数前都会自动调用srand()函数；
srand()函数会设置供rand()使用的随机数种子，原型为：
void srand(unsigned int)；	//形参即rand()的种子，通常采用(unsigned int)time(NULL)即系统定时器做种子
例：
	#include "stdio.h"
	#include "stdlib.h"
	#include "time.h"
	int main()
	{
		int i;
		srand((unsigned int)time(NULL));
		for(i=0;i<10;i++)
			printf("%f\n",(float)rand()/RAND_MAX);
	}
	
/*
************	printf、sprintf、fprintf	3个标准库函数之间的区别
*/
printf是标准输出流的输出函数，用来向屏幕这样的标准输出设备输出。
	格式：int printf(const char *format,...)				//固定形参+可变形参
sprintf将某一类型的数据写入某个字符串缓冲区中。
	格式：int sprintf(char *buffer,const char *format,...)	//字符串缓冲区+固定形参+可变形参
fprintf将某一类型的数据写入流文件中，即输出对象是文件。
	格式：int fprintf(FILE *stream,const char *format,...)	//流文件（实质是存储文件信息的结构体指针）+固定形参+可变形参
		