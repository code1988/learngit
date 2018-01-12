1. 互斥量(pthread_mutex_t)
    使用互斥量之前，必须先对它进行初始化，初始化互斥量的方式有以下2种：
        [1]. 对于静态分配的互斥量通常在定义时就可以直接设置为常量PTHREAD_MUTEX_INITIALIZER
        [2]. 对于动态分配的互斥量通常调用pthread_mutex_init进行初始化

    /* API  : int pthread_mutex_init(pthread_mutex_t *mutex,pthread_mutexattr_t *attr)
     * 描述 : 初始化互斥量
     * @mutex   - 指向一个要初始化的互斥量
     * @attr    - 用于设置互斥量的属性，NULL表示设置默认属性
     */

    /* API  : int pthread_mutex_destroy(pthread_mutex_t *mutex)
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
    类似于互斥量，使用读写锁之前，必须先对它进行初始化，初始化读写锁的方式有以下2种：
        [1]. 对于静态分配的读写锁通常在定义时就可以直接设置为常量PTHREAD_RWLOCK_INITIALIZER
        [2]. 对于动态分配的读写锁通常调用pthread_rwlock_init进行初始化

    /* API  : int pthread_rwlock_init(pthread_rwlock_t *rwlock,pthread_rwlockattr_t *attr)
     * 描述 : 初始化读写锁
     * @rwlock  - 指向一个要初始化的读写锁
     * @attr    - 用于设置读写锁的属性，NULL表示设置默认属性
     */

    /* API  : int pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
     * 描述 : 销毁读写锁
     * @rwlock  - 指向一个要销毁的读写锁
     *
     * 备注 : 销毁之前必须确保没有线程在该读写锁上阻塞
     *        销毁之前必须确保该读写锁没有被上锁
     *        不需要销毁使用PTHREAD_RWLOCK_INITIALIZER初始化的静态分配的读写锁
     */

    读写锁POSIX标准中只定义了1个进程共享属性(类似于互斥量的进程共享属性)

    /* API  : int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
     * 描述 : 读模式下进行加锁
     * @rwlock  - 指向一个要加读锁的读写锁
     */

    /* API  : int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
     * 描述 : 写模式下进行加锁
     * @rwlock  - 指向一个要加写锁的读写锁
     */

    /* API  : int pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
     * 描述 : 对读写锁进行解锁(读锁和写锁通用)
     * @rwlock  - 指向一个要解锁的读写锁
     */

    类似于互斥量，读写锁也有非阻塞方式和带超时时间的API，格式和用法基本都类似，这里不再展开分析
    显然读写锁非常适合于对数据结构读的次数远大于写的场景

3. 条件变量(pthread_cond_init)
    条件变量需要配合互斥量一起使用，线程在改变条件变量之前必须先锁住互斥量。
    类似于互斥量，使用条件变量之前，必须先对它进行初始化，初始化条件变量的方式有以下2种：
        [1]. 对于静态分配的条件变量通常在定义时就可以直接设置为常量PTHREAD_COND_INITIALIZER
        [2]. 对于动态分配的条件变量通常调用pthread_cond_init进行初始化
    
    /* API  : int pthread_cond_init(pthread_cond_t *cond,pthread_condattr_t *attr)
     * 描述 : 初始化条件变量
     * @cond    - 指向一个要初始化的条件变量
     * @attr    - 用于设置条件变量的属性，NULL表示设置默认属性
     */

    /* API  : int pthread_cond_destroy(pthread_cond_t *cond)
     * 描述 : 销毁条件变量
     * @cond    - 指向一个要销毁的条件变量
     *
     * 备注 : 销毁之前必须确保没有线程在该条件变量上阻塞
     *        不需要销毁使用PTHREAD_COND_INITIALIZER初始化的静态分配的条件变量
     */

    条件变量主要包含以下2个属性：
        [1]. 进程共享属性(类似于互斥量)
        [2]. 时钟属性: 用于控制pthread_cond_timedwait传入的超时参数采用的是哪个时钟，缺省值是CLOCK_REALTIME，也就是日历时间

    /* API  : int pthread_cond_wait(pthread_cond_t *cond,pthread_mutex_t *mutex)
     * 描述 : 等待条件变量为真
     * @cond    - 指向一个要等待的条件变量
     * @mutex   - 指向一个用来对条件变量进行保护的互斥量
     *
     * 备注 : 调用本函数前需要先锁住该互斥量，然后本函数在执行的过程中会自动把调用线程放到条件变量的等待队列中，并且解锁互斥量
     *        当条件变量为真时，本函数在返回前会重新锁住互斥量
     *        在本函数返回之后，需要重新检查条件(也就是条件变量作用的具体数据结构)是否可用(因为可能存在一些极端情况，比如多个线程同时在等待该条件变量，先等到的线程已经改掉了条件)
     */

    /* API  : int pthread_cond_timedwait(pthread_cond_t *cond,pthread_mutex_t *mutex,const struct timespec *tsptr)
     * 描述 : 等待条件变量为真(带超时时间)
     * @cond    - 指向一个要等待的条件变量
     * @mutex   - 指向一个用来对条件变量进行保护的互斥量
     * @tsptr   - 指向一个精确到纳秒的日历时间
     *
     * 备注 : 如果阻塞到设置的日历时间该条件变量还没有为真，直接返回 ETIMEDOUT
     */

    /* API  : int pthread_cond_signal(pthread_cond_t *cond)
     * 描述 : 通知至少一个等待该条件的线程条件已经满足(即条件变量作用的具体数据结构已经可用)
     * @cond    - 指向一个要通知的条件变量
     *
     * 备注 : 显然，本函数应该在改变了条件状态后调用;
     *        说是至少通知一个，实际可能跟系统实现有关
     */

    /* API  : int pthread_cond_broadcast(pthread_cond_t *cond)
     * 描述 : 通知所有等待该条件的线程条件已经满足(即条件变量作用的具体数据结构已经可用)
     * @cond    - 指向一个要广播通知的条件变量
     */

