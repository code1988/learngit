namespace��Ϊ�˷�ֹȫ�ֱ���������������C++������ĸ��namespace�����ࡢ���󡢺����ۼ���һ��������
1. ʹ�ã�������������namespace�������ı���
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
	
2. ʹ�ùؼ���using���Դ�namespace�����������ı�������ǰ�Ĺ�������Ҳ������������namespace
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
	
3. namespace֧��Ƕ��
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

4. namespace��������һ����÷�
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