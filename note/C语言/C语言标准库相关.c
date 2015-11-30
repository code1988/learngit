/*
************ 	C��׼��Ԥ�����
*/
__FILE__	//��ǰԴ�����ļ��� ��������·��
__func__	//��ǰ���ں�����
__DATE__	//����
__TIME__	//ʱ��
__LINE__	//��ǰ�к�

/*
************	assert()�����÷�
*/
assert������assert.h�У�������������ķ���������������ֹ����ִ�С�
����ԭ�ͣ�	assert(int expression)		//������ʽexpression,���Ϊ0�����ӡһ��������Ϣ��Ȼ����ֹ����

����
	#include "stdio.h"
	#define NDEBUGE								//�����������Խ���assert��������
	#include "assert.h"
	
	char buf[4]="abc";
	void __attribute__((instruction)) before();
	void __attribute__((destruction)) after();
	
	int mian()
	{
		assert(buf[3] == '\0');					//�ж��������λ�Ƿ��� '\0',��������������ֹ
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
	��������
		this is before, before 
		this is main, abc
		this is after, May 19 2014
Ƶ������assert��Ӱ��������ܣ����Ӷ��⿪��������ͨ����	#include "assert.h"	���֮ǰ����	#define NDEBUG	������assert����

/*
************	�����rand()�����÷�
*/
rand������stdlib.hͷ�ļ��У�ԭ��Ϊ��
int rand(void);				//����һ��[0,RAND_MAX]����������
							//RAND_MAX�Ǹ���׼����ĺ궨�壬ͨ��Ϊ32767
					
rand()������Ҫ���srand()����һ��ʹ�ã�ϵͳ�ڵ���rand()����ǰ�����Զ�����srand()������
srand()���������ù�rand()ʹ�õ���������ӣ�ԭ��Ϊ��
void srand(unsigned int)��	//�βμ�rand()�����ӣ�ͨ������(unsigned int)time(NULL)��ϵͳ��ʱ��������
����
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
************	printf��sprintf��fprintf	3����׼�⺯��֮�������
*/
printf�Ǳ�׼������������������������Ļ�����ı�׼����豸�����
	��ʽ��int printf(const char *format,...)				//�̶��β�+�ɱ��β�
sprintf��ĳһ���͵�����д��ĳ���ַ����������С�
	��ʽ��int sprintf(char *buffer,const char *format,...)	//�ַ���������+�̶��β�+�ɱ��β�
fprintf��ĳһ���͵�����д�����ļ��У�������������ļ���
	��ʽ��int fprintf(FILE *stream,const char *format,...)	//���ļ���ʵ���Ǵ洢�ļ���Ϣ�Ľṹ��ָ�룩+�̶��β�+�ɱ��β�
		