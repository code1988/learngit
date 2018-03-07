I/O多路复用有3种具体实现模型：select、poll和epoll

/* int select(int nfds, fd_set *readfds, fd_set *writefds,fd_set *exceptfds, struct timeval *timeout)
 * @nfds        要监听的最大描述符编号加1，select支持的最大描述符编号为FD_SETSIZE(通常是1024)
 * @readfds     要监听可读事件的描述符集合，NULL表示不监听这类事件
 * @writefds    要监听可写事件的描述符集合，NULL表示不监听这类事件
 * @exceptfds   要监听异常事件的描述符集合，NULL表示不监听这类事件
 * @timeout     超时时间，其中NULL表示永远等待，全0表示立即返回(即使没有事件触发)
 * @返回值      如果因为监听的事件被触发则返回关联的描述符总数，同时会修改传入的描述符集合，那些置1的描述符就是被触发了的；
 *              如果因为超时则返回0，同时所有的描述符集合都会被清0；
 *              如果出错则返回-1。
 *
 * 备注：
 *      [1]. select返回-1时需要额外判断errno，因为信号会中断select调用并也返回-1(同时将errno设置为EINTR)，这种情况下通常需要重新调用select
 *      [2]. fd_set这个数据类型用于表示描述符集合，其实现跟平台相关，所以对描述符集合只能通过系统提供的4个接口进行操作：
 *                  FD_ZERO(fd_set *fdset)          将一个描述符集合的所有位清0
 *                  FD_SET(int fd,fd_set *fdset)    将一个描述符集合中fd对应的位置1
 *                  FD_CLR(int fd,fd_set *fdset)    将一个描述符集合中fd对应的位清0
 *                  FD_ISSET(int fd,fd_set *fdset)  判断一个描述符集合中fd对应的位是否已置1
 *      [3]. 如果同一描述符的读、写事件都被触发，那么在select返回值中会对其记两次数
 *      [4]. 由于select调用过程中可能会修改传入的描述符集合和timeout，所以再次调用select时需要重新设置这几个入参
 *      [5]. select异常事件主要包括网络连接上有带外数据到达等,需要注意的是异常事件不包括到达文件尾端和TCP连接断开,这两种情况下,select会认为关联的描述符可读,然后调用read系列API,返回0
 *
 * select模型的主要缺陷:
 *          select会修改传入的描述符集合和timeout，导致每次调用select都需要重新初始化这些入参,这对于需要反复执行的场景显然不太合理;
 *	        如果某个描述符监听的条件被触发，select仅仅会返回，但并不告诉用户是哪个描述符，导致用户只能逐个遍历已加入监听的描述符;
 *	        支持的描述符数量太少，缺省是 FD_SETSIZE = 1024
 */


/* int poll(struct pollfd *fds, nfds_t nfds, int timeout)
 * @fds     不同于select使用了3个位图结构fd_set来设置要监听的描述符集合,poll改用pollfd结构体数组实现
 * @nfds    指定了fds数组中元素(即pollfd结构体)的个数
 * @timeout 超时时间,单位ms,其中负值表示永远等待,0表示立即返回(即使没有事件触发)
 * @返回值  如果成功则返回有事件或异常上报的描述符总数;
 *          如果超时则返回0；
 *          如果出错则返回-1
 *
 * 备注:
 *      [1]. poll返回-1时的行为和select类似
 *      [2]. pollfd结构体包含了要监听描述符的相关信息,具体如下
 *                  struct pollfd{
 *                      int fd;         // 一个已经打开了的描述符
 *                      short events;   // 该描述符要监听的事件集合,显然由用户设置
 *                      short revents;  // 该描述符监听的事件集合中触发了的事件集合,或者是POLLERR/POLLHUP/POLLNVAL之一,显然是在poll返回时由内核设置
 *                  };
 *      [3]. poll支持的事件标志主要有:
 *                  POLLIN/POLLRDNORM       有普通数据可读(包括到达文件末尾和TCP连接对端正常关闭等)
 *                  POLLPRI                 有紧急数据可读
 *                  POLLOUT/POLLWRNORM      可不阻塞地写普通数据
 *                  POLLRDHUP               通常流式套接字(比如TCP)对端关闭连接时本端会触发该事件
 *                  POLLERR                 (只会出现在返回的revents中)指定描述符上发生异常,通常会同时附加注册时的POLLIN/POLLOUT事件
 *                  POLLNVAL                (只会出现在返回的revents中)指定描述符没有打开
 *                  POLLHUP                 (只会出现在返回的revents中)指定描述符被挂断,linux平台上的典型例子就是关闭pipe的写端,然后poll pipe的读端,返回的通常就是POLLHUP
 *
 * poll模型相对select模型的主要改进:
 *          poll去掉了描述符数量的限制；
 *          poll不会修改传入的pollfd结构体数组,所以在反复执行poll的场景中不需要再重复进行初始化
 *
 * poll模型仍旧存在的主要缺陷:
 *          poll和select一样都需要在返回后遍历描述符来获取已经触发的事件,那么随着监听的描述符数量增加,必然导致性能明显下降(即便某一时刻只有很少部分处于就绪状态)
 */


