Linux内核中，提供了一个用来创建双向循环链表的结构list_head，定义如下：
	struct list_head{
		struct list_head *next,*prev;	
	}; 
该结构不带数据域，而是作为一个模块嵌入到需要使用链表功能的数据结构中，从而具备了通用性

链表初始化有2种方式，根据头节点是否已经创建，即头节点是否已经分配地址，来判断使用哪种方式
	方式1：
 	#define LIST_HEAD_INIT(name)	{&(name),&(name)}	
 	#define LIST_HEAD(name)			struct list_head name = LIST_HEAD_INIT(name)	// name.next = &name;name.prev = &name;
	这种方法创建一个链表头节点name，并对头节点进行赋值，使前驱和后继都指向自己，实现循环
	方式2：
	inline void INIT_LIST_HEAD(struct list_head *list)
	{
		list->next = list;
		list->prev = list;	
	}
	这种方法基于一个已经创建的链表头节点list，完成头节点赋值，使前驱和后继都指向自己，实现循环

以下面结构为例：
	struct file_node{
		char c;
		struct list_head node;	
	};
	此时list_head就作为它的父结构中的一员了，当知道list_head的地址时，就可以通过list_entry宏来获取父结构地址，算法如下：
	