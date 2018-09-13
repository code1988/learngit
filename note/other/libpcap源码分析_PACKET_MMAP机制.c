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

    /* 最后还需要创建一个pcap内部用于管理接收环形缓冲区每个帧头/块头的数组
     * 备注： 对于V3模式来说，pcap实际是基于内存块进行管理的
     */
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
    // 开启PACKET_MMAP情况下，offset字段其实只在每次执行捕获期间有意义
    handle->offset = 0;
    return 1;
}

小结： 至此已经成功开启PACKET_MMAP功能，显然其中的核心部分在于环形缓冲区的配置，
       接下来将以pcap_read_linux_mmap_v3为线索分析如何在开启PACKET_MMAP的情况下进行捕获

int pcap_read_linux_mmap_v3(pcap_t *handle, int max_packets, pcap_handler callback,u_char *user)
{
    // 获取该pcap句柄的私有空间
    struct pcap_linux *handlep = handle->priv;
    int pkts = 0;       // 记录了成功捕获的报文数量
    union thdr h;
    
again:
    // 等待接收环形缓冲区中有内存块提交给用户空间
    if (handlep->current_packet == NULL) {
        h.raw = RING_GET_CURRENT_FRAME(handle);
        // 如果当前等待读取的内存块中没有报文，则在这里进行等待
        if (h.h3->hdr.bh1.block_status == TP_STATUS_KERNEL) {
            pcap_wait_for_frames_mmap(handle);
        }
    }

    /* 再次检查接收环形缓冲区中是否有内存块提交给用户空间，如果没有，通常就会结束捕获操作;
     * 除非该pcap句柄设置的是收到至少1个报文捕获操作才会结束
     */
    h.raw = RING_GET_CURRENT_FRAME(handle);
    if (h.h3->hdr.bh1.block_status == TP_STATUS_KERNEL) {
        if (pkts == 0 && handlep->timeout == 0)
            goto again;

        return pkts;
    }

    // 程序运行到这里意味着当前等待读取的内存块中有报文可读
    /* 循环捕获并处理数据包，具体的流程就是依次遍历每个内存块以及其中的每个帧，进行读取,
     * 直到达到设置的捕获数量或者接收环形缓存区没有报文可读
     */
    while ((pkts < max_packets) || PACKET_COUNT_IS_UNLIMITED(max_packets)) {
        int packets_to_read;

        // 每次读取一个新的内存块前都需要刷新current_packet和packets_left
        if (handlep->current_packet == NULL) {
            // 如果当前等待读取的内存块中没有报文，则退出捕获
            h.raw = RING_GET_CURRENT_FRAME(handle);
            if (h.h3->hdr.bh1.block_status == TP_STATUS_KERNEL) 
                break;

            handlep->current_packet = h.raw + h.h3->hdr.bh1.offset_to_first_pkt;
            handlep->packets_left = h.h3->hdr.bh1.num_pkts;
        }

        /* 在一个内存块中要读取的报文数量不仅受到该内存块中能被读取的报文数量的限制,
         * 还收到本次捕获操作设置的捕获数量的限制
         */
        packets_to_read = handlep->packets_left;
        if (!PACKET_COUNT_IS_UNLIMITED(max_packets) && packets_to_read > (max_packets - pkts)) 
            packets_to_read = max_packets - pkts;

        /* 从一个内存块中循环捕获packets_to_read数量的帧，直到满足以下任一条件才退出：
         *      超过了要读取的包数量;
         *      强制退出
         */
        while (packets_to_read-- && !handle->break_loop) {
            // 获取接收环形缓冲区中每个帧的头部结构
            struct tpacket3_hdr* tp3_hdr = (struct tpacket3_hdr*) handlep->current_packet;
            // 尝试捕获环形缓冲区中每个帧
            ret = pcap_handle_packet_mmap(
                    handle,
                    callback,
                    user,
                    handldp->current_packet,
                    tp3_hdr->tp_len,
                    tp3_hdr->tp_mac,
                    tp3_hdr->tp_snaplen,
                    tp3_hdr->tp_sec,
                    handle->opt.tstamp_precision == PCAP_TSTAMP_PRECISION_NANO ? tp3_hdr->tp_nsec : tp3_hdr->tp_nsec / 1000,
                    (tp3_hdr->hv1.tp_vlan_tci || (tp3_hdr->tp_status & TP_STATUS_VLAN_VALID)),
                    tp3_hdr->hv1.tp_vlan_tci,
                    VLAN_TPID(tp3_hdr, &tp3_hdr->hv1));
            if (ret == 1) {
                // 这种情况意味着成功捕获到一帧
                pkts++;
                handlep->packets_read++;
            } else if (ret < 0) {
                // 这种情况意味着捕获该帧的过程中出现错误
                handlep->current_packet = NULL;
                return ret;
            }

            // 准备读取该内存块中的下一帧
            handlep->current_packet += tp3_hdr->tp_next_offset;
            handlep->packets_left--;
        }

        // 当一个内存块中的帧都被读取之后，需要把该内存块还给内核
        if (handlep->packets_left <= 0) {
            h.h3->hdr.bh1.block_status = TP_STATUS_KERNEL;

            // 准备读取下一个内存块
            if (++handle->offset >= handle->cc) 
                handle->offset = 0;

            handlep->current_packet = NULL;
        }

        // 每次读完一个内存块后都会检查是否要求强制退出
        if (handle->break_loop) {
            handle->break_loop = 0;
            return PCAP_ERROR_BREAK;
        }
    }

    // 如果该pcap句柄设置的超时为0,则会一直等待下去直到收到报文为止
    if (pkts == 0 && handlep->timeout == 0)
        goto again;

    return pkts;
}

