Linux�ں��У��ṩ��һ����������˫��ѭ������Ľṹlist_head���������£�
	struct list_head{
		struct list_head *next,*prev;	
	}; 
�ýṹ���������򣬶�����Ϊһ��ģ��Ƕ�뵽��Ҫʹ�������ܵ����ݽṹ�У��Ӷ��߱���ͨ����

1. �����ʼ����
	��ʽ1��
 	#define LIST_HEAD_INIT(name)	{&(name),&(name)}	
 	#define LIST_HEAD(name)			struct list_head name = LIST_HEAD_INIT(name)	// name.next = &name;name.prev = &name;
	���ַ�������һ������ͷ�ڵ�name������ͷ�ڵ���и�ֵ��ʹǰ���ͺ�̶�ָ���Լ���ʵ��ѭ��