4. 自旋锁(pthread_spin_t)
    和互斥量的区别:
            互斥量采用sleep-waiting机制，而自旋锁采用busy-waiting机制。
    显然，因为自旋锁不会引起调用进程睡眠，所以通常使用在临界区很小，并且不希望在线程调度上花费太多时间的SMP系统。
    实际使用中，用户态自旋锁基本很少用，主要是因为互斥量的实现已经非常高效。即便使用也通常是作为实现其他类型锁的底层原语。

    使用自旋锁之前，必须先对它进行初始化，但自旋锁只能通过动态方式初始化，并且自旋锁只有1个属性，就是进程共享属性。

    /* API  : int pthread_spin_init(pthread_spin_t *lock,int pshared)
     * 描述 : 初始自旋锁
     * @lock    - 指向一个要初始化的自旋锁
     * @pshared - 用于设置自旋锁是否支持进程共享属性
     */

    /* API  : int pthread_spin_destroy(pthread_spin_t *lock)
     * 描述 : 销毁自旋锁
     * @lock    - 指向一个要销毁的自旋锁
     */

    /* API  : int pthread_spin_lock(pthread_spin_t *lock)
     * 描述 : 对自旋锁进行加锁，如果该自旋锁已经被其他线程上锁，调用线程将会忙等直到得到该锁为止
     * @lock    - 指向一个要加锁的自旋锁
     *
     * 备注 : 不允许调用线程对自旋锁进行重复加锁，否则其结果是未定义的
     */

    /* API  : int pthread_spin_trylock(pthread_spin_t *lock)
     * 描述 : 对自旋锁进行加锁(非阻塞方式)
     * @lock    - 指向一个要加锁的自旋锁
     *
     * 备注 : 如果该自旋锁已经被其他线程上锁，不自旋，直接返回 EBUSY
     */

    /* API  : int pthread_spin_unlock(pthread_spin_t *lock)
     * 描述 : 对自旋锁进行解锁
     * @lock    - 指向一个要解锁的自旋锁
     *
     * 备注 : 不允许对没有加锁的自旋锁进行解锁，否则其结果是未定义的
     */

    不应该在持有自旋锁的情况下调用可能会进入休眠状态的函数。

5. 屏障(pthread_barrier_t)
    不同于以上那些同步机制，屏障通常用于同步多个线程之间的并行工作，其工作机制如下：
            每个合作线程都会阻塞在屏障设置的等待点，直到所有的合作线程都到达各自所在的等待点，然后才都会从该点继续往后执行

    使用屏障之前，必须先对它进行初始化，但屏障只能通过动态方式初始化。

    /* API  : int pthread_barrier_init(pthread_barrier_t *barrier,const pthread_barrierattr_t *attr,unsigned int count)
     * 描述 : 初始化屏障
     * @barrier     - 指向一个要初始化的屏障
     * @attr        - 用于设置屏障的属性，NULL表示设置默认属性
     * @count       - 屏障计数值，用于指定该屏障管理的合作线程数量
     */

    /* API  : int pthread_barrier_destroy(pthread_barrier_t *barrier)
     * 描述 : 销毁屏障
     * @barrier     - 指向一个要销毁的屏障
     */

    目前屏障只定义了1个进程共享属性(类似于互斥量的进程共享属性)

    /* API  : int pthread_barrier_wait(pthread_barrier_t *barrier)
     * 描述 : 等待其他合作线程都到达各自的屏障点，调用线程在所有合作线程没有全部到达屏障点之前会进入休眠状态
     *        如果该线程是最后一个到达屏障点的合作线程，则所有的合作线程都被唤醒
     * @barrier     - 指向一个要进行等待的屏障
     *
     * 备注 : 所有合作线程中只会有1个线程调用本函数会返回PTHREAD_BARRIER_SERIAL_THREAD(其余线程返回值都为0)，
     *        所以该线程可以作为主合作线程，用于对其他线程完成的工作结果进行相关汇总处理.
     *        当然，实际使用中也可以手动指定一个特定合作线程作为主合作线程
     */

    当阻塞在屏障点的所有合作线程被唤醒后，屏障就可以被重用。

