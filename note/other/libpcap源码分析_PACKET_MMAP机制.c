使用PACKET_MMAP机制的原因：
        不开启PACKET_MMAP时的捕获过程是非常低效的，它使用非常受限的缓冲区，并且每捕获一个报文就需要一次系统调用，
        如果还想获取这个报文的时间戳，就需要再执行一次系统调用.
        而启用PACKET_MMAP的捕获过程就是非常高效的，它提供了一个映射到用户空间的长度可配的环形缓冲区，这个缓冲区可以用于收发报文.
        用这种方式接收报文时，只需要等待报文到来即可，大部分情况下都不需要发出一个系统调用；
        用这种方式发送报文时，多个报文只需要一个系统调用就可以以最高带宽发送出去.
        另外，用户空间和内核使用共享缓存的方式可以减少报文的拷贝。

下面就从libpcap中的activate_mmap函数为线索，展开对PACKET_MMAP使用方法的分析。
/* 尝试对指定pcap句柄开启PACKET_MMAP功能
 * @返回值  1表示成功开启；0表示系统不支持PACKET_MMAP功能；-1表示出错
 */
int activate_mmap(pcap_t *handle, int *status)
{
    // 获取该pcap句柄的私有空间
    struct pcap_linux *handlep = handle->priv;

    // 分配一块缓存用于oneshot的情况，缓存大小为该pcap句柄支持捕获的最大包长
    handlep->oneshot_buffer = malloc(handle->snapshot);

    // 设置普通缓冲区的默认长度为2M，这里将尝试作为PACKET_MMAP的环形缓冲区使用
    if (handle->opt.buffer_size == 0) 
        handle->opt.buffer_size = 2*1024*1024;

    // 为该捕获套接字设置合适的环形缓冲区版本，优先考虑设置TPACKET_V3
    prepare_tpacket_socket(handle);

    // 创建环形缓冲区，并将其映射到用户空间
    create_ring(handle, status);

    // 根据环形缓冲区版本注册linux上PACKET_MMAP读操作回调函数
    switch (handlep->tp_version) {
    case TPACKET_V1:
        handle->read_op = pcap_read_linux_mmap_v1;
        break;
    case TPACKET_V1_64:
        handle->read_op = pcap_read_linux_mmap_v1_64;
        break;
    case TPACKET_V2:
        handle->read_op = pcap_read_linux_mmap_v2;
        break;
    case TPACKET_V3:
        handle->read_op = pcap_read_linux_mmap_v3;
        break;
    }

    // 最后注册一系列linux上PACKET_MMAP相关回调函数
    handle->cleanup_op = pcap_cleanup_linux_mmap;
    handle->setfilter_op = pcap_setfilter_linux_mmap;
    handle->setnonblock_op = pcap_setnonblock_mmap;
    handle->getnonblock_op = pcap_getnonblock_mmap;
    handle->oneshot_callback = pcap_oneshot_mmap;
    handle->selectable_fd = handle->fd;
    return 1;
}

int prepare_tpacket_socket(pcap_t *handle)
{
    // 获取该pcap句柄的私有空间
    struct pcap_linux *handlep = handle->priv;
    int ret;

    /* 只有该pcap句柄没有使能immediate标识的前提下，才会首先尝试将环形缓冲区版本设置为TPACKET_V3
     * 这是因为实现决定了TPACKET_V3模式下报文可能无法被实时传递给用户
     */
    if (!handle->opt.immediate) {
        ret = init_tpacket(handle, TPACKET_V3, "TPACKET_V3"); 
        if (ret == 0)       // 成功开启TPACKET_V3模式
            return 1;
        else if (ret == -1) // 开启TPACKET_V3模式失败且并非是kernel不支持的原因
            return -1;
    }

    // 在kernel不支持TPACKET_V3模式的情况下，则尝试开启TPACKET_V2模式
    ret = init_tpacket(handle, TPACKET_V2, "TPACKET_V2");
    if (ret == 0)       // 成功开启TPACKET_V2模式
        return 1;
    else if (ret == -1) // 开启TPACKET_V2模式失败且并非是kernel不支持的原因
        return -1;

    /* 在kernel不支持TPACKET_V3、TPACKET_V2模式的情况下，则最后临时假设为TPACKET_V1模式
     * 因为只要内核支持PACKET_MMAP机制，就必然支持TPACKET_V1模式
     */
    handlep->tp_version = TPACKET_V1;
    handlep->tp_hdrlen = sizeof(struct tpacket_hdr);

    return 1;
}

