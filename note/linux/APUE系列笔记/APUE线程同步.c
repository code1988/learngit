1. 互斥量(pthread_mutex_t)
    使用互斥量之前，必须先对它进行初始化，初始化互斥量的方式有以下2种：
        [1]. 对于静态分配的互斥量通常在定义时就可以直接设置为常量PTHREAD_MUTEX_INITIALIZER
        [2]. 对于动态分配的互斥量通常调用pthread_mutex_init进行初始化

    /* API  : int pthread_mutex_init(pthread_mutex_t *mutex,pthread_mutexattr_t *attr)
     * 描述 : 初始化互斥量
     * @mutex   - 指向一个要初始化的互斥量
     * @attr    - 用于设置互斥量的属性，NULL表示设置默认属性
     */

    /* API  : int pthread_mutex_destroy(pthread_mutex_t mutex)
     * 描述 : 销毁互斥量
     * @mutex   - 指向一个要销毁的互斥量
     */

    互斥量主要包含以下3个属性:
        [1]. 进程共享属性，缺省值是PTHREAD_PROCESS_PRIVATE，意味着只有同一个进程内的多个线程可以访问同一个互斥量
        [2]. 健壮属性，缺省值是PTHREAD_MUTEX_STALLED，意味着持有互斥量的进程终止时不需要采取特别的动作
        [3]. 类型属性，linux中缺省值是PTHREAD_MUTEX_NORMAL，意味着标准互斥量类型，不做任何特殊的错误检查或死锁检测
    备注：本人实际使用中通常不会去使用其非默认属性，所以这里不对互斥量非默认属性的设置加以展开，后面的其他同步方式同理



