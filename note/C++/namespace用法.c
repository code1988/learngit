namespace是为了防止全局变量出现重名而在C++中引入的概念，namespace允许类、对象、函数聚集在一个名字下
1. 使用：：操作符调用namespace中声明的变量
	using namespace std;
	namespace first
	{
		int a = 1;	
	}
	namespace second
	{
		double a = 3.14;	
	}
	int main()
	{
		cout<<first::a<<endl;
		cout<<second::a<<endl;
		return 0;	
	}
	
2. 使用关键字using可以从namespace中引入声明的变量到当前的工作区域，也可以引入整个namespace
	using namespace std;
	namespace first
	{
		int a = 1;	
	}
	namespace second
	{
		double b = 3.14;	
	}
	int main()
	{
		using first::a;
		using namespace second;
		
		cout<<a<<endl;
		cout<<b<<endl;
		return 0;	
	}
	
3. namespace支持嵌套
	using namespace std;
	namespace first
	{
		int a = 1;	
		namespace second
		{
			double b = 3.14;
			void hello();	
		}
		
		void second::hello()
		{
			cout <<"hello world!"<<endl;	
		}
	}
	int main()
	{
		using namespace first;
		
		cout<<second::b<<endl;
		second::hello();
		return 0;
	}

4. namespace和类结合在一起的用法
	using namespace std;
	namespace first
	{
		namespace second
		{
			class Point
			{
				public:
					void setpoint(int x,int y);
					void printpoint();
				private:
					int xpos;
					int ypos;	
			};
			void Point::setpoint(int x,int y)
			{
				xpos = x;
				ypos = y;	
			}
			void Point::printpoint()
			{
				cout<<"x = "<<xpos<<endl;
				cout<<"y = "<<ypos<<endl;	
			}	
		}	
	}
	int main()
	{
		using namespace first;
		second:Point point;
			
		point.setpoint(2,3);
		point.printpoint();
		return 0;	
	}