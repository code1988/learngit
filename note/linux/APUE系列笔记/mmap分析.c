mmap(memery-mapped I/O)可以将一个文件或其他对象(比如一段匿名内存)映射到进程的虚拟地址空间。
mmap技术的用途主要有以下两个方面：
    [1]. 将一个文件映射到进程的虚拟地址空间后，进程就可以直接访问这段虚拟地址来进行文件的I/O操作，而不再需要使用read、write等系统调用
    [2]. 多个进程可以通过映射同一个文件实现共享内存，来作为进程间的一种IPC方式

mmap接口格式：
    void *mmap(void *addr,size_t len,int prot,int flag,int fd,off_t off);
    @addr   - 用于指定映射区域的起始地址。通常设置为0,表示由系统为该映射区选择一个合适的起始地址
              需要注意的是，addr的值最好确保是内存页长的整数倍
    @len    - 映射区域的长度
    @prot   - 映射区域的访问权限，支持的权限类型如下:
                    PROT_NONE   映射区不可访问
                    PROT_READ   映射区可读
                    PROT_WRITE  映射区可写
                    PROT_EXEC   映射区可执行
              需要注意的是，这里设置的访问权限必须确保是文件open时设置的访问权限的子集
    @flag   - 用于设置映射区域的属性，Linux中支持的主要属性类型如下：
                    MAP_SHARED      共享进程的本次映射操作.
                                    这意味着后续本进程对映射区域的任何修改都对其他映射了相同区域的进程可见，并且这些修改会同步到原文件中.
                                    本标志和MAP_PRIVATE标志中必须指定一个
                    MAP_PRIVATE     创建一个使用COW机制的私有映射.
                                    这意味着后续本进程对映射区域的任何修改对其他进程不进程可见，并且这些修改不会同步到原文件中.
                                    本标志和MAP_SHARED标志中必须指定一个
                    MAP_ANONYMOUS   映射一段匿名内存,映射成功后这段内存会用0初始化.
                                    使用本标志时，fd参数会被忽略(但最好设置为-1)，off参数必须设置为0
                    MAP_FIXED       强制使用addr作为实际的映射区域起始地址，不建议使用该标志
    @fd     - 要被映射文件的描述符(显然只有在映射对象是文件的情况下有意义).
              文件被映射前，必须先打开该文件，映射完成后该文件即使立即关闭也不会解除映射
    @off    - 要映射字节在文件中的起始偏移量(映射对象不是文件时off必须设置为0)
              需要注意的是，off的值最好确保是内存页长的整数倍

mmap映射文件进行I/O操作时，没有对原文件进行追加的能力，只能在原文件当前长度范围内操作，也就是说，mmap只有在文件被映射部分中的修改才能最终写入文件.
mmap创建的映射区域可以通过fork继承，这是因为fork得到的子进程会继承父进程的虚拟地址空间，显然其中就包含了映射区域.
mmap创建的映射区域不能通过exec继承，这是因为exec会使用新程序来覆盖原程序的地址空间.

对于设置为MAP_SHARED对象为文件的映射区域，修改并不会立即回写到原文件中，何时回写脏页由内核决定，也可以手动回写，接口如下:
    int msync(void *addr,size_t len,int flags);
    @addr   - 需要回写的映射区域地址
    @len    - 需要回写的映射区域长度
    @flags  - 用于对本次回写操作进行控制，可控制的选项如下：
                    MS_ASYNC    简单安排要回写的页
                    MS_SYNC     等待写操作完成后返回
                    以上两个标志必选其一。

解除映射的方法有以下两种：
    [1]. 进程结束时会自动解除映射
    [2]. 调用munmap函数
需要注意的是，解除映射操作并不会使映射区的内容回写到原文件

例子：
/* < main.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(int argc,char *argv[])
{
    void *src;
    int fd;

    int page_size = getpagesize(); 

    fd= open("./testfile",O_RDWR | O_CREAT | O_TRUNC,S_IRUSR | S_IWUSR | S_IROTH);
    if (fd< 0) {
        perror("open");
        exit(1);
    }
    if (ftruncate(fd,5)) {
        perror("ftruncate");
        exit(1);
    }
#if TEST_SIGSEGV1
    src = mmap(0,page_size * 2,PROT_READ,MAP_SHARED,fd,0);
    if (!src) {
        perror("mmap");
        exit(1);
    }

    strcpy(src,"ha");
#else
    src = mmap(0,page_size * 2,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    if (!src) {
        perror("mmap");
        exit(1);
    }

#if TEST_OVERWRITE
    strcpy(src,"hahahaha");
#elif TEST_SIGBUS
    strcpy(src + page_size,"ha");
#elif TEST_SIGSEGV2
    strcpy(src + page_size * 5,"ha");
#endif
#endif

    return 0;
}

$ gcc -Wall main.c -o mmap -DTEST_OVERWRITE
$ ./mmap 
$ cat testfile
        hahah
$ gcc -Wall main.c -o mmap -DTEST_SIGBUS
$ ./mmap
        总线错误
$ gcc -Wall main.c -o mmap -DTEST_SIGSEGV1
$ ./mmap
        段错误
$ gcc -Wall main.c -o mmap -DTEST_SIGSEGV2
$ ./mmap
        段错误

以上结果用于印证以下几点：
    [1]. 如果映射区域被mmap指定为只读，则进程试图修改这片映射区中的数据时，就会触发SIGSEGV信号
    [2]. 进程能够访问的有效地址空间取决于文件被映射部分所占用的内存页数量.
         如果超出这个有效地址空间，但未超出映射空间，进程访问这部分时就会触发SIGBUS信号;
         如果超出映射空间，进程访问这部分时就会触发SIGSEGV信号
    [3]. 只有在文件被映射部分中的修改才能最终写入文件
