Linux�ں��У��ṩ��һ����������˫��ѭ������Ľṹlist_head���������£�
	struct list_head{
		struct list_head *next,*prev;	
	}; 
�ýṹ���������򣬶�����Ϊһ��ģ��Ƕ�뵽��Ҫʹ�������ܵ����ݽṹ�У��Ӷ��߱���ͨ����

�����ʼ����2�ַ�ʽ������ͷ�ڵ��Ƿ��Ѿ���������ͷ�ڵ��Ƿ��Ѿ������ַ�����ж�ʹ�����ַ�ʽ
	��ʽ1��
 	#define LIST_HEAD_INIT(name)	{&(name),&(name)}	
 	#define LIST_HEAD(name)			struct list_head name = LIST_HEAD_INIT(name)	// name.next = &name;name.prev = &name;
	���ַ�������һ������ͷ�ڵ�name������ͷ�ڵ���и�ֵ��ʹǰ���ͺ�̶�ָ���Լ���ʵ��ѭ��
	��ʽ2��
	inline void INIT_LIST_HEAD(struct list_head *list)
	{
		list->next = list;
		list->prev = list;	
	}
	���ַ�������һ���Ѿ�����������ͷ�ڵ�list�����ͷ�ڵ㸳ֵ��ʹǰ���ͺ�̶�ָ���Լ���ʵ��ѭ��

������ṹΪ����
	struct file_node{
		char c;
		struct list_head node;	
	};
	��ʱlist_head����Ϊ���ĸ��ṹ�е�һԱ�ˣ���֪��list_head�ĵ�ַʱ���Ϳ���ͨ��list_entry������ȡ���ṹ��ַ���㷨���£�
	