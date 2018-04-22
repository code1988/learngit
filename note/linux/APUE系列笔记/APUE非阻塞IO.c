I/O操作(比如open、read、write、send、recv等)默认都工作在阻塞模式，意味着调用时有可能会使进程永远阻塞；
而处于非阻塞模式的I/O操作如果不能完成，则调用立即出错返回。

备注：这里需要注意的一点是，针对磁盘的I/O操作(无锁情况下)不会引发进程永远阻塞，比如read磁盘中的一个空文件时就是直接返回0

有两类方法可以用于设置非阻塞I/O调用：
    [1]. 一类是持久性的，具体又可以细分为以下几种实现方法
            调用open创建普通的文件描述符时指定O_NONBLOCK;
            调用socket创建套接字描述符时指定SOCK_NONBLOCK;
            对于已经创建的描述符，则可调用fcntl为该描述符设置O_NONBLOCK标志;
         将文件/套接字状态修改为非阻塞，可以确保后续针对该文件/套接字的I/O操作都是非阻塞的
    [2]. 另一类就是临时性的，主要就是在套接字调用recv/send系列接口时指定MSG_DONTWAIT标志(并非是posix标准)，这只能确保本次I/O操作是非阻塞的

posix规定，对于非阻塞的描述符如果无数据可读或者暂不支持写入数据，则read/write(recv/send行为类似但略有区别)返回-1，errno被设置为EAGAIN。
但实际的行为还是跟具体平台相关，下面分别是非阻塞read、非阻塞write的例子(Debian 8)

例1.
/* <  gcc main.c */
int main(void)
{
    if(fcntl(STDIN_FILENO,F_SETFL,fcntl(STDIN_FILENO,F_GETFL) | O_NONBLOCK) < 0){
        perror("fcntl");
        return -1;
    }
    char buf[100];
    ssize_t res;
    res = read(STDIN_FILENO,buf,sizeof(buf));
    if(res < 0){
        printf("errno = %d\n",errno);
        return -1;
    }
    printf("len = %d\n",res);
    return 0;
}

$ cat /dev/null | ./a.out
  error = 11
$ ./a.out < /dev/null
  len = 0


例2.
/* <  gcc main.c */
int main(void)
{
    char buf[500000];
    ssize_t ntowrite,nwrite;
    ntowrite = read(STDIN_FILENO,buf,sizeof(buf));
    if(ntowrite < 0){
        perror("read");
        return -1;
    }
    if(fcntl(STDOUT_FILENO,F_SETFL,fcntl(STDOUT_FILENO,F_GETFL) | O_NONBLOCK) < 0){
        perror("fcntl");
        return -1;
    }
    char *ptr = buf;
    while(ntowrite > 0){
        errno = 0;
        nwrite = write(STDOUT_FILENO,ptr,ntowrite);
        fprintf(stderr,"nwrite = %d,errno = %d\n",nwrite,errno);
        if(nwrite > 0){
            ptr += nwrite;
            ntowrite -= nwrite;
        }
    }
    return 0;
}

$ ./a.out < large_file 2>stderr.out
$ cat stderr.out
  nwrite = 253730,errno = 0
  nwrite = -1,errno = 11
  ...
  nwrite = -1,errno = 11
  nwrite = 134806,errno = 0
  nwrite = -1,errno = 11
  ...
  nwrite = -1,errno = 11
  nwrite = 4806,errno = 0


