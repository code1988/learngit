libpcap是跨平台网络数据包捕获函数库，本文将基于Linux平台对其源码以及核心原理进行深入分析
备注： 以下分析都基于libpcap-1.8.1版本进行
       以下分析按照库的核心API为线索展开
       以下分析源码时只列出核心逻辑

1. API: pcap_open_live 
   描述： 针对指定的网络接口创建一个捕获句柄，用于后续捕获数据
   实现逻辑分析：
    @device  - 指定网络接口名，比如"eth0"。如果传入NULL或"any"，则意味着对所有接口进行捕获
    @snaplen - 设置每个数据包的捕捉长度，上限MAXIMUM_SNAPLEN
    @promisc - 是否打开混杂模式
    @to_ms   - 设置获取数据包时的超时时间(ms)
    备注：to_ms值会影响3个捕获函数(pcap_next、pcap_loop、pcap_dispatch)的行为
    pcap_t *pcap_open_live(const char *device, int snaplen, int promisc, int to_ms, char *errbuf)
    {
        pcap_t *p;
        // 基于指定的设备接口创建一个pcap句柄
        p = pcap_create(device, errbuf);
        // 设置最大捕获包的长度
        status = pcap_set_snaplen(p, snaplen);
        // 设置数据包的捕获模式
        status = pcap_set_promisc(p, promisc);
        // 设置执行捕获操作的持续时间
        status = pcap_set_timeout(p, to_ms);
        // 使指定pcap句柄进入活动状态，这里实际包含了创建捕获套接字的动作
        status = pcap_activate(p);
        return p;
    }
    
    pcap_t *pcap_create(const char *device, char *errbuf)
    {
        pcap_t *p;
        char *device_str;

        // 转储传入的设备名，如果传入NULL，则设置为"any"
        if (device == NULL)
            device_str = strdup("any");
        else 
            device_str = strdup(device);

        // 创建一个普通网络接口类型的pcap句柄
        p = pcap_create_interface(device_str, errbuf); 
        p->opt.device = device_str;
        return p;
    }

    pcap_t *pcap_create_interface(const char *device, char *ebuf)
    {
        pcap_t *handle;
        // 创建并初始化一个包含私有空间struct pcap_linux的pcap句柄
        handle = pcap_create_common(ebuf, sizeof (struct pcap_linux));

        // 在刚创建了该pcap句柄后，这里首先覆盖了2个回调函数
        handle->activate_op = pcap_activate_linux;
        handle->can_set_rfmon_op = pcap_can_set_rfmon_linux;
    }

    pcap_t *pcap_create_common(char *ebuf, size_t size)
    {
        pcap_t *p;
        // 申请一片连续内存,用作包含size长度私有空间的pcap句柄，其中私有空间紧跟在该pcap结构后
        p = pcap_alloc_pcap_t(ebuf, size);

        // 为新建的pcap句柄注册一系列缺省的回调函数，这些缺省的回调函数大部分会在后面覆盖为linux下对应回调函数
        p->can_set_rfmon_op = pcap_cant_set_rfmon;
        initialize_ops(p);

        return p;
    }

    int pcap_set_snaplen(pcap_t *p, int snaplen)
    {
        // 设置该pcap句柄捕获包的最大长度前，需要确保当前并未处于活动状态
        if (pcap_check_activated(p))
            return (PCAP_ERROR_ACTIVATED); 

        // 如果传入了无效的最大包长，则会设置为缺省值
        if (snaplen <= 0 || snaplen > MAXIMUM_SNAPLEN)
            snaplen = MAXIMUM_SNAPLEN;

        p->snapshot = snaplen;
    }
    
    int pcap_set_promisc(pcap_t *p, int promisc)
    {
        // 设置该pcap句柄关联接口的数据包捕获模式前，需要确保当前并未处于活动状态
        if (pcap_check_activated(p))
            return (PCAP_ERROR_ACTIVATED); 

        p->opt.promisc = promisc;
    }

    int pcap_set_timeout(pcap_t *p, int timeout_ms)
    {
        // 设置该pcap句柄执行捕获操作的持续时间前，需要确保当前并未处于活动状态
        if (pcap_check_activated(p))
            return (PCAP_ERROR_ACTIVATED); 

        p->opt.timeout = timeout_ms;
    }

    int pcap_activate(pcap_t *p)
    {
        int status;
        // 确保没有进行重复激活
        if (pcap_check_activated(p))
            return (PCAP_ERROR_ACTIVATED); 
        
        // 调用事先注册的activate_op方法，完成对该pcap句柄的激活，这个过程中会创建用于捕获的套接字，以及尝试开启PACKET_MMAP机制
        status = p->activate_op(p);
        return status;
    }

    int pcap_activate_linux(pcap_t *handle)
    {
        // 获取该pcap句柄的私有空间
        struct pcap_linux *handlep = handle->priv;

        device = handle->opt.device;

        // 为该pcap句柄注册linux平台相关的一系列回调函数(linux平台的回调函数又分为2组，这里缺省注册了一组不使用PACKET_MMAP机制的回调)
        handle->inject_op = pcap_inject_linux;
        handle->setfilter_op = pcap_setfilter_linux;
        handle->setdirection_op = pcap_setdirection_linux;
        handle->set_datalink_op = pcap_set_datalink_linux;
        handle->getnonblock_op = pcap_getnonblock_fd;
        handle->setnonblock_op = pcap_setnonblock_fd;
        handle->cleanup_op = pcap_cleanup_linux;
        handle->read_op = pcap_read_linux;
        handle->stats_op = pcap_stats_linux;

        // "any"设备不支持混杂模式
        if (strcmp(device, "any") == 0) {
            if (handle->opt.promisc) 
                handle->opt.promisc = 0;
        }

    }
   
   相关数据结构：
    // 对一个接口执行捕获操作的句柄结构
    struct pcap {
        read_op_t read_op;      // 在该接口上进行读操作的回调函数(linux上就是 pcap_read_linux / pcap_read_linux_mmap_v3)
        int fd;                 // 该接口关联的套接字
        int selectable_fd;      // 通常就是fd
        u_int bufsize;          // 该值初始时来自用户配置的snapshot，当开启PACKET_MMAP时，跟配置的接收环形缓冲区tp_frame_size值同步
        void *buffer;           // 当开启PACKET_MMAP时，指向一个成员为union thdr结构的数组，记录了接收环形缓冲区中每个帧的帧头;当不支持PACKET_MMAP时，暂略
        int cc;                 // 跟配置的接收环形缓冲区tp_frame_nr值同步(由于pcap中内存块数量和帧数量相等，所以本字段也就是内存块数量)
        int break_loop;         // 标识是否强制退出循环捕获
        void *priv;             // 指向该pcap句柄的私有空间(紧跟在本pcap结构后)，linux下就是struct pcap_linux
        struct pcap *next;      // 这张链表记录了所有已经打开的pcap句柄，目的是可以被用于关闭操作
        int snapshot;           // 该pcap句柄支持的最大捕获包的长度,对于普通的以太网接口可以设置为1518,对于环回口可以设置为65549,其他情况下可以设置为MAXIMUM_SNAPLEN
        int linktype;           // 网络链路类型，对于以太网设备/环回设备，通常就是DLT_EN10MB
        int activated;          // 标识该pcap句柄是否处于运作状态，处于运作状态的pcap句柄将不允许进行修改
        pcap_direction_t direction;     // 捕包方向
        struct bpf_program fcode;       // BPF过滤模块
        int dlt_count;                  // 该设备对应的dlt_list中元素数量，通常为2
        u_int *dlt_list;                // 指向该设备对应的DLT_*列表

        activate_op_t activate_op;              // 对应回调函数：pcap_activate_linux
        can_set_rfmon_op_t can_set_rfmon_op;    // 对应回调函数：pcap_can_set_rfmon_linux
        inject_op_t inject_op;                  // 对应回调函数：pcap_inject_linux
        setfilter_op_t setfilter_op;            // 对应回调函数：pcap_setfilter_linux       / pcap_setfilter_linux_mmap
        setdirection_op_t setdirection_op;      // 对应回调函数：pcap_setdirection_linux
        set_datalink_op_t set_datalink_op;      // 对应回调函数：pcap_set_datalink_linux
        getnonblock_op_t getnonblock_op;        // 对应回调函数：pcap_getnonblock_fd        / pcap_getnonblock_mmap
        setnonblock_op_t setnonblock_op;        // 对应回调函数：pcap_setnonblock_fd        / pcap_setnonblock_mmap
        stats_op_t stats_op;                    // 对应回调函数：pcap_stats_linux
        pcap_handler oneshot_callback;          // 对应回调函数：pcap_oneshot_mmap
        cleanup_op_t cleanup_op;                // 对应回调函数：pcap_cleanup_linux_mmap
    }

    // pcap句柄包含的一个子结构
    struct pcap_opt {
        char *device;       // 接口名，比如"eth0"
        int timeout;        // 该pcap句柄进行捕获操作的持续时间(ms)，0意味着不超时
        u_int buffer_size;  // 接收缓冲区长度，缺省就是2M. 当PACKET_MMAP开启时，该值用来配置接收环形缓冲区;当不支持PACKET_MMAP时，该值用来配置套接字的接收缓冲区
        int promisc;        // 标识该pcap句柄是否开启混杂模式，需要注意的是，"any"设备不允许开启混杂模式
        int rfmon;          // 表示该pcap句柄是否开启监听模式，该模式只用于无线网卡
        int immediate;      // 标识收到报文时是否立即传递给用户
        int tstamp_type;        // 该pcap句柄使用的时间戳类型
        int tstamp_precision;   // 该pcap句柄使用的时间戳精度
    }

    // 跟pcap句柄关联的linux平台私有空间
    struct pcap_linux {
        u_int   packets_read;       // 统计捕获到的包数量
        long    proc_dropped;       // 统计丢弃的包数量

        char    *device;            // 接口名，比如"eth0"
        int filter_in_userland;     // 标识用户空间是否需要过滤包
        int timeout;                // 进行捕获操作的持续时间，同步自pcap->opt.timeout
    }

