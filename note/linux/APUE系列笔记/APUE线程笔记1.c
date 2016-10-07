1. 多线程的优势：
    [1]. 通过合理的分配任务到多个线程，每个线程在进行事件处理时可以采用同步编程模式，相比异步编程，同步编程简单方便很多
    [2]. 多个进程间进行数据交互必须通过各种IPC机制，而同一个进程下的多个线程间共享同一片地址空间，数据可以直接交互
    [3]. 通过合理的分配任务到多个线程，可以改善整个程序的响应时间和并发性能，即便是运行在单核CPU上，只要不是CPU密集型的程序，都可以通过多线程改善性能

2. POSIX线程模型
    linux正式支持POSIX线程模型大约是从kernel-2.6和glibc-2.3开始，具体的名叫 Native POSIX Threads Library(NPTL)，有关NPTL的深入分析将会另写一份笔记
    编写基于POSIX线程的程序时，需要在代码中加入"#include <pthread.h>"，然后在编译代码时加入"-lpthread"即可
    编写基于POSIX线程的程序时，对于出错的处理通常不要依赖errno，而是应该基于pthread系列函数返回的错误码进行出错处理

3. 线程ID(pthread_t)
    类似于进程ID的定义，线程ID用于在所在进程中唯一标识一个线程，虽然在linux中使用无符号长整型表示pthread_t，但不建议在实际操作中直接当作整数处理
    /* API  : int pthread_equal(pthread_t tid1,pthread_t tid2)
     * 描述 : 用来比较两个线程ID，相等返回非0，不相等返回0
     */

    /* API  : pthread_t pthread_self(void)
     * 描述 : 获取自身的线程ID
     */

4. 线程属性(pthread_attr_t)
    

5. 线程创建
    /* API  : int pthread_create(pthread_t *tidp,const pthread_attr_t *attr,void *(*start_rtn)(void *),void *arg)
     * 描述 : 创建一个线程
     * @tidp    - 存放新创建线程的线程ID
     * @attr    - 用于设置新创建线程的属性，NULL表示默认属性
     * @start_rtn   - 指向新创建的线程入口函数
     * @arg         - 用于作为start_rtn函数的入参
     *
     * 备注；同进程创建使用的fork调用类似，线程创建时并不能确保哪个线程先运行
     */ 
    