/* int epoll_create(int size)       // 创建一个epoll实例
 * @size    早期用于通知内核该新建的epoll实例可以监听的描述符数量,linux 2.6.8后,这个参数被忽略,只是为了保持兼容,所以需要传入一个正数
 * @返回值  成功则返回一个指向新建epoll实例的描述符；出错则返回-1
 *
 * int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)   // 在指定epoll实例中对一个描述符进行操作
 * @epfd    对应一个epoll实例(已经创建)的描述符
 * @op      有效的操作有3种:
 *              EPOLL_CTL_ADD   将一个描述符注册到epoll实例中,同时附带注册了该描述符关联的事件
 *              EPOLL_CTL_MOD   修改一个已经注册的描述符关联的事件
 *              EPOLL_CTL_DEL   注销一个已经注册的描述符
 * @fd      需要进行操作的描述符
 * @event   对应该描述符关联的事件(linux 2.6.9后,传入EPOLL_CTL_DEL时对应的event可以为NULL)
 * @返回值  成功则返回0；出错则返回-1
 *
 * int epoll_wait(int epfd,struct epoll_event *events,int maxevents,int timeout)    // 等待epoll上监听的I/O事件触发
 * @epfd        对应一个epoll实例(已经创建)的描述符
 * @events      返回时用于存放触发事件的数组
 * @maxevents   events数组可以存放的事件数量上限
 * @timeout     超时时间,单位ms,其中-1表示永远等待,0表示立即返回(即使没有事件触发)
 * @返回值      如果成功则返回有事件或异常上报的描述符总数;
 *              如果超时则返回0；
 *              如果出错则返回-1
 *
 * 备注:
 *      [1]. epoll模型属于linux平台特有
 *      [2]. epoll_event结构体包含了要监听描述符关联的相关信息,具体如下
 *                  struct epoll_event{
 *                      uint32_t events;        // 在epoll_ctl中用于设置对应描述符要监听的事件集合(显然由用户设置);在epoll_wait中用于返回对应描述符触发的事件集合(显然是由内核设置)
 *                      epoll_data_t data;      // 在epoll_ctl中用于设置对应描述符的自定义内容(显然由用户设置)；在epoll_wait中用于返回对应描述符事先注册的自定义内容(显然是由内核设置)
 *                  };
 *                  typedef union epoll_data{
 *                      void *ptr;
 *                      int fd;
 *                      uint32_t u32;
 *                      uint64_t u64;
 *                  };
 *      [3]. epoll支持的事件标志主要有:
 *                  EPOLLIN         类似POLLIN
 *                  EPOLLPRI        类似POLLPRI
 *                  EPOLLOUT        类似POLLOUT
 *                  EPOLLRDHUP      类似POLLRDHUP
 *                  EPOLLERR        类似POLLERR
 *                  EPOLLHUP        类似POLLHUP
 *                  EPOLLET         设置对应的描述符为边沿触发(缺省是水平触发)
 *                  EPOLLONESHOT    设置对应的描述符只做一次性监听(缺省是持久生效的),意味着触发过一次后就会失效
 *
 * epoll模型相对select模型和poll模型的主要改进:
 *          当某个描述符关联的事件触发时,epoll通过回调机制直接将对应的描述符通知用户,从而去掉了遍历操作,
 *          这种改进使得epoll模型中的I/O效率基本不会随着监听的描述符数量增加而下降
 *                    
 */
