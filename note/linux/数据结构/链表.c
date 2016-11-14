1. 	Linux�ں��У��ṩ��һ����������˫��ѭ������Ľṹlist_head���������£�
	struct list_head{
		struct list_head *next,*prev;	
	}; 
	�ýṹ���������򣬶�����Ϊһ��ģ��Ƕ�뵽��Ҫʹ�������ܵ����ݽṹ�У��Ӷ��߱���ͨ����

2. 	�����ʼ����2�ַ�ʽ������ͷ�ڵ��Ƿ��Ѿ���������ͷ�ڵ��Ƿ��Ѿ������ַ�����ж�ʹ�����ַ�ʽ
	��ʽ1��
 	#define LIST_HEAD_INIT(name)	{&(name),&(name)}	
 	#define LIST_HEAD(name)			struct list_head name = LIST_HEAD_INIT(name)	// name.next = &name;name.prev = &name;
	���ַ�������һ������ͷ�ڵ�name����������prev��nextָ�붼ָ���Լ����Ӷ��õ�һ�ſ�����
	
	��ʽ2��
	inline void INIT_LIST_HEAD(struct list_head *list)
	{
		list->next = list;
		list->prev = list;	
	}
	���ַ�������һ���Ѿ�����������ͷ�ڵ�list��ʹ����prev��nextָ�붼ָ���Լ����Ӷ��õ�һ�ſ�����
	ע��㣺ͷ����ǲ�ʹ�õģ�Ҳ����ͷ�����û��û�и��ṹ�ģ�Ҳ���ǲ��ܵ���list_entry���ȡ���ṹ�׵�ַ��ͷ���������������������ṹ����β�ڵ�


3.	linux����������ṹ����ͨ����ṹ�ı����������ڣ�
	list_head�ڵ��е�next��prevָ�������һ��/��һ��list_head�ڵ��ַ�������ǰ���list_head�ڵ�ĸ��ṹ��ַ
	���Ե�����Ҫ�������ṹ�е�������Աʱ������������list_head�ڵ��ڸ��ṹ�еĵ�ַƫ��������õ����ṹ��ַ
	
	������ṹΪ����
		struct file_node{
			char c;
			struct list_head node;	
		};
		��ʱlist_head����Ϊ���ĸ��ṹ�е�һԱ�ˣ���֪��list_head�ĵ�ַʱ���Ϳ���ͨ��list_entry������ȡ���ṹ��ַ���㷨���£�
		
	#define offsetof(TYPE,MEMBER)	((size_t) &((TYPE *)0)->MEMBER)		// ����MEMBER��TYPE�е�ƫ����������ֽ����£�
																		// (TYPE *)0;						��0��ַǿ��ת��ΪTYPE����
																		// ((TYPE *)0)->MEMBER;				��0��ַ�ҵ�TYPE�ĳ�ԱMEMBER
																		// &((TYPE *)0)->MEMBER;			��0��ַ�ҵ�TYPE�ĳ�ԱMEMBER���ڵĵ�ַ
																		// (size_t)&((TYPE *)0)->MEMBER;	����ַǿ��ת����size_t���ͣ��õ�ƫ����
	#define container_of(ptr,type,member)	({const typeof(((type *)0)->member) *__mptr = (ptr);\
											(type *)((char *)__mptr - offsetof(type,member));})		// container_of���صĽ������ptr���ڵĸ��ṹ��ַ
																									// ʹ���м����__mptr��ԭ���ǽ���궨����ܴ��ڵĶ�����ȱ��
																									// __mptr�Ǹ��ṹ����list_head�ڵ�ĵ�ַ
																									// offsetof�������list_head�ڵ��ڸ��ṹ���е�ƫ����
																									// ����������͵õ����ṹ��ĵ�ַ
	#define list_entry(jptr,type,member)	container_of(ptr,type,member)	// ����list_entry�깦�ܾ��ǣ��ɽṹ���Ա��ַ��ṹ���ַ
																			// ptr 		- ����ṹ���е�list_head��Աָ��
																			// type 	- ����ṹ������
																			// member	- ����ṹ����list_head��Ա��
																			
											
	