//
/**********  链表：就是用一组任意的存储单元存储线性表元素的一种数据结构  **************/
//
单链表：数据节点是单向排列的，一个结点，其结构分为两部分：
	数据域：用于存储本身数据
	链域：用来存储下一个结点地址
	例：
		typedef struct node
		{
			char name[20];
			struct node *link;	
		}stud;
	//--------------------------------------------------------------------
	单链表完整程序（标准格式）：
	#include <stdio.h> 
	#include <malloc.h> 								/*包含动态内存分配函数的头文件*/ 
	#define N 10 										/*N为人数*/
	 
	typedef struct node									/*链表结构声明*/
	{ 
		char name[20]; 
		struct node *link; 
	}stud; 
	 
	stud *creat(int n) 									/*建立单链表的函数，形参n为人数*/ 
	{ 
		stud *p,*h,*s; 									/* *h保存表头结点的指针，*p指向当前结点的前一个结点，*s指向当前结点*/
		int i;											/*计数器*/ 
		if( (h=(stud *)malloc(sizeof(stud))) == NULL) 	/*分配空间并检测（返回一个指向stud形式的结构体的指针，指针里面存放的仅仅是该结构体的首地址）*/ 
		{ 
			printf("不能分配内存空间!"); 
			exit(0); 
		} 
		h->name[0]='\0'; 								/*把表头结点的数据域置空*/ 
		h->link=NULL; 									/*把表头结点的链域置空*/
		p=h; 											/*p指向表头结点*/
		for(i=0;i<n;i++) 
		{ 
			if((s= (stud *)malloc(sizeof(stud)))==NULL)	/*分配新存储空间并检测*/ 
			{ 
				printf("不能分配内存空间!"); 
				exit(0); 
			} 
			p->link=s; 									/*把s的地址赋给p所指向的结点（上一个结点！！！）的链域，这样就把p和s所指向的结点连接起来了*/
														/*此刻的p还是上一个结点，p中的链域为NULL，新的链域在这行代码被赋予，NULL被覆盖*/
			/*当前结点s内容开始填充*/
			printf("请输入第%d个人的姓名",i+1); 
			scanf("%s",s->name); 						/*在当前结点s的数据域中存储姓名*/
			s->link=NULL;								/*每次开辟新结点s时，其链域总是先用NULL填充，等到下一个结点开辟时，再回头重新填充链域值*/
			/*把填充完后的当前结点s往前挪一结，目的是使s空出来，可以被用于开辟下一个新的结点*/
			p=s; 
		} 
		return(h); 										/*返回链表表头结点地址的指针*/
	} 
	
	stud *search(stud *h,char *user_name)				/*查找链表的函数，h指针是链表的表头指针，user_name是要查找的人都姓名*/
	{
		stud *s;										/*当前指针，指向与所要查找的姓名进行比较的结点*/
		char *y;										/*保存结点数据域内姓名的指针*/
		s = h->link;
		while(s != NULL)
		{
			y = s->name;
			if(strcmp(user_name,y)==0)					/*把数据域里的姓名跟所要查找的姓名进行比较，若相同则返回0*/
			{
				return s;								/*返回所要查找的结点地址*/
			}	
			else
				s = s->link;							/*若当前节点姓名不符合，则准备进入下一个结点查找*/
		} 
		return NULL;
	}
	
	stud *search2(stud *h,char *user_name)				/*查找链表函数，与前一个查找函数的区别仅仅在于返回的是直接前驱结点的指针，而前一个查找函数返回的是当前结点的指针*/
	{
		stud *p,*s;										/*p指针指向前驱结点，s指针指向当前结点*/
		char *y;
		
		p = h;
		s = h->link;
		while(s != NULL)
		{
			y = s->name;
			if(strcmp(user_name,y) == 0)
				return p;								/*返回前驱结点*/
			else
			{
				p = s;
				s = s->link;
			}
		}
		return NULL;
	}
	
	void insert(stud *p)								/*插入结点函数，在指针p后插入*/
	{
		stud *s;										/*当前指针，指向要插入的结点*/
		char user_name[10];								/*要新增的姓名*/
		if((s=malloc(sizeof(stud))) == NULL)			/*分配新存储空间并检测*/ 
		{
			printf("不能分配内存空间\n");
			exit(1);
		}
		printf("新增姓名：\n");
		scanf("%s",user_name);
		strcpy(s->name,user_name);						/*新结点内容填充，将p的链域存入s的链域中，而p的链域重新存入指向s结点的指针*/
		s->link = p->link;
		p->link = s;
	}

	int main() 
	{ 
		int number; 									/*保存人数的变量*/ 
		stud *head,*user_p,*user_s; 					/*head是保存单链表的表头结点地址的指针*/
		char full_name[10];
		
		number=N; 
		head = creat(number); 							/*把所新建的单链表表头地址赋给head*/ 
		printf("enter the name you want to search:\n");
		scanf("%s",full_name);
		user_p = search(head,full_name);				/*查找链表,返回当前结点*/
		if(user_p != NULL)
		{
			printf("name have be got\n");
			insert(user_p);								/*插入新结点*/
			printf("新增姓名成功\n");
			getchar();									/*随便敲个字符*/
			getchar();
			user_s = search2(head,"宋尧飞");			/*再次查找链表，返回前驱结点*/
			if(user_s != NULL)
			{
				del(user_p,user_s);						/*删除user_p结点，此处即为姓名宋尧飞对应的结点*/
				printf("删除姓名：宋尧飞成功\n");
			}
			else
				printf("不存在姓名：宋尧飞\n");
		}	
		else
			printf("不存在姓名：%s\n",full_name);
		getchar();	
		return 0;
	}