int init_tpacket(pcap_t *handle, int version, const char *version_str)
{
    // 获取该pcap句柄的私有空间
    struct pcap_linux *handlep = handle->priv;
    int val = version;
    socklen_t len = sizeof(val);

    // 首先尝试获取该版本环形缓冲区中帧头长，这也是一种探测内核是否支持该版本的环形缓冲区的方式
    if (getsockopt(handle->fd, SOL_PACKET, PACKET_HDRLEN, &val, &len) < 0) {
        // 返回这两种错误号都表示kernel不支持
        if (errno == ENOPROTOOPT || errno == EINVAL)
            return 1;

        return -1;
    }
    handlep->tp_hdrlen = val; 

    // 如果内核支持，则将该pcap句柄关联的接字设置一个该版本的环形缓冲区
    val = version;
    setsockopt(handle->fd, SOL_PACKET, PACKET_VERSION, &val,sizeof(val));
    handlep->tp_version = version;

    // 设置环形缓冲区中每个帧VLAN_TAG_LEN长度的保留空间，用于VLAN tag重组
    val = VLAN_TAG_LEN;
    setsockopt(handle->fd, SOL_PACKET, PACKET_RESERVE, &val,sizeof(val));
    
    return 0;
}

int create_ring(pcap_t *handle, int *status)
{
    // 获取该pcap句柄的私有空间
    struct pcap_linux *handlep = handle->priv;
    struct tpacket_req3 req;
    socklen_t len;
    unsigned int sk_type,tp_reserve, maclen, tp_hdrlen, netoff, macoff;
    unsigned int frame_size;
    
    // 根据配置的版本创建对应的接收环形缓冲区
    switch (handlep->tp_version) {
    case TPACKET_V1:
    case TPACKET_V2: 
        /* V1、V2版本需要设置一个合适的环形缓冲区帧长，缺省同步自snapshot，
         * 但是因为snapshot可能设置了一个极大的值，这会导致一个环形缓冲区放不下几个帧，并且存在大量空间的浪费，
         * 所以接下来会尝试进一步调整为一个合理的帧长值
         */
        frame_size = handle->snapshot;
        // 针对以太网接口调整环形缓冲区帧长
        if (handle->linktype == DLT_EN10MB) {
            int mtu;
            int offload;

            // 检查该接口是否支持offload机制
            offload = iface_get_offload(handle);
            // 对于不支持offload机制的接口，可以使用该接口的MTU值来进一步调整环形缓冲区的帧长
            if (!offload) {
                mtu = iface_get_mtu(handle->fd, handle->opt.device,handle->errbuf); 
                if (frame_size > (unsigned int)mtu + 18)
                    frame_size = (unsigned int)mtu + 18; 
            }
        }

        // 获取套接字类型
        len = sizeof(sk_type);
        getsockopt(handle->fd, SOL_SOCKET, SO_TYPE, &sk_type,&len);
        /* 获取环形缓冲区中每个帧的保留空间长度
         * 备注：对于V3/V2模式的环形缓冲区，之前是有设置过VLAN_TAG_LEN字节的保留空间，而V1模式则没有设置过
         */
        len = sizeof(tp_reserve);
        if (getsockopt(handle->fd, SOL_PACKET, PACKET_RESERVE,&tp_reserve, &len) < 0) {
            if (errno != ENOPROTOOPT)
                return -1;

            tp_reserve = 0;
        }

        // 以下一系列计算的最终目的是得到一个合适帧长值
        maclen = (sk_type == SOCK_DGRAM) ? 0 : MAX_LINKHEADER_SIZE;
        tp_hdrlen = TPACKET_ALIGN(handlep->tp_hdrlen) + sizeof(struct sockaddr_ll) ;
        netoff = TPACKET_ALIGN(tp_hdrlen + (maclen < 16 ? 16 : maclen)) + tp_reserve;
        macoff = netoff - maclen;
        req.tp_frame_size = TPACKET_ALIGN(macoff + frame_size);     // 最终通过一系列计算才得到合适的帧长值
        req.tp_frame_nr = handle->opt.buffer_size/req.tp_frame_size;// 得到帧长值之后，就可以进一步计算得到环形接收缓冲区可以存放的帧总数
        break;
    case TPACKET_V3: 
        // 区别于V1/V2，V3的帧长可变，只需要设置一个帧长上限值即可
        req.tp_frame_size = MAXIMUM_SNAPLEN;
        req.tp_frame_nr = handle->opt.buffer_size/req.tp_frame_size; 
        break;
    }

    /* 计算V1/V2/V3的内存块长度,内存块长度只能取PAGE_SIZE * 2^n，并且要确保至少放下1个帧
     * 备注：由于V3模式设置帧长上限MAXIMUM_SNAPLEN必然大于PAGE_SIZE，所以可知V3模式下1个内存块中只会有1个帧
     */
    req.tp_block_size = getpagesize();
    while (req.tp_block_size < req.tp_frame_size)
        req.tp_block_size <<= 1;

    frames_per_block = req.tp_block_size/req.tp_frame_size;

retry:
    // 计算内存块数量和帧总数，这里显然再次对帧总数进行调整，最终确保帧总数是内存块总数的整数倍
    req.tp_block_nr = req.tp_frame_nr / frames_per_block;
    req.tp_frame_nr = req.tp_block_nr * frames_per_block;

    // 设置每个内存块的寿命
    req.tp_retire_blk_tov = (handlep->timeout>=0)?handlep->timeout:0;
    // 每个内存块不设私有空间
    req.tp_sizeof_priv = 0; 
    // 清空环形缓冲区的标志集合
    req.tp_feature_req_word = 0;

    // 创建接收环形缓冲区
    if (setsockopt(handle->fd, SOL_PACKET, PACKET_RX_RING,(void *) &req, sizeof(req))) {
        // 如果失败原因是内存不足，则减少帧总数然后再次进行创建
        if ((errno == ENOMEM) && (req.tp_block_nr > 1)) {
            if (req.tp_frame_nr < 20)
                req.tp_frame_nr -= 1;
            else
                req.tp_frame_nr -= req.tp_frame_nr/20;

            goto retry;
        }
        // 如果kernel不支持PACKET_MMAP则直接返回
        if (errno == ENOPROTOOPT)
            return 0;
    }

    // 程序运行到这里意味着接收环形缓冲区创建成功
    // 接着就是将新创建的接收环形缓冲区映射到用户空间
    handlep->mmapbuflen = req.tp_block_nr * req.tp_block_size;
    handlep->mmapbuf = mmap(0, handlep->mmapbuflen,PROT_READ|PROT_WRITE, MAP_SHARED, handle->fd, 0);

    // 最后还需要创建一个pcap内部用于管理接收环形缓冲区每个帧头/块头的数组
    handle->cc = req.tp_frame_nr;
    handle->buffer = malloc(handle->cc * sizeof(union thdr *));

    // 将接收环形缓冲区中每个帧头地址记录到管理数组buffer中
    handle->offset = 0;
    for (i=0; i<req.tp_block_nr; ++i) {
        void *base = &handlep->mmapbuf[i*req.tp_block_size];
        for (j=0; j<frames_per_block; ++j, ++handle->offset) {
            RING_GET_CURRENT_FRAME(handle) = base; 
            base += req.tp_frame_size;
        }
    }

    handle->bufsize = req.tp_frame_size;
    // 开启PACKET_MMAP情况下，offset字段其实不再有意义
    handle->offset = 0;
    return 1;
}