int pcap_handle_packet_mmap(...)
{
    struct sockaddr_ll *sll; 
    unsigned char *bp;
    struct pcap_pkthdr pcaphdr;

    // 确保帧长不超过阈值
    if (tp_mac + tp_snaplen > handle->bufsize)
        return -1;

    bp = frame + tp_mac;
    sll = (void *)frame + TPACKET_ALIGN(handlep->tp_hdrlen);
    // 如果工作在加工模式，这在这里构建对应的伪头
    if (handlep->cooked) {
        // 略
    }

    // 如果该pcap句柄开启了用户级别过滤，就在这里进行过滤
    if (handlep->filter_in_userland && handle->fcode.bf_insns) {
        // 略
    }

    // 拒绝方向不匹配的包
    if (!linux_check_direction(handle, sll))
        return 0;

    // 记录包的基础信息
    pcaphdr.ts.tv_sec = tp_sec;
    pcaphdr.ts.tv_usec = tp_usec;
    pcaphdr.caplen = tp_snaplen;
    pcaphdr.len = tp_len;
    // 如果工作在加工模式，两个长度字段还要加上伪头长
    if (handlep->cooked) {
        pcaphdr.caplen += SLL_HDR_LEN;
        pcaphdr.len += SLL_HDR_LEN;
    }

    // 只有TPACKET_V2和TPACKET_V3才支持携带vlan信息，这里就是对vlan帧进行了复原
    if (tp_vlan_tci_valid && handlep->vlan_offset != -1 && tp_snaplen >= (unsigned int) handlep->vlan_offset) {
        struct vlan_tag *tag;
        bp -= VLAN_TAG_LEN;
        memmove(bp, bp + VLAN_TAG_LEN, handlep->vlan_offset);
        tag = (struct vlan_tag *)(bp + handlep->vlan_offset);
        tag->vlan_tpid = htons(tp_vlan_tpid);
        tag->vlan_tci = htons(tp_vlan_tci); 
        pcaphdr.caplen += VLAN_TAG_LEN;
        pcaphdr.len += VLAN_TAG_LEN;
    }

    // 如果用户空间收到了超出阈值长度的包，则截断
    if (pcaphdr.caplen > (bpf_u_int32)handle->snapshot)
        pcaphdr.caplen = handle->snapshot;

    // 最后调用用户注册的捕获回调函数
    callback(user, &pcaphdr, bp);

    return 1;
}

小结： 至此，基于TPACKET_V3模式的PACKET_MMAP机制已经分析完毕，至于libpcap中关于TPACKET_V1、TPACKET_V3模式的捕获流程不再做分析.
       这里仅仅列出了V1、V2、V3这三种模式的差别：
            TPACKET_V1 : 这是缺省的环形缓冲区版本
            TPACKET_V2 : 相比V1的改进有以下几点
                                32位的用户空间环形缓冲区可以基于64位内核工作;
                                时间戳的精度从ms提升到ns;
                                支持携带VLAN信息(这意味着通过V1接收到的vlan包将会丢失vlan信息)；
            TPACKET_V3 : 相比V2的改进有以下几点
                                内存块可以配置成可变帧长(V1、V2的帧长都是tpacket_req.tp_frame_size固定值);
                                read/poll基于block-level(V1、V2基于packet_level);
                                开始支持poll超时参数；
                                新增了用户可配置选项：tp_retire_blk_tov等；
                                RX Hash数据可以被用户使用；
备注： 需要注意的是，V3当前只支持接收环形缓冲区.

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

// 开启PACKET_MMAP时libpcap对V1、V2、V3版本环形缓冲区的统一管理结构
union thdr {
    struct tpacket_hdr          *h1;
    struct tpacket2_hdr         *h2;
    struct tpacket_block_desc   *h3;     
    void *raw;
}

// TPACKET_V3环形缓冲区每个帧的头部结构
struct tpacket3_hdr {
    __u32       tp_next_offset; // 指向同一个内存块中的下一个帧
    __u32       tp_sec;         // 时间戳(s)
    __u32       tp_nsec;        // 时间戳(ns)
    __u32       tp_snaplen;     // 捕获到的帧实际长度
    __u32       tp_len;         // 帧的理论长度
    __u32       tp_status;      // 帧的状态
    __u16       tp_mac;         // 以太网MAC字段距离帧头的偏移量
    __u16       tp_net;
    union {
        struct tpacket_hdr_variant1 hv1;    // 包含vlan信息的子结构
    };
    __u8        tp_padding[8];
}
