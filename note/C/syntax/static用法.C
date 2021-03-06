﻿/*********************************************************************************
内存空间结构模型：
	高地址
	
		程序栈(向下生长)	//局部变量和函数参数
		
		......
		
		堆(向上生长)		//动态分配内存区域，如malloc申请的内存空间
		未初始化数据段(BSS)	//存放未初始化的全局变量和静态变量，
		初始化数据段		//存放程序中所有赋了初值的全局变量和静态非零变量
		
		代码				//存储程序文本
	
	低地址
	
********************************************************************************/
1. 全局静态变量
static + 全局变量 = 全局静态变量
静态全局变量生存期为整个源程序，但限制了其作用域，只在定义该变量的源文件内有效，同一源程序的其他源文件不能使用它。
未初始化的静态全局变量会被自动初始化为0

2. 局部静态变量
static + 局部变量 = 局部静态变量
局部静态变量在函数内定义，生存期为整个源程序，但只能在定义该变量的函数内使用，退出该函数后，尽管该变量还在，但不能使用它。
未初始化的静态局部变量会被自动初始化为0
有些时候，在函数中必须要使用static变量，比如当某函数的返回值为指针类型时，则必须是static类型的局部变量的地址作为返回值，若为auto型，则返回错指针。

当static用来修饰局部变量时，它就改变了局部变量的存储位置，从栈里放到了静态存储区；当static修饰全局变量时，它只改变了作用域，存储位置不变。

3. 静态函数(内部函数)
函数默认是extern的，在函数的返回类型前加上static，函数就被定义为静态函数，静态函数只在声明它的源文件里可见，而不能被其他源文件调用。
使用内部函数的好处是：不同人编写不同源文件时，不用担心函数名是否会与其他源文件里的函数重名。



