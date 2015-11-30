/*
/**************************************************************************
*/
进程：资源分配的最小单位
线程：程序执行的最小单位
/*
/**************************************************************************
*/
二者区别：
进程有独立的地址空间，一个进程崩溃后，在保护模式下不会对其他进程产生影响。
而线程只是一个进程的不同执行流，一个进程可以有多个线程组成，线程有自己的堆栈和局部变量，但没有单独的地址空间，同属一个进程的所有线程共享该进程拥有的全部资源（各自线程的堆栈和局部变量不共享），一个线程死掉就导致整个进程死掉。
所以多进程的程序要比多线程的程序健壮，但进程间切换时，耗费的资源较大，效率要差一些。
对于一些要求同时进行并且要共享某些变量的并发操作，只能用线程，不能用进程。

使用多线程的优点：	1.和进程相比，线程是一种非常节俭的多任务操作方式（共用地址空间，共享大部分数据，所以花费空间小；线程间切换时间短）
					2.线程间方便的通信机制
					3.提高应用程序响应
					4.使多CPU系统更加有效
					5.改善程序结构

/*********************************	 进程相关函数用法	*****************************************/
1. 创建进程的函数声明：	pid_t fork(void)	//头文件（unist.h）
											//返回值取决于在哪个进程中来检测该值,如果在新创建进程中，为0；如果在父进程中，为新创建的进程ID；如果创建失败，为负值
	例：
		#include <stdio.h>
		#include <unistd.h>
		int main ()
		{
		    printf("i'm main process,my id is %d\n",getpid());					//返回当前进程ID
		    pid_t id = fork();													//变量id在父进程中为新创建的子进程ID，在子进程中为0
		    if (id<0) {
		        printf("error\n");
		    }else if (id==0) {
		        printf("hi, i'm in new process, my id is %d \n", getpid());		//返回子进程ID
		    }else {
		        printf("hi, i'm in old process, the return value is %d\n", id);	//返回子进程ID
		    }
		    return 0;
		}
		程序3个输出值：	
						20972
						20973
						20973
	
	例：
		#include <stdio.h>
		#include <unistd.h>
		int main ()
		{
			printf("app start...");			//调用printf时，字符串被写入stdout缓冲区（还没有刷到屏幕上），然后fork，子进程复制了父进程的缓冲区，当他们遇到return语句时，缓冲区强制刷新，就分别将字符串刷到了屏幕上
											//在字符串后面加上“\n”时，就可避免这种现象，因为stdout是按行缓冲的
			fork();
			return 0;
		}
		输出为：
				app start...app start...
2. 创建进程后运行新程序
	创建新进程后，新进程会接着运行父进程中的代码，如果要想新进程运行另外的代码，也就相当于启动一个新程序，需要在新进程中使用 exce 函数族
	exec函数族作用都一样，都是用一个可执行文件覆盖进程现有的代码，并转到该可执行文件起始位置开始执行。
	exec函数族原型如下：
	int execl(const char *path, const char *arg0, ... /*, (char *)0 */);
	int execlp(const char *file, const char *arg0, ... /*, (char *)0 */);
	int execv(const char *path, char *const argv[]);
	int execvp(const char *file, char *const argv[]);
	以execl函数为例，第一个参数path是可执行文件路径（绝对路径），从第二个arg0参数开始以及后面所有的是传递给可执行文件的命令行参数。
	注意参数arg0是可执行文件本身（这种用法类似于main函数参数），最后那个注释是提醒最后一个参数应该穿空字符串。
	如果函数运行成功，则不会有返回值；运行错误则返回-1，具体的错误号会被设置在errno（errno是一个全局变量，用于程序设置错误号）
	例：
		#include <stdio.h>
		#include <unistd.h>
		int main ()
		{
			pid_t id=fork();
			
			if(id == 0)
				ececl("/bin/ls","/bin/ls","-l",NULL);	//子进程中用execl加载ls程序并运行之，其中ls程序覆盖了原本从父进程继承过来的代码，所以子进程不会再执行下面那行printf函数
			printf("app end\n");
			return 0;
		}
		输出为：
				app end									//父进程输出
				（在当前目录下执行ls命令）				//子进程输出
	
	execlp与execl区别在于execlp会去系统环境变量查找file所指的程序的位置，如果能找到，file就可以不是绝对路径了，如	execlp("ls","ls","-l",NULL);
	execv/ececvp与前两个类似，后缀由l变成了v，表示参数不再由参数列表传递，而改用参数数组argv[],当然最后一个元素也必须是NULL
3. 进程控制
	退出进程:	void exit(int status) / void _exit(int status)	//status表示进程将以何种状态退出，常用的是EXIT_SUCCESS(也就是0)表示成功状态退出；EXIT_FAILURE(也就是1)表示失败状态退出
				void abort()									//非正常退出，会产生一个SIGABORT信号
	等待子进程结束：waitpid /wait	//父进程只有等待子进程结束后才能往下继续运行
		pid_t waitpid(pid_t pid,int *stat_loc,int options);
		参数pid：
			>0  表示父进程所需要等待的子进程进程号
			=0  表示任意group id与父进程相同的子进程
			=-1 表示任意子进程，此时waitpid与wait相同
			<-1 取其绝对值作为需要等待的子进程进程号
		参数stat_loc:
			表示进程退出时进程状态的存储位置
		参数options：
			控制函数是否立即返回，有3个值：0，WNOHAND(即1)，WUNTRACED(即2)
	waitpid与wait关系：wait(&status)  等于  waitpid(-1,&status,0)		//实际常用wait(NULL)

4. 进程通信
	无名管道（pipe）：int pipe(int fd[2])					//这个函数将创建无名管道，并将管道的读描述字端包含在fd[0]中，写描述字端包含在fd[1]中
	例：
		#include <stdio.h>
		#include <unistd.h>
		#include <string.h>
		#include <stlib.h>
		
		int main()											//本例通过无名管道完成了2个进程之间的通信：在父进程中关闭管道的读端，往写端写入字符串；在子进程中关闭管道的写端，从读端读出字符串
		{
			pid_t pid;
			int pipe_fd[2];
			char buf[1024];
			const char data[]="hello world!";
			
			memset(buf,0,1024);
			pipe(pipe_fd[2]);								//切记，先创建管道
			pid = fork();									//在创建新进程！！！		
			if(pid == 0)
			{
				close(pipe_fd[1]);	
				read(pipe_fd[0],buf,1024);
				printf("%s\n",buf);
				close(pipe_fd[0]);
				exit(0);
			}
			
			close(pipe_fd[0]);
			write(pipe_fd[1],data,strlen(data));
			printf("write '%s'\n",data);
			close(pipe_fd[1]);
			wait(NULL);										//可用waitpid(pid,NULL,0)代替
			printf("app end \n");
			
			return 0;				
		}
	有名管道（FIFO）：int mkfifo(char * path,mode_t mode)	//path即要创建的管道文件存放路径，mode即文件权限
	感觉FIFO就是这个进程创建一个文件，并往文件里写入数据；然后另一个进程再打开这个文件，并从文件里取出数据。
	FIFO跟普通文件的区别仅在于数据读取后，文件里就不存在了，而普通文件读取数据后，文件里数据还在。