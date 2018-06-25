记录锁提供的功能： 
        当第一个进程正在访问文件的某个部分时，使用记录锁可以限制其他进程访问同一文件区的行为
        这里需要注意一点，记录锁支持锁定文件的一个区域（当然也可以是整个文件）


fcntl记录锁接口格式：
        int fcntl(int fd,int cmd,struct flock *flockptr)
        @fd         - 要设置记录锁的文件描述符
        @cmd        - 对于记录锁，可以取F_GETLK、F_SETLK、F_SETLKW，含义分别如下：
                      F_GETLK   测试调用进程是否可以获取到flockptr描述的锁，
                                如果该锁无法被当前进程获取，则会通过l_pid字段反馈当前拥有该锁的进程；
                                如果该锁可以被当前进程获取，则l_type字段会被改写为F_UNLCK.
                                显然在自身进程已经持有flockptr描述的锁情况下，再调用该命令没有任何意义
                      F_SETLK   设置由flockptr描述的锁，实际就是上读锁、上写锁、解锁这三种功能。
                                linux中如果调用进程请求该锁，但该锁已被其他进程持有并且不允许共享，则fcntl会立即出错返回，同时errno为 EAGAIN
                      F_SETLKW  这个命令是F_SETLK执行上读锁、上写锁时的阻塞版本，意味着调用进程在无法立即获取该锁时会被置为休眠
        @flockptr   - flock结构如下：
                      struct flock {
                          short l_type;     // 本次记录锁操作类型，可以取F_RDLCK(上读锁)、F_WRLCK(上写锁)、F_UNLCK(解锁)
                          short l_whence;   // 本次记录锁操作基准位置，可以取SEEK_SET、SEEK_CUR、SEEK_END
                          off_t l_start;    // 本次记录锁操作相对基准位置的偏移量
                          off_t l_len;      // 本次记录锁操作字节长度
                          pid_t l_pid;      // 用于存放当前持有该记录锁的进程ID，本字段只在F_GETLK情况下有意义
                      };
                      这里需要注意几点：
                      [1]. 锁可以在文件尾端之后开始，但不能在文件起始位置之前开始
                      [2]. l_len取0则表示锁的范围会自动覆盖最大可能偏移量，这意味着不管向该文件追加写了多少数据，这些数据都处于锁的范围内
                      [3]. 对整个文件加锁的常用方法就是: l_whence取SEEK_SET，l_start取0，l_len取0

fcntl记录锁又分为两种类型，共享读锁和独占性写锁。其使用规则如下：
        [1]. 任意多个进程在文件同一区域上可以有一把共享的读锁，但在文件同一区域上只能有一个进程有一把独占写锁，这个特性基本类似于读写锁
        [2]. 单个进程在文件同一区域进行重复上锁时，不论旧锁是哪种类型，新锁都将会替换旧锁
        [3]. 显然加锁时必须确保文件处于打开状态，并且上读锁需要确保文件为可读模式，上写锁需要确保文件为可写模式

fcntl记录锁在自动继承和释放方面遵循3条规则:
        [1]. 锁跟持有该锁的进程关联，这意味着当一个进程终止时，它所持有的所有fcntl记录锁都会自动释放
             锁跟文件关联，当一个文件描述符关闭时，该进程通过这一描述符引用的文件上持有的所有fcntl记录锁都会被自动释放
        [2]. 由fork产生的子进程不继承父进程持有的锁
        [3]. 在一个描述符没有设置close-on-exec属性前提下，进程执行exec后，新程序可以继承原程序在该描述符引用文件上持有的锁

例子:
/* < main.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define read_lock(fd,offset,whence,len)     log_reg(fd,F_SETLK,F_RDLCK,offset,whence,len)
#define readw_lock(fd,offset,whence,len)    log_reg(fd,F_SETLKW,F_RDLCK,offset,whence,len)
#define write_lock(fd,offset,whence,len)    log_reg(fd,F_SETLK,F_WRLCK,offset,whence,len)
#define writew_lock(fd,offset,whence,len)   log_reg(fd,F_SETLKW,F_WRLCK,offset,whence,len)
#define un_lock(fd,offset,whence,len)       log_reg(fd,F_SETLK,F_UNLCK,offset,whence,len)

#define is_read_lockable(fd,offset,whence,len)  (log_test(fd,F_RDLCK,offset,whence,len) == 0)
#define is_write_lockable(fd,offset,whence,len) (log_test(fd,F_WRLCK,offset,whence,len) == 0)

pid_t log_test(int fd,int type,off_t offset,int whence,off_t len)
{
    struct flock lock;

    lock.l_type = type;
    lock.l_whence = whence;
    lock.l_start = offset;
    lock.l_len = len;
    
    if (fcntl(fd,F_GETLK,&lock) < 0) {
        perror("fcntl");
        exit(1);
    }

    if (lock.l_type == F_UNLCK) 
        return 0;

    return lock.l_pid;
}

int log_reg(int fd,int cmd,int type,off_t offset,int whence,off_t len)
{
    struct flock lock;

    lock.l_type = type;
    lock.l_whence = whence;
    lock.l_start = offset;
    lock.l_len = len;

    return fcntl(fd,cmd,&lock);
}

int main(int argc,char *argv[])
{
    int fd = open("./test",O_RDWR);
    if (fd < 0) {
        perror("open");
        return -1;
    }

#ifdef LOCK
    if (read_lock(fd,0,SEEK_SET,0) < 0) {
        perror("write_lock");
        return -1;
    }

    printf("read lock\n");
    sleep(10);
#ifdef PROCESSEND
    printf("file close\n");
    sleep(10);
#endif
    printf("process terminate\n");
#else
    if (is_read_lockable(fd,0,SEEK_SET,0) == 1) 
        printf("process can get read lock for file 'test'\n");
    else
        printf("process can not get read lock for file 'test'\n");

    if (is_write_lockable(fd,0,SEEK_SET,0) == 1) 
        printf("process can get write lock for file 'test'\n");
    else
        printf("process can not get write lock for file 'test'\n");
#endif

    return 0;
}

$ echo hello > ./test
$ gcc -Wall main.c -o lock -DLOCK
$ gcc -Wall main.c -o testlock
开2个终端，并且都处于代码目录下
终端1执行命令:
$ ./lock
        "read lock"
紧接着终端2执行命令:
$ ./testlock
        "process can get read lock for file 'test'"
        "process can not get write lock for file 'test'"
过大约10s后，终端1上程序lock终止，终端2上再次执行命令：
$ ./testlock
        "process can get read lock for file 'test'"
        "process can get write lock for file 'test'"
以上操作用于印证一条规则： 当一个进程终止时，它所建立的所有fcntl记录锁都会自动释放
----------------------------------------------------------------------

$ gcc -Wall main.c -o lock -DLOCK -DPROCESSEND
终端1执行命令:
$ ./lock
        "read lock"
紧接着终端2执行命令:
$ ./testlock
        "process can get read lock for file 'test'"
        "process can not get write lock for file 'test'"
过大约10s后，终端1上出现如下打印时：
        "file close"
终端2上再次执行命令：
$ ./testlock
        "process can get read lock for file 'test'"
        "process can get write lock for file 'test'"

以上操作用于印证一条规则： 当一个文件描述符关闭时，该进程通过这一描述符引用的文件上持有的所有fcntl记录锁都会被自动释放
