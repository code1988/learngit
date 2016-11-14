1. 	Linux内核中，提供了一个用来创建双向循环链表的结构list_head，定义如下：
	struct list_head{
		struct list_head *next,*prev;	
	}; 
	该结构不带数据域，而是作为一个模块嵌入到需要使用链表功能的数据结构中，从而具备了通用性

2. 	链表初始化有2种方式，根据头节点是否已经创建，即头节点是否已经分配地址，来判断使用哪种方式
	方式1：
 	#define LIST_HEAD_INIT(name)	{&(name),&(name)}	
 	#define LIST_HEAD(name)			struct list_head name = LIST_HEAD_INIT(name)	// name.next = &name;name.prev = &name;
	这种方法创建一个链表头节点name，并且他的prev和next指针都指向自己，从而得到一张空链表
	
	方式2：
	inline void INIT_LIST_HEAD(struct list_head *list)
	{
		list->next = list;
		list->prev = list;	
	}
	这种方法基于一个已经创建的链表头节点list，使他的prev和next指针都指向自己，从而得到一张空链表
	注意点：头结点是不使用的，也就是头结点是没有没有父结构的，也就是不能调用list_entry宏获取父结构首地址，头结点仅用于连接真正带父结构的首尾节点


3.	linux的这种链表结构跟普通链表结构的本质区别在于：
	list_head节点中的next、prev指向的是下一个/上一个list_head节点地址，而不是包含list_head节点的父结构地址
	所以当我们要操作父结构中的其他成员时，必须首先由list_head节点在父结构中的地址偏移量计算得到父结构地址
	
	以下面结构为例：
		struct file_node{
			char c;
			struct list_head node;	
		};
		此时list_head就作为它的父结构中的一员了，当知道list_head的地址时，就可以通过list_entry宏来获取父结构地址，算法如下：
		
	#define offsetof(TYPE,MEMBER)	((size_t) &((TYPE *)0)->MEMBER)		// 计算MEMBER在TYPE中的偏移量，代码分解如下：
																		// (TYPE *)0;						将0地址强制转换为TYPE类型
																		// ((TYPE *)0)->MEMBER;				从0地址找到TYPE的成员MEMBER
																		// &((TYPE *)0)->MEMBER;			从0地址找到TYPE的成员MEMBER所在的地址
																		// (size_t)&((TYPE *)0)->MEMBER;	将地址强制转换成size_t类型，得到偏移量
	#define container_of(ptr,type,member)	({const typeof(((type *)0)->member) *__mptr = (ptr);\
											(type *)((char *)__mptr - offsetof(type,member));})		// container_of返回的结果就是ptr所在的父结构地址
																									// 使用中间变量__mptr的原因是解决宏定义可能存在的二义性缺陷
																									// __mptr是父结构体中list_head节点的地址
																									// offsetof宏求的是list_head节点在父结构体中的偏移量
																									// 两者相减，就得到父结构体的地址
	#define list_entry(jptr,type,member)	container_of(ptr,type,member)	// 所以list_entry宏功能就是，由结构体成员地址求结构体地址
																			// ptr 		- 所求结构体中的list_head成员指针
																			// type 	- 所求结构体类型
																			// member	- 所求结构体中list_head成员名
																			
											
	