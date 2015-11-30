										构造函数和析构函数
/***********************************************************************************************************
构造函数的特点：在对象被创建时自动执行
				构造函数名与类名相同
				没有返回值
				构造函数不能被显式调用
				
构造函数的初始化表：对象中的一些数据成员除了在构造函数体中进行初始化外，还可以通过调用初始化表完成
					在构造函数头后面，通过单个冒号：引出
					使用初始化表进行初始化是在构造函数被调用之前就完成的，这点与构造函数体内进行初始化不同
					初始化顺序取决于数据成员在类中声明的顺序				

析构函数特点：	析构函数名也跟类名相同，但是在紧贴函数名前加了波浪号~
				析构函数没有返回值，也没有形参，也不能重载
				当对象被撤销时，析构函数自动被调用

例：
	#include <iostream>
	#include <string>
	using namespace std;
	
	bool val;
	
	class Fruit
	{
		string name;
		string color;
	
	public:
		void print()
		{
			cout<<color<<" "<<name<<endl;
		}
		Fruit(const string nm="apple",const string clr="green"):name(nm),color(clr)
		{
			val = true;
		}
		~Fruit()
		{
			val = false;
		}
	};
	
	int main()
	{
		{
			Fruit banana("banana","yellow");
			banana.print();
			cout<<"val is "<<val<<endl;
		}
		cout<<"val is "<<val<<endl;
		
		Fruit apple;
		apple.print();
		
		return 0;
	}	
	备注：大括号限定了类banana的作用域				
***********************************************************************************************************/	

							
										复制构造函数
/***********************************************************************************************************
复制构造函数是一种特殊的构造函数，所以拥有构造函数的所有特点，并且，第一个形参必须是本类型的一个引用变量
格式：
	class M
	{
		public:
		M(形参)	// 构造函数的声明/定义
		M(M& m)	// 复制构造函数的声明/定义
	}
	M::M(M& m){}	// 复制构造函数定义
	
拷贝构造函数的调用时机：	
	1. 	对象以值传递的方式传入函数参数
		class M
		{
			int a;
		public:
			// 构造函数
			M(int b=100):a(b){cout<<"creat"<<endl;}
			// 复制构造函数
			M(const M& m):a(m.a){cout<<"copy"<<endl;}
			// 析构函数
			~M(){cout<<"delete"<<endl;}
		};
		
		void g_Fun(M n){cout<<"This is g_Fun!"<<endl;}
		
		int main()
		{
			M test;
			
			g_Fun(test);
			
			return 0;
		}
		
		结果：	creat
				copy
				This is g_Fun!
				delete
				delete
		备注：	test对象传入函数g_Fun时，会调用复制构造函数把对象test的值给对象C，等g_Fun()执行完后，析构掉对象C
		
	2.	对象以值传递的方式从函数返回
		class M
		{
			int a;
		public:
			// 构造函数
			M(int b=100):a(b){cout<<"creat"<<endl;}
			// 复制构造函数
			M(const M& m):a(m.a){cout<<"copy"<<endl;}
			// 析构函数
			~M(){cout<<"delete"<<endl;}
		};
		
		M g_Fun()
		{
			M m;
			
			cout<<"This is g_Fun!"<<endl;
			
			return m;
		}
		
		int main()
		{
			g_Fun();
			
			return 0;
		}
		
		结果：	creat
				This is g_Fun!
				copy
				delete
				delete
		备注：	g_Fun()函数执行到return时，会先产生一个临时对象tmp，然后调用复制构造函数把对象m的值给对象tmp，接着析构掉对象m，等g_Fun()执行完后，最后析构掉tmp
		
	3. 	一个对象给另外一个对象进行初始化
		M m;		// 定义一个对象并通过构造函数完成初始化
		M n(m);		// 调用复制构造函数完成对象n初始化，赋值写法1
		M x = m;	// 调用复制构造函数完成对象x初始化，赋值写法2
		
复制构造函数的浅拷贝和深拷贝之分：
	在没有显式定义复制构造函数的情况下，编译器自动产生一个默认复制构造函数，只对对象中的数据成员进行简单的赋值，这就是浅拷贝
	对于对象中的动态成员，重新动态分配空间，完成对象的复制后，各自指向一段内存空间，但具有相同的内容，这就是深拷贝
***********************************************************************************************************/	
