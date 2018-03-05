I/O多路复用有3种具体实现模型：select、poll和epoll

/* int select(int nfds, fd_set *readfds, fd_set *writefds,fd_set *exceptfds, struct timeval *timeout)
 * @nfds        要监听的最大描述符编号加1，select支持的最大描述符编号为FD_SETSIZE(通常是1024)
 * @readfds     要监听可读事件的描述符集合，NULL表示不监听这类事件
 * @writefds    要监听可写事件的描述符集合，NULL表示不监听这类事件
 * @exceptfds   要监听异常事件的描述符集合，NULL表示不监听这类事件
 * @timeout     超时时间，其中NULL表示永远等待，全0表示不等待
 * @返回值      如果因为关心的事件被触发则返回关联的描述符总数，同时会修改传入的描述符集合，那些置1的描述符就是被触发了的；
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
 *      [5]. 异常事件主要包括网络连接上有带外数据到达等,需要注意的是异常事件不包括到达文件尾端和TCP连接断开,这两种情况下,select会认为关联的描述符可读,然后调用read系列API,返回0
 *
 * select模型的主要缺陷:
 *          select会修改传入的描述符集合和timeout，导致每次调用select都需要重新初始化这些入参,这对于需要反复执行的场景显然不太合理;
 *	        如果某个描述符监听的条件被触发，select仅仅会返回，但并不告诉用户是哪个描述符，导致用户只能逐个遍历所有已加入监听的描述符;
 *	        支持的描述符数量太少，缺省是 FD_SETSIZE = 1024
 */


