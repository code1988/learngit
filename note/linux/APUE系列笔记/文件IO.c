1. int fcntl(int fd,int cmd,...) 
    fcntl函数是利用可变参数实现的，可变参数的类型和格式取决于前面的cmd参数
    fcntl函数可以通过cmd参数实现多种功能:
        F_DUPFD         : 复制文件描述符fd，新描述符取>=arg的最小可用值，并作为返回值返回，复制成功后和源fd共享同一个文件表.
                          该功能类似于dup()和dup2()，区别在于 -- 
                            dup返回的新描述符就是取最小可用值而没有arg限制
                            dup2的新描述符已经通过入参arg指定，即便arg已经被占用并打开
        F_GETFD/F_SETFD : 获取/设置文件描述符标记，目前只定义了一个文件描述符标志FD_CLOEXEC
        F_GETFL/F_SETFL : 获取/设置文件状态标记，注意跟上面的文件描述符区别，这里的状态标记指的就是open时设置的flag标记.
                          需要注意的是，F_SETFL只允许设置文件状态标记中的O_APPEND、O_ASYNC、O_NONBLOCK、O_DIRECT、O_NOATIME
        F_GETLK/F_SETLK/F_SETLKW    : 实现记录锁功能(详见《记录锁.c》)

2. int open(const char *pathname,int flags) / int open(const char *pathname,int flags,mode_t mode)
    open函数有以上2种调用格式，第三个参数仅当创建新文件时（即使用了O_CREAT）才使用
    pathname: 待打开/创建文件的POSIX路径名
    flag    : 指定文件的打开/创建模式，
              O_RDONLY/O_WRONLY/O_RDWR，这三个中必须且只能选一个，
              然后以下标志是可以"|"的
                    O_CREAT     如果指定文件不存在，则创建
                    O_EXCL      如果指定文件已经存在，则返回-1
                    O_NONBLOCK  以非阻塞方式打开文件
                    O_APPEND    每次写入时都追加到文件尾端。
                                设置了该标志的文件，"在每次调用write时，进程首先都会将所属的该文件表项中的当前偏移量自动设置到该文件的尾端"这种操作会合并为原子操作
                                另外，需要注意的一点是，跟fopen库函数不同，每个open后的文件，其"当前文件偏移量"都为0，跟O_APPEND标志无关，而fopen可以通过传入"+"符号将偏移量指针直接初始化到末尾
                    O_DIRECT    无缓冲的输入/输出，即不通过kernel中的page cache，该标志会对写盘性能有很大影响
                                一个危险是，若一个进程以O_DIRECT标志打开文件，而另一进程以普通方式（使用了高速缓冲）打开同一文件，
                                则由O_DIRECT方式读写的数据和高速缓冲中内容之间存在不一致
                                一个弊端是，可能会大大降低性能，因为内核对高速缓冲做了大量优化
                                （除了数据库系统，其他应用场合不推荐使用,the whole notion of "direct io" is totally braindamaged!! --by linus）
                                使用direct io需要遵循的原则是：
                                    1）应用层用于传递数据的缓冲区，其内存边界必须对其为kernel页大小的整数倍（所以不能采用malloc分配，改用posic_memalign）
                                    2）数据传输的开始点，也必须是kernel页大小的整数倍
                                    3）待传递数据的长度，也必须是kernel页大小的整数倍

                    O_DSYNC     使每次write文件时等待数据整体写入底层硬件后返回(数据整体写入完成的标志是文件长度更新),该标志的作用类似与每次普通write后调用fdadasync(int fd)
                    O_SYNC      使每次write文件时等待文件整体写入底层硬件后返回(文件整体写入完成的标志是文件修改时间更新)，该标志的作用类似与每次普通write后调用fsync(int fd)
                                Linux 2.6.33前，只有O_SYNC标志，并且实际执行了O_DSYNC的功能
                                Linux 2.6.33后，O_SYNC标志功能被正确执行，同时引入了O_DSYNC标志用于执行老O_SYNC标志的功能
    mode    : 可以指定文件的访问权限位和设置-用户-ID位（不能在这里设置文件类型!!），创建新文件时才会生效
                    S_IRUSR     文件所有者具有可读权限
                    S_IWUSR     文件所有者具有可写权限
                    S_IXUSR     文件所有者具有可执行权限
                    S_IRWXU     文件所有者具有可读、可写、可执行权限
                    S_IRGRP/S_IWGRP/S_IXGRP/S_IRWXG     这4个是文件所属用户组具有的权限，同上
                    S_IROTH/S_IWOTH/S_IXOTH/S_IRWXO     这4个是其他用户具有的权限，同上

3. ssize_t read(int fd,void *buf,size_t count)
    read函数只是一个通用的读文件设备的接口，是否阻塞需要由设备的属性和设置所决定，
    一般读字符终端、socket、FIFO时，默认都是阻塞方式，
    读磁盘上的常规文件时是不阻塞的

4. ssize_t write(int fd,const void *buf,size_t count)
    write函数返回成功并不能保证数据已经写到磁盘,因为write函数只是将数据拷贝到了内核缓冲区.
    write期间如果尚未拷贝任何数据到内核缓冲区就被信号中断,则调用失败并且errno为EINTER；如果写入了超过1个数据后被信号中断,则调用成功并返回成功写入的字节数
                    
                    
linux整个文件的设计架构：
                                                                 *                                                    *
                                |-----------|        |----------|*    |---------|                                     * |-------|       |-------|
                     控制流---> |           |   ---> |          |*--->|         |  ---------------------------------->* |       |  ---> |       |
                                |application|        |   clib   |*    |   page  |       |-------|                     * |  disk |       |  disk |
                                |   buffer  |        |  buffer  |*    |  cache  |       |   IO  |       |-------|     * | cache |       |       |
                     数据流===> |           |   ===> |          |*===>|         |  ==>  | queue |  ==>  | driver|  ==>* |       |  ===> |       |
                                |-----------|        |----------|*    |---------|       |-------|       |-------|     * |-------|       |-------|
                                                                 *                                                    *
                                                                 *                                                    *
                                            应用层               *                      内核层                        *         设备层  
                                                                 *                                                    *
                                                            系统调用接口                                        

文件的几种写入流程：
    1） fwrite方式就是把数据从application buffer拷贝到了clib buffer，fwrite返回后，数据还在clib buffer，整个操作都是应用层行为，数据依旧是在该进程的内存空间
        当调用fclose时，fclose调用会把这些数据刷新到disk上；
        除了fclose方法外，也可以通过fflush + fsync主动刷新，fflush是把数据从clib buffer拷贝到page cache，fsync是把数据从page cache刷新到disk
    2） write方式就是直接通过系统调用把数据从应用层拷贝到内核层，也就是从application buffer拷贝到page cache中；
        系统调用会触发用户态 <---> 内核态切换，为了避免消耗，可以使用mmap把page cache地址空间映射到用户空间，应用程序像操作应用层内存一样写文件，免去了系统调用开销
    3） 如果想绕过page cache直接把数据送到设备，可以通过open文件时带上O_DIRECT标志（不推荐）
    4） 如果想绕开文件系统直接写扇区，那就是RAW设备写

安全性问题：
    当进程挂掉后，如果数据还在application buffer或clib buffer，则必然丢失了，一旦数据进入内核层，到了page cache，只要内核不死就不会丢失了
    当内核挂掉后，只要数据没有到达disk cache，则必然丢失了

    


























