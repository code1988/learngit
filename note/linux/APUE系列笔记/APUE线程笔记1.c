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
    线程属性对象pthread_attr_t中包含了多个属性，但pthread_attr_t的内部结构细节被隐藏在了NPTL中，应用程序通过NPTL提供的一组API来管理线程属性
    线程属性包括以下这些：
                线程的分离状态                  
                线程的栈属性
                线程的调度策略                  
                线程的调度参数
                线程的继承性
                线程的作用域
                线程的栈末尾警戒缓冲区大小
    线程属性对象在使用前必须经过初始化，相对应的，使用完毕后也必须执行销毁
    /* API  : int pthread_attr_init(pthread_attr_t *attr)
     * 描述 : 初始化一个线程属性对象attr
     * @attr    - 指向要被初始化的线程属性对象
     *
     * 备注 : 初始化之后，attr中存放的就是线程属性的默认值;
     *        不允许对已经初始化的线程属性对象重复进行初始化
     */

    /* API  : int pthread_attr_destroy(pthread_attr_t *attr) 
     * 描述 : 销毁一个线程属性对象attr
     * @attr    - 指向要被销毁的线程属性对象
     *
     * 备注 : 销毁线程属性对象并不会对已经创建的线程(创建时使用了该对象)有任何影响;
     *        线程属性对象在被销毁之后，又可以被重新初始化
     */
    
    线程的分离状态属性决定了线程终止时的行为：
            非分离的线程在结束时不会自行释放占用的系统资源，而是要等到有线程对该线程调用了pthread_join时才会完全释放;
            已经分离的线程在结束时会立即释放自身占用的系统资源，并且其他线程也不允许再调用pthread_join来等待它的终止状态
    /* API  : int pthread_attr_getdetachstate(const pthread_attr_t *attr,int *detachstate)
     * 描述 : 获取线程属性对象中的分离状态属性
     * @attr        - 指向线程属性对象
     * @detachstate - 用于存放线程的分离状态属性:
     *                                      PTHREAD_CREATE_DETACHED - 以分离状态启动线程 
     *                                      PTHREAD_CREATE_JOINABLE - 以非分离状态启动线程(默认属性)
     */

    /* API  : int pthread_attr_setdetachstate(pthread_attr_t *attr,int *detachstate)
     * 描述 : 修改线程属性对象中的分离状态属性
     * @attr        - 指向线程属性对象
     * @detachstate - 用于设置线程的分离状态属性
     *
     * 备注 : 如果创建线程前就确定不需要关心该线程的终止状态，就调用本函数提前将线程的分离状态属性设置为PTHREAD_CREATE_DETACHED
     */

    /* API  : int pthread_detach(pthread_t tid)
     * 描述 : 分离指定线程
     * @tid     - 要被分离的线程ID
     *
     * 备注 : 如果对已经存在的某个线程的终止状态不再关心，就调用本函数将其分离
     */

    线程的栈属性又包括了栈地址、栈大小以及栈末尾的警戒缓冲区大小
    /* API  : int pthread_attr_getstack(const pthread_attr_t *attr,void **stackaddr,size_t *stacksize)
     * 描述 : 获取线程属性对象中的自定义的栈地址和栈大小属性
     * @attr        - 指向线程属性对象
     * @stackaddr   - 用于存放自定义的线程的栈地址
     * @stacksize   - 用于存放自定义的线程的栈大小
     *
     * 备注 : (glibc2.19 + kernel3.16.36环境) 本函数只能获取通过pthread_attr_setstack设置的自定义的线程栈地址和栈大小属性,
     *        如果没有设置过自定义值，本函数返回空值。
     *        本函数的这个特性APUE并未阐明，网上的资料大都是相互抄袭，人云亦云
     */
    
    /* API  : int pthread_attr_setstack(pthread_attr_t *attr,void *stackaddr,size_t stacksize)
     * 描述 : 自定义线程属性对象中的栈地址和栈大小属性
     * @attr        - 指向线程属性对象
     * @stackaddr   - 用于设置自定义的线程栈地址
     * @stacksize   - 用于设置自定义的线程栈大小
     *
     */
    
5. 线程创建
    /* API  : int pthread_create(pthread_t *tidp,const pthread_attr_t *attr,void *(*start_rtn)(void *),void *arg)
     * 描述 : 创建一个线程
     * @tidp    - 存放新创建线程的线程ID
     * @attr    - 用于设置新创建线程的属性，NULL表示默认属性
     * @start_rtn   - 指向新创建的线程入口函数
     * @arg         - 用于作为start_rtn函数的入参
     *
     * 备注 : 同进程创建使用的fork调用类似，线程创建时并不能确保哪个线程先运行
     */ 
    
