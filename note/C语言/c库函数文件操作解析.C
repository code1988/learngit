1. c语言进行文件操作，必须首先打开文件，用到fopen函数。
	函数原型：	FILE *fopen(const char *path,const char *mode)
		其中，path为文件路径+文件名，mode为打开方式
		如果文件路径缺省，则表示文件名在本工程目录下
		mode种类有：r/rb	只读，文件必须存在，否则返回NULL
					w/wb	只写，若文件存在，原有内容会被清除，若不存在，则创建
					r+/rb+ 	可读可写，文件必须存在，否则返回NULL
					w+/wb+ 	可读可写，若文件存在，原有内容会被清除，若不存在，则创建
					a		追加，只允许写，若文件存在，则追加内容添加在文件末尾，若不存在，则创建
					a+		追加&可读写，若进行读操作，则从头开始读，若进行写操作，则从末尾开始添加，若不存在，则创建
		以r/w/r+/w+打开成功后返回文件指针，指向文件头； 以a/a+打开成功后，文件指针指向末尾
	//例：测试a/a+分别打开文件后指针位置，以及写入数据后指针位置(测试文件test.c大小：49字节)
	#include "stdio.h"
	#include "stdlib.h"
	
	int main(void)
	{
		int n;
		FILE *fp;
		if((fp=fopen("test.c","a")) == NULL)	//if((fp=fopen("test.c","a+")) == NULL)
		{
			printf("cannot open file\n");
			exit(1);
		}
		n = ftell(fp);
		printf("%d\n",n);
		fputs("test",fp);
		n = ftell(fp);
		printf("%d\n",n);
		fclose(fp);
		return 0;
	}	
	运行结果：
		a方式：	49				a+方式：	0
				53							53
	//结论：以a打开文件后指针指向末尾；以a+打开文件后指针初始指向文件头，进行写操作时自动跳转到末尾.
2. 文件的读写操作主要有3种
	(1). 字符读写
		函数原型：	int fputc(int ch,FILE *fp)		//若写入成功，则返回写入的字符，否则返回-1
					int fgetc(FILE *fp)				//若读取成功，则返回读取的字符，否则返回-1
		//例:从指定文件中读一个字符
		#include "stdio.h"
		#include "stdlib.h"
		int main(void)
		{
			FILE *fp;
			int ch;
			if((fp=fopen("test.c","r")) == NULL)
			{
				printf("cannot open file\n");
				exit(1);	
			}
			while((ch=fgetc(fp)) != EOF)
			{
				putchar(ch);	
			}
			fclose(fp);
			putchar('n');	
			return 0;
		}
		//如果将ch定义成char则不能判断是否结束，因为数据FF会被识别为-1(EOF),而int型则不会(0x000000FF != -1)
	(2). 块读写
		函数原型：	unsigned int fread(void *buffer,unsigned int size,unsigned int n,FILE *fp)	//从fp指向的文件中读取size*n字节的数据放入buffer中，若读取成功返回实际读取的n值，否则返回0
					unsigned int fwrite(const void *buffer,unsigned int size,unsigned int n,FILE *fp)	//将buffer中的size*n字节数据写入fp指向的文件，若写入成功返回实际写入的n值，否则返回0
		//例:
		#include "stdio.h"
		#include "stdlib.h"
		typedef struct node
		{
			char name[20];
			double tall;
			char age;
		}student;
		int main(void)
		{
			FILE *fp;
			int i;
			student s1[3]={{"jialin",160.5,24},{"code",183.5,27},{"love",1314.9,99}};
			student s2[3];
		
			if((fp=fopen("test.c","w+")) == NULL) 	//该模式下可以不需要作判断
			{
				printf("cannot open the file\n");
				exit(1);
			}
			fwrite(s1,sizeof(student),3,fp);		//fwrite(s1,sizeof(s1),1,fp);
			rewind(fp);								//此处必须将文件指针重新指向文件头	
			fread(s2,sizeof(student),3,fp);
			for(i=0;i<3;i++)
			{
				printf("%s %f %d\n",s2[i].name,s2[i].tall,s2[i].age);
			}
			fclose(fp);
			return 0;
		}
	(3). 格式化读写
		函数原型：	int fscanf(FILE *fp,const char format[,argument]...)	//从文件格式化读取数据，若读取成功，返回读取数据个数，否则返回-1
					int fprintf(FILE *fp,const char format[,argument]...)	//从文件格式化写入数据，若写入成功，返回写入的数据个数，否则返回-1
		格式上其实类似于scanf/printf写法，只是在固定形参部分加上文件名，作为输入输出的目标文件