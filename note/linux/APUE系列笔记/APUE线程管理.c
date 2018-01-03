1. 线程ID(pthread_t)
    类似于进程ID的定义，线程ID用于在所在进程中唯一标识一个线程，虽然在linux中使用无符号长整型表示pthread_t，但不建议在实际操作中直接当作整数处理
    /* API  : int pthread_equal(pthread_t tid1,pthread_t tid2)
     * 描述 : 用来比较两个线程ID，相等返回非0，不相等返回0
     */

    /* API  : pthread_t pthread_self(void)
     * 描述 : 获取自身的线程ID
     */

2. 线程创建
    /* API  : int pthread_create(pthread_t *tidp,const pthread_attr_t *attr,void *(*start_rtn)(void *),void *arg)
     * 描述 : 创建一个线程
     * @tidp    - 存放新创建线程的线程ID
     * @attr    - 用于设置新创建线程的属性，NULL表示默认属性
     * @start_rtn   - 指向新创建的线程入口函数
     * @arg         - 用于作为start_rtn函数的入参
     *
     * 备注 : 同进程创建使用的fork调用类似，线程创建时并不能确保哪个线程先运行
     */ 
    
3. 线程终止
    在不终止整个进程的前提下，单个线程可以通过3种方式退出(不包括程序开始时的控制线程)：
            [1]. 线程从其入口函数中返回，返回值就是线程的退出码
            [2]. 线程运行过程中调用pthread_exit
            [3]. 线程可以被同一进程中的其他线程取消
    另外，类似于进程的一点是，线程也可以安排退出时需要调用的函数，这些线程清理函数会在以下3种情况下被调用：
            [1]. 调用pthread_eixt时
            [2]. 响应取消请求时
            [3]. 用非0参数调用pthread_cleanup_pop时
    显然，线程从其入口函数中通过"return"返回时，线程清理函数并不会被触发
    /* API  : int pthread_exit(void *retval)
     * 描述 : 退出本线程
     * @retval  - 用于设置线程的退出码
     *
     * 备注 : void *类型的retval类似于pthread_create中的arg，既可以传递一个简单的整型值，也可以传递一个复杂的结构体
     *        但需要注意的是，retval变量不应该在线程栈上分配，因为线程结束后对应的线程栈将被撤销，甚至已经被其他数据填充
     */

    /* API  : int pthread_cancel(pthread_t tid)
     * 描述 : 请求取消同一进程中的其他指定线程
     * @tid     - 用于设置要取消的线程ID
     *
     * 备注 : 本函数只是发送一个取消的请求给指定的线程，不管结果如何都直接返回
     *        收到该请求的线程，其缺省行为类似调用pthread_eixt((void *)-1)
     */
    
    /* API  : int pthread_cleanup_push(void (*rtn)(void *),void *arg)
     * 描述 : 压入一个线程清理函数
     * @rtn     - 指向要注册的线程清理函数
     * @arg     - 将会传递给该线程清理函数的入参
     */

    /* API  : int pthread_cleanup_pop(int execute)
     * 描述 : 弹出一个线程清理函数，同时根据execute决定是否执行
     * @execute - 标志位,0 - 不执行，1 - 执行
     *
     * 备注 : 清理函数的压入和弹出遵循LIFO(先进后出)原则
     *        这两个函数必须成对使用
     */

    非分离线程的退出码可以被其他线程获取到
    /* API  : int pthread_join(pthread_t tid,void **retval)
     * 描述 : 等待指定的线程结束
     * @tid     - 等待结束的线程ID
     * @retval  - 用于存放指定线程的退出码，如果不关心退出码，就设为NULL
     *
     * 备注 : 目标线程必须是非分离属性的
     */

