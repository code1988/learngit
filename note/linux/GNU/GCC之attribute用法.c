//=======================================================================
__attribute__�������ú������ԡ��������ԡ��������ԡ�
__attribute__��д��ʽ��
	__attribute__((����))		//���������Ƕ��

//-----------------------------------------------------------------------
�÷�1��
	__attribute__((constructor))	//����ĳ����������Ϊconstructor���ԣ���ú�������main()����ִ��֮ǰ���Զ�ִ��
	__attribute__((destructor))		//����ĳ����������Ϊdestructor���ԣ���ú�������main()����ִ��֮���Զ�ִ��
	����
		#include "stdio.h"
		
		void before() __attribute__((constructor));		//��main����֮ǰִ��before����
		void after() __attribute__((destructor));		//��mian����֮��ִ��after����
		
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
		�����Ϣ��	this is before
					this is main
					this is after
	ע��㣺__attribute__�����ڷ��ں���������λ�ã������Ǻ����Ķ���λ��(�������ñ������Ե�ʱ������������ԣ���û���˵����)
			__attribute__��GCC�Ĺؼ��֣�VC�в���ʹ��,keilʹ�õ�ARMCC��������Ҳ��ʹ��
//-------------------------------------------------------------------------
�÷�2��
	__attribute__((section("����")))	//�����õĺ��������ݷ���ָ�����������
										//�����/����θ���������Ҫ�������յ�elf��binaryʱ��link����˵�ġ�
										//link�����У����������Դ����������ɵ� .o �ļ�����Щ .o �ļ������Ķξ��������
										//link�����У�������ǿ�ִ���ļ�elf��⣬��Щ����ļ������Ķξ��������
	��1��
		int var __attribute__((section(".data1"))) = 0;			//��������ı���var����������Ϊ.data1�������
		static int __attribute__((section(".data2"))) func(void)//������func������Ϊ.data2�������
	��2�����ӵ�ģ�KL25��ģ���
		typedef void(*vector_entry)(void);										//���庯��ָ��  
		__attribute__((section(".vectortable"))) vector_entry rom_vector[]=		//������һ�����飬�����Ա�Ǻ���ָ�룻�ٰ������������.vectortable�����
		{
			//ʡ�������Ա		
		}
//-------------------------------------------------------------------------
�÷�3��
	__attribute__((unused))		//���ĳ��������������������ԣ�����ζ�Ÿú����������ʹ��ʹ�ã�������Ҳ�������������Ϣ
	__attribute__((used))		//This attribute, attached to a function, means that code must be emitted for the function even if it appears that the function is not referenced. 
								//This is useful, for example, when the function is referenced only in inline assembly. 