小结： 至此已经成功开启PACKET_MMAP功能，显然其中的核心部分在于环形缓冲区的配置，
       接下来将以pcap_read_linux_mmap_v3为线索分析如何在开启PACKET_MMAP的情况下进行捕获

相关数据结构：
/* 创建TPACKET_V3环形缓冲区时对应的配置参数结构
 * 备注： tpacket_req3结构是tpacket_req结构的超集，实际可以统一使用本结构去设置所有版本的环形缓冲区，V1/V2版本会自动忽略多余的字段
 */
struct tpacket_req3 {
    unsigned int    tp_block_size;      // 每个连续内存块的最小尺寸(必须是 PAGE_SIZE * 2^n )
    unsigned int    tp_block_nr;        // 内存块数量
    unsigned int    tp_frame_size;      // 每个帧的大小(虽然V3中的帧长是可变的，但创建时还是会传入一个最大的允许值)
    unsigned int    tp_frame_nr;        // 帧的总个数(必须等于 每个内存块中的帧数量*内存块数量)
    unsigned int    tp_retire_blk_tov;  // 内存块的寿命(ms)，超时后即使内存块没有被数据填入也会被内核停用，0意味着不设超时
    unsigned int    tp_sizeof_priv;     // 每个内存块中私有空间大小，0意味着不设私有空间
    unsigned int    tp_feature_req_word;// 标志位集合(目前就支持1个标志 TP_FT_REQ_FILL_RXHASH)
}
