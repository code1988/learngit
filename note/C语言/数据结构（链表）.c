//
/**********  ����������һ������Ĵ洢��Ԫ�洢���Ա�Ԫ�ص�һ�����ݽṹ  **************/
//
���������ݽڵ��ǵ������еģ�һ����㣬��ṹ��Ϊ�����֣�
	���������ڴ洢��������
	���������洢��һ������ַ
	����
		typedef struct node
		{
			char name[20];
			struct node *link;	
		}stud;
	//--------------------------------------------------------------------
	�������������򣨱�׼��ʽ����
	#include <stdio.h> 
	#include <malloc.h> 								/*������̬�ڴ���亯����ͷ�ļ�*/ 
	#define N 10 										/*NΪ����*/
	 
	typedef struct node									/*����ṹ����*/
	{ 
		char name[20]; 
		struct node *link; 
	}stud; 
	 
	stud *creat(int n) 									/*����������ĺ������β�nΪ����*/ 
	{ 
		stud *p,*h,*s; 									/* *h�����ͷ����ָ�룬*pָ��ǰ����ǰһ����㣬*sָ��ǰ���*/
		int i;											/*������*/ 
		if( (h=(stud *)malloc(sizeof(stud))) == NULL) 	/*����ռ䲢��⣨����һ��ָ��stud��ʽ�Ľṹ���ָ�룬ָ�������ŵĽ����Ǹýṹ����׵�ַ��*/ 
		{ 
			printf("���ܷ����ڴ�ռ�!"); 
			exit(0); 
		} 
		h->name[0]='\0'; 								/*�ѱ�ͷ�����������ÿ�*/ 
		h->link=NULL; 									/*�ѱ�ͷ���������ÿ�*/
		p=h; 											/*pָ���ͷ���*/
		for(i=0;i<n;i++) 
		{ 
			if((s= (stud *)malloc(sizeof(stud)))==NULL)	/*�����´洢�ռ䲢���*/ 
			{ 
				printf("���ܷ����ڴ�ռ�!"); 
				exit(0); 
			} 
			p->link=s; 									/*��s�ĵ�ַ����p��ָ��Ľ�㣨��һ����㣡�����������������Ͱ�p��s��ָ��Ľ������������*/
														/*�˿̵�p������һ����㣬p�е�����ΪNULL���µ����������д��뱻���裬NULL������*/
			/*��ǰ���s���ݿ�ʼ���*/
			printf("�������%d���˵�����",i+1); 
			scanf("%s",s->name); 						/*�ڵ�ǰ���s���������д洢����*/
			s->link=NULL;								/*ÿ�ο����½��sʱ����������������NULL��䣬�ȵ���һ����㿪��ʱ���ٻ�ͷ�����������ֵ*/
			/*��������ĵ�ǰ���s��ǰŲһ�ᣬĿ����ʹs�ճ��������Ա����ڿ�����һ���µĽ��*/
			p=s; 
		} 
		return(h); 										/*���������ͷ����ַ��ָ��*/
	} 
	
	stud *search(stud *h,char *user_name)				/*��������ĺ�����hָ��������ı�ͷָ�룬user_name��Ҫ���ҵ��˶�����*/
	{
		stud *s;										/*��ǰָ�룬ָ������Ҫ���ҵ��������бȽϵĽ��*/
		char *y;										/*��������������������ָ��*/
		s = h->link;
		while(s != NULL)
		{
			y = s->name;
			if(strcmp(user_name,y)==0)					/*�������������������Ҫ���ҵ��������бȽϣ�����ͬ�򷵻�0*/
			{
				return s;								/*������Ҫ���ҵĽ���ַ*/
			}	
			else
				s = s->link;							/*����ǰ�ڵ����������ϣ���׼��������һ��������*/
		} 
		return NULL;
	}
	
	stud *search2(stud *h,char *user_name)				/*��������������ǰһ�����Һ���������������ڷ��ص���ֱ��ǰ������ָ�룬��ǰһ�����Һ������ص��ǵ�ǰ����ָ��*/
	{
		stud *p,*s;										/*pָ��ָ��ǰ����㣬sָ��ָ��ǰ���*/
		char *y;
		
		p = h;
		s = h->link;
		while(s != NULL)
		{
			y = s->name;
			if(strcmp(user_name,y) == 0)
				return p;								/*����ǰ�����*/
			else
			{
				p = s;
				s = s->link;
			}
		}
		return NULL;
	}
	
	void insert(stud *p)								/*�����㺯������ָ��p�����*/
	{
		stud *s;										/*��ǰָ�룬ָ��Ҫ����Ľ��*/
		char user_name[10];								/*Ҫ����������*/
		if((s=malloc(sizeof(stud))) == NULL)			/*�����´洢�ռ䲢���*/ 
		{
			printf("���ܷ����ڴ�ռ�\n");
			exit(1);
		}
		printf("����������\n");
		scanf("%s",user_name);
		strcpy(s->name,user_name);						/*�½��������䣬��p���������s�������У���p���������´���ָ��s����ָ��*/
		s->link = p->link;
		p->link = s;
	}

	int main() 
	{ 
		int number; 									/*���������ı���*/ 
		stud *head,*user_p,*user_s; 					/*head�Ǳ��浥����ı�ͷ����ַ��ָ��*/
		char full_name[10];
		
		number=N; 
		head = creat(number); 							/*�����½��ĵ������ͷ��ַ����head*/ 
		printf("enter the name you want to search:\n");
		scanf("%s",full_name);
		user_p = search(head,full_name);				/*��������,���ص�ǰ���*/
		if(user_p != NULL)
		{
			printf("name have be got\n");
			insert(user_p);								/*�����½��*/
			printf("���������ɹ�\n");
			getchar();									/*����ø��ַ�*/
			getchar();
			user_s = search2(head,"��Ң��");			/*�ٴβ�����������ǰ�����*/
			if(user_s != NULL)
			{
				del(user_p,user_s);						/*ɾ��user_p��㣬�˴���Ϊ������Ң�ɶ�Ӧ�Ľ��*/
				printf("ɾ����������Ң�ɳɹ�\n");
			}
			else
				printf("��������������Ң��\n");
		}	
		else
			printf("������������%s\n",full_name);
		getchar();	
		return 0;
	}
