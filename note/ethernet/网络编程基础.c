1. 主机字节序、网络字节序
---------------------------------------------------------------------------------------------------
    主机字节序通常采用little-endian，低字节放低地址，高字节放高地址
    网络字节序采用big-endian，低字节放高地址，高字节放低地址

    unsigned long htonl(unsigned long host)     // 主机long转网络long
    unsigned long ntohl(unsigned long net)      // 网络long转主机long
    unsigned short htons(unsigned short host)   // 主机short转网络short
    unsigned short ntohs(unsigned short net)    // 网络short转主机short

2. 点分十进制字符串IP和32位整型IP
---------------------------------------------------------------------------------------------------
    int inet_aton(const char *cp,struct in_addr *inp)   // 点分十进制字符串IP转32位整型IP,成功返回非0,失败返回0
    char *inet_ntoa(struct in_addr in)                  // 32位整型IP转点分十进制字符串IP
    
    更新的程序用inet_pton和inet_ntop来代替上面两个函数，因为新函数支持ipv4和ipv6
    int inet_pton(int af,const char *src,void *dst)                         // 点分十进制字符串IP转32位整型IP
    const char *inet_ntop(int af,const void *src,char *dst,socklen_t size)  // 32位整型IP转点分十进制字符串IP

    上面转换函数都用到的一个结构
        typedef uint32_t in_addt_t;
        struct in_addr{
            in_addr_t s_addr;                           // 网络字节序       
        }; 

3. 主机信息获取
---------------------------------------------------------------------------------------------------
    主机信息主要包括主机名/域名/主机别名、主机IP地址，这些都可以唯一标识一台主机。
    由于通过IP来标识主机的方式可读性太差，所以出现了主机名和域名。
    主机名和域名的区别在于：主机名用于局域网中，而域名用于公网中。

    struct hostent *gethostbyname(const char *name)                     // 根据域名或主机名来获取对应的主机信息，从而得到其中的IP地址
    struct hostent *gethostbyaddr(const void *addr,int len,int type)    // 根据IP地址获取主机信息，从而的到其中的域名或主机名
    这两个函数都返回一个hostent结构指针：
    struct hostent{
        char *h_name;                   // 主机的规范名，比如百度的规范名为"www.a.shifen.com"
        char **h_aliases;               // 主机的别名列表(别名可以有多个)，比如百度的别名只有一个，就是"www.baidu.com"
        int h_addrtype;                 // 主机IP地址类型，AF_INET/AF_INET6
        int h_length;                   // 主机IP地址长度
        char **h_addr_list;             // 主机的IP地址列表,对于AF_INET类型，列表元素其实就是(struct in_addr *)的结构
        #define h_addr h_addr_list[0]   // 指向主机的首个IP地址
    };

    以上2个API只支持IPV4，之后POSIX引入了一个新的API，可以同时支持IPV4和IPV6：
    int getaddrinfo(const char *hostname, const char *service, const struct addrinfo *hints, struct addrinfo **result)
    参数说明：
        @hostname   - 指向一个主机名/域名/地址串(比如"localhost"/"www.baidu.com"/"192.168.9.10")
                    - 当该值设置为NULL时，其含义跟AI_PASSIVE标志相关：
                                当设置了AI_PASSIVE时，返回的套接口地址为INADDR_ANY;
                                当没有设置AI_PASSIVE时,返回的套接口地址为本机环回地址
        @service    - 服务名可以是十进制的端口号字串(比如"3721")，也可以是标准的服务名称(比如"http")
        @hints      - 指向一个struct addrinfo结构，用于设置期望返回的信息类型
                    - 当该值设置为NULL时，意味着对返回的信息类型不做任何限制
        @result     - 用于存放一个返回得到的指针，该指针指向一个struct addrinfo结构的链表
                    - 需要注意的是，这张链表占用的空间需要调用者调用freeaddrinfo进行释放
        @返回值     - 0：成功，同时返回的信息存放在result中   非0：失败，同时调用gai_strerror可以获取对应的错误信息
    这个API同时使用了一个新的addrinfo结构指针用来设置和存放返回的主机信息：
    struct addrinfo
    {
        int ai_flags;			    // 附加选项集合(AI_*)，通常设置在hints中
        int ai_family;		        // 期望的地址族(AF_INET/AF_INET6/AF_UNSPEC)，通常设置在hints中
        int ai_socktype;		    // 期望的套接字类型(SOCK_STREAM/SOCK_DGRAM)，通常设置在hints中
        int ai_protocol;		    // 期望的协议类型(一般填0)
        socklen_t ai_addrlen;		// 通常记录在result中，表示满足期望条件的IP地址长度(4/6字节)
        struct sockaddr *ai_addr;	// 通常记录在result中，表示满足期望条件的IP地址
        char *ai_canonname;		    // 通常记录在result中，表示满足期望条件的主机信息(只有当hints.ai_flags设置了AI_CANONNAME时才会存在)
        struct addrinfo *ai_next;	// 指向满足期望条件的下一个addrinfo结构
    }
    使用说明：
            [1]. 如果hints.ai_flags设置了AI_NUMERICHOST，则传入的hostname必须是一个地址串，否则报错
            [2]. 如果hints.ai_flags设置了AI_NUMERICSERV，则传入的service必须是一个端口号字符串，否则报错
            [3]. 服务端获取套接口地址信息时通常会设置hints.ai_flags为AI_PASSIVE，并且hostname为NULL，这会返回适合bind和accept的套接字地址
            [4]. 客户端获取套接口地址信息时通常不设置AI_PASSIVE，因为这样返回的套接字地址适合connect和sendto/sendmsg
            
