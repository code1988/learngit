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
     *
     * 备注 : 销毁之前必须确保没有线程在该互斥量上阻塞
     *        销毁之前必须确保该互斥量没有被上锁
     *        不需要销毁使用PTHREAD_MUTEX_INITIALIZER初始化的静态分配的互斥量
     */

    互斥量主要包含以下3个属性:
        [1]. 进程共享属性，缺省值是PTHREAD_PROCESS_PRIVATE，意味着只有同一个进程内的多个线程可以访问同一个互斥量
        [2]. 健壮属性，缺省值是PTHREAD_MUTEX_STALLED，意味着持有互斥量的进程终止时不需要采取特别的动作
        [3]. 类型属性，linux中缺省值是PTHREAD_MUTEX_NORMAL，意味着标准互斥量类型，不做任何特殊的错误检查或死锁检测
    备注：本人实际使用中通常不会去使用其非默认属性，所以这里不对互斥量非默认属性的设置加以展开，后面的其他同步方式同理

    /* API  : int pthread_mutex_lock(pthread_mutex_t *mutex)
     * 描述 : 对互斥量进行加锁，如果互斥量已经被其他线程上锁，调用线程将会阻塞直到该互斥量被解锁
     * @mutex   - 指向一个要加锁的互斥量
     *
     * 备注 : 已经对该互斥量上锁的线程，在解锁之前，绝对不允许再次进行加锁，因为直接就会导致死锁
     */

    /* API  : int pthread_mutex_trylock(pthread_mutex_t *mutex)
     * 描述 : 对互斥量进行加锁(非阻塞方式)
     * @mutex   - 指向一个要加锁的互斥量
     *
     * 备注 : 如果互斥量已经被其他线程上锁，直接返回 EBUSY
     */

    /* API  : int pthread_mutex_timedlock(pthread_mutex_t *mutex,const struct timespec *tsptr)
     * 描述 : 对互斥量进行加锁(带超时时间)
     * @mutex   - 指向一个要加锁的互斥量
     * @tsptr   - 指向一个精确到纳秒的日历时间
     *
     * 备注 : 如果阻塞到设置的日历时间还没有获取到该互斥量，直接返回 ETIMEDOUT
     */

    /* API  : int pthread_mutex_unlock(pthread_mutex_t *mutex)
     * 描述 : 对互斥量进行解锁
     * @mutex   - 指向一个要解锁的互斥量
     *
     * 备注 : 如果互斥量已经被其他线程上锁，直接返回 EBUSY
     */

    互斥量是最常用的线程同步方式，在避免死锁的前提下，需要注意把控好锁的粒度：
            如果锁的粒度太粗，就会出现很多线程阻塞等待相同的锁；
            但如果锁的粒度太细，那么过多的锁开销会使系统性能受到影响，而且代码的复杂度大大提升


2. 读写锁(pthread_rwlock_t)
    不同于互斥量只有上锁和未上锁2种状态，读写锁有3种状态：
            [1].读模式下上锁
            [2].写模式下上锁
            [3].未上锁
    读写锁的使用规则：
            [1].当读写锁是写模式下上锁时，所有试图对这个锁上锁(不管是读上锁还是写上锁)的线程都会被阻塞；
            [2].当读写锁是读模式下上锁时，所有试图以读模式对它进行上锁的线程都可以上锁成功，但是试图以写模式对它进行上锁的线程都会被阻塞，直到所有线程释放了读锁
            [2].当读写锁是读模式下上锁时，
