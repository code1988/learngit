Linux内核中，提供了一个用来创建双向循环链表的结构list_head，定义如下：
	struct list_head{
		struct list_head *next,*prev;	
	}; 
该结构不带数据域，而是作为一个模块嵌入到需要使用链表功能的数据结构中，从而具备了通用性

1. 链表初始化：
	方式1：
 	#define LIST_HEAD_INIT(name)	{&(name),&(name)}	
 	#define LIST_HEAD(name)			struct list_head name = LIST_HEAD_INIT(name)	// name.next = &name;name.prev = &name;
	这种方法创建一个链表头节点name，并对头节点进行赋值，使前驱和后继都指向自己，实现循环