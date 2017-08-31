netlink支持用户进程和内核相互交互（两边都可以主动发起），同时还支持用户进程之间相互交互（虽然这种应用通常都采用unix-sock）
但是有一点需要注意，内核不支持接收netlink组播消息
本文将从用户进程发送一个netlink消息开始，对整个netlink消息通信原理进行展开分析

用户进程一般都通过调用sendmsg来向内核或其他进程发送netlink消息(有关sendmsg系统调用的公用部分代码解析将在另一片文章中展开)
    /* 用户进程对netlink套接字调用sendmsg()系统调用后，内核执行netlink操作的总入口函数
     * @sock    - 指向用户进程的netlink套接字，也就是发送方的
     * @msg     - 承载了发送方传递的netlink消息
     * @len     - netlink消息长度
     * @kiocb   - 这个参数在最新内核中已经去掉，这里索性不再进行分析(其实是因为还没开撸这块代码~)
     *
     * 备注： netlink套接字在创建的过程中(具体是在__netlink_create函数开头)，已经和netlink_ops(socket层netlink协议族的通用操作集合)关联,
     *        其中注册的sendmsg回调就是指向本函数
     */
    static int netlink_sendmsg(struct kiocb *kiocb, struct socket *sock,struct msghdr *msg, size_t len)
    {
        // 显然，这里定义的addr指向netlink消息的目的地 
        DECLARE_SOCKADDR(struct sockaddr_nl *, addr, msg->msg_name);    

        // netlink消息不支持传输带外数据
        if (msg->msg_flags&MSG_OOB)
            return -EOPNOTSUPP;

        // 辅助消息处理
        if (NULL == siocb->scm)
            siocb->scm = &scm;
        err = scm_send(sock, msg, siocb->scm, true);

        if (msg->msg_namelen) {
            // 如果用户进程指定了目的地址，则对其进行合法性检测，然后从中获取单播地址和组播地址
            err = -EINVAL;
            if (addr->nl_family != AF_NETLINK)
                goto out;
            dst_portid = addr->nl_pid;
            dst_group = ffs(addr->nl_groups);
            err =  -EPERM;

            // 如果有设置目的组播地址，或者目的单播地址不是发往kernel，就需要检查具体的netlink协议是否支持
            if ((dst_group || dst_portid) && !netlink_allowed(sock, NL_CFG_F_NONROOT_SEND))
                goto out;
            netlink_skb_flags |= NETLINK_SKB_DST;
        
        }    
        else{
            // 如果用户进程没有指定目的地址，则使用该netlink套接字默认的(缺省都是0,可以通过connect系统调用指定)
            dst_portid = nlk->dst_portid;
            dst_group = nlk->dst_group;
        }

        // 如果该netlink套接字还没有被绑定过，首先执行动态绑定
        if (!nlk->portid){
            err = netlink_autobind(sock);
        }

        // 以下这部分只有当内核配置了CONFIG_NETLINK_MMAP选项才生效(暂不分析)
        if (netlink_tx_is_mmaped(sk) && msg->msg_iov->iov_base == NULL){
            err = netlink_mmap_sendmsg(sk, msg, dst_portid, dst_group,siocb);
        }

        // 检查需要发送的数据长度是否合法
        err = -EMSGSIZE;
        if (len > sk->sk_sndbuf - 32)
            goto out;

        // 分配skb结构空间
        err = -ENOBUFS;
        skb = netlink_alloc_large_skb(len, dst_group);

        // 初始化这个skb中的cb字段
        NETLINK_CB(skb).portid  = nlk->portid;
        NETLINK_CB(skb).dst_group = dst_group;
        NETLINK_CB(skb).creds   = siocb->scm->creds; 
        NETLINK_CB(skb).flags   = netlink_skb_flags;

        // 将数据从msg_iov拷贝到skb中
        err = -EFAULT;
        memcpy_fromiovec(skb_put(skb, len), msg->msg_iov, len)

        // 执行LSM模块，略过
        err = security_netlink_send(sk, skb);

        // 至此，netlink消息已经被放入新创建的sbk中，接下来，内核根据单播还是组播消息，执行了不同的发送流程
        // 发送netlink组播消息
        if (dst_group){
            atomic_inc(&skb->users);
            netlink_broadcast(sk, skb, dst_portid, dst_group, GFP_KERNEL);
        }

        // 发送netlink单播消息
        err = netlink_unicast(sk, skb, dst_portid, msg->msg_flags&MSG_DONTWAIT); 
    }

    小结：从netlink_sendmsg函数的末尾可以看出，来自用户进程的netlink消息会以netlink_unicast单播方式或netlink_broadcast组播方式进行传递。
          所以接下来进一步对这两种传播方式进行展开分析。


    /* 以下是内核执行netlink单播消息的发送流程
     * @ssk         - 源sock结构
     * @skb         - 属于发送方的承载了netlink消息的skb 
     * @portid      - 目的单播地址
     * @nonblock    - 1：非阻塞调用，2：阻塞调用
     *
     * 备注: 以下3种情况都会调用到本函数：
     *          [1]. kernel     --单播--> 用户进程
     *          [2]. 用户进程   --单播--> kernel 
     *          [3]. 用户进程a  --单播--> 用户进程b
     */
    int netlink_unicast(struct sock *ssk, struct sk_buff *skb,u32 portid, int nonblock)
    {
        // 重新调整skb数据区大小
        skb = netlink_trim(skb, gfp_any());
        
        // 计算发送超时时间(如果是非阻塞调用，则返回0)
        timeo = sock_sndtimeo(ssk, nonblock);

    retry:
        // 根据源sock结构和目的单播地址，得到目的sock结构
        sk = netlink_getsockbyportid(ssk, portid);

        // 如果该目的netlink套接字属于内核，则进入第 [2] 种情况下的分支
        if (netlink_is_kernel(sk))
            return netlink_unicast_kernel(sk, skb, ssk);

        // 程序运行到这里，意味着以下属于第 [1]/[3] 种情况下的分支
        // 对于发往用户进程的单播消息都要调用BPF过滤
        if (sk_filter(sk, skb)){
            err = skb->len;
            kfree_skb(skb);
            sock_put(sk);
            return err; 
        }

        // 将该skb绑定到目的用户进程netlink套接字上，如果成功，skb的所有者也从这里开始变更为目的用户进程netlink套接字
        err = netlink_attachskb(sk, skb, &timeo, ssk);
        // 如果返回值是1，意味着要重新尝试绑定操作
        if (err == 1)
            goto retry;
        if (err)
            return err;

        // 将该skb发送到目的用户进程netlink套接字
        return netlink_sendskb(sk, skb);
    }

    /* 来自用户进程的netlink消息单播发往内核netlink套接字
     * @sk  - 目的sock结构
     * @skb - 属于发送方的承载了netlink消息的skb
     * @ssk - 源sock结构
     *
     * 备注：skb的所有者在本函数中发生了变化
     */
    static int netlink_unicast_kernel(struct sock *sk, struct sk_buff *skb,struct sock *ssk)
    {
        // 获取目的netlink套接字，也就是内核netlink套接字
        struct netlink_sock *nlk = nlk_sk(sk);

        // 检查内核netlink套接字是否注册了netlink_rcv回调(就是各个协议在创建内核netlink套接字时通常会传入的input函数)
        if (nlk->netlink_rcv != NULL) {
            ret = skb->len;
            // 设置该skb的所有者是内核的netlink套接字
            netlink_skb_set_owner_r(skb, sk);
            // 保存该netlink消息的源sock结构
            NETLINK_CB(skb).sk = ssk;
            // netlink tap机制暂略
            netlink_deliver_tap_kernel(sk, ssk, skb);
            // 调用内核netlink套接字的协议类型相关的netlink_rcv回调
            nlk->netlink_rcv(skb);
        } else {
            // 如果指定的内核netlink套接字没有注册netlink_rcv回调，就直接丢弃所有收到的netlink消息
            kfree_skb(skb);
        }
        sock_put(sk);
    }
    
    /* 将一个指定skb绑定到一个指定的属于用户进程的netlink套接字上
     * @sk      - 指向目的sock结构
     * @skb     - 属于发送方的承载了netlink消息的skb
     * @timeo   - 指向一个超时时间
     * @ssk     - 指向源sock结构
     */
    int netlink_attachskb(struct sock *sk, struct sk_buff *skb,long *timeo, struct sock *ssk) 
    {
        // 获取目的netlink套接字，也就是目的用户进程netlink套接字
        nlk = nlk_sk(sk);

        /* 在没有内核使能CONFIG_NETLINK_MMAP配置选项时,
         * 如果目的netlink套接字上已经接收尚未处理的数据大小超过了接收缓冲区大小，或者目的netlink套接字被设置了拥挤标志，
         * 意味着该sbk不能立即被目的netlink套接字接收，需要加入等待队列 
         */
        if ((atomic_read(&sk->sk_rmem_alloc) > sk->sk_rcvbuf || test_bit(NETLINK_CONGESTED, &nlk->state)) && !netlink_skb_is_mmaped(skb)){
            // 申请一个等待队列
            DECLARE_WAITQUEUE(wait, current);
            // 如果传入的超时时间为0,意味着非阻塞调用，则丢弃这条netlink消息，并返回EAGAIN
            if (!*timeo) {
                /* 如果该netlink消息对应的源sock结构不存在，或者该netlink消息来自kernel
                 * 则对目的netlink套接字设置缓冲区溢出状态
                 */
                if (!ssk || netlink_is_kernel(ssk))
                    netlink_overrun(sk);
                sock_put(sk);
                kfree_skb(skb);
                return -EAGAIN; 
            }

            // 程序运行到这里意味着是阻塞调用
            // 改变当前进程状态为可中断
            __set_current_state(TASK_INTERRUPTIBLE);
            // 将目的netlink套接字加入等待队列（同时意味着进入睡眠，猜测）
            add_wait_queue(&nlk->wait, &wait);

            // 程序到这里意味着被唤醒了(猜测)
            // 如果接收条件还是不满足，则要计算剩余的超时时间
            if ((atomic_read(&sk->sk_rmem_alloc) > sk->sk_rcvbuf || test_bit(NETLINK_CONGESTED, &nlk->state)) && !sock_flag(sk, SOCK_DEAD))
                *timeo = schedule_timeout(*timeo);

            // 改变当前进程状态为运行
            __set_current_state(TASK_RUNNING);
            // 将目的netlink套接字从等待队列中删除
            remove_wait_queue(&nlk->wait, &wait);
            sock_put(sk); 

            // 目测是信号相关处理，略
            if (signal_pending(current)) {
                kfree_skb(skb);
                return sock_intr_errno(*timeo);
            }

            // 返回1,意味着还将要再次调用本函数
            return 1;
        }

        // 设置该skb的所有者是目的用户进程netlink套接字，这里才是真正的绑定操作，上面都是前奏
        netlink_skb_set_owner_r(skb, sk);
    }

    /* 将指定skb发送到目的用户进程netlink套接字(显然，本函数只是个封装)
     * @sk  - 目的用户进程netlink套接字
     * @skb - 指向一个承载了netlink消息的skb
     * @返回值  - 成功返回实际发送的netlink消息长度
     *
     * 备注：该skb调用本函数前已经绑定到该用户进程netlink套接字
     */
    int netlink_sendskb(struct sock *sk, struct sk_buff *skb)
    {
        int len = __netlink_sendskb(sk, skb);
        sock_put(sk);
        return len
    }

    /* 将指定skb发送到指定用户进程netlink套接字，换句话说，实际也就是将该skb放入了该套接字的接收队列中
     * @sk  - 指向一个用户进程netlink套接字
     * @skb - 指向一个承载了netlink消息的skb
     *
     * 备注：该skb调用本函数前已经绑定到该用户进程netlink套接字
     *       之所以说组播消息不支持发往内核的根本原因就在本函数中：
     *              本函数最后通过调用sk_data_ready钩子函数来通知所在套接字接收组播消息，
     *              而内核netlink套接字实际屏蔽了这个钩子函数，也就意味着永远无法接收到组播消息
     */
    static int __netlink_sendskb(struct sock *sk, struct sk_buff *skb)
    {
        // 这里可以知道，成功时的返回值就是skb中承载的netlink消息长度
        int len = skb->len;

        // 目前不知道tap系列函数的用处
        netlink_deliver_tap(skb);

        // 将skb放入该netlink套接字接收队列末尾
        skb_queue_tail(&sk->sk_receive_queue, skb);
        // 执行sk_data_ready回调通知该套接字有数据可读
        sk->sk_data_ready(sk, len);

        return len;
    }

    小结：至此，一个来自用户进程的netlink单播消息的传递流程基本分析完毕
          可以看出，流程的终点，一个是内核netlink套接字具体协议类型相关的netlink_rcv回调，另一个是目的用户进程netlink套接字的接收队列
          还可以看出，跟内核发往用户进程的netlink单播消息流程存在部分重合

    /* 发送netlink组播消息(显然这只是个封装)
     * @ssk         - 源sock结构
     * @skb         - 属于发送方的承载了netlink消息的skb
     * @portid      - 目的单播地址
     * @group       - 目的组播地址
     *
     * 备注: 以下2种情况都会调用到本函数：
     *          [1]. 用户进程   --组播--> 用户进程
     *          [2]. kernel     --组播--> 用户进程
     */
    int netlink_broadcast(struct sock *ssk, struct sk_buff *skb, u32 portid,u32 group, gfp_t allocation)
    {
        return netlink_broadcast_filtered(ssk, skb, portid, group, allocation,NULL, NULL);
    }

    /* 发送netlink组播消息
     * @ssk         - 源sock结构
     * @skb         - 属于发送方的承载了netlink消息的skb
     * @portid      - 目的单播地址
     * @group       - 目的组播地址
     * @filter      - 指向一个过滤器
     * @filter_data - 指向一个传递给过滤器的参数 
     */
    int netlink_broadcast_filtered(struct sock *ssk, struct sk_buff *skb, u32 portid,u32 group, gfp_t allocation,int (*filter)(struct sock *dsk, struct sk_buff *skb, void *data),void *filter_data)
    {
        // 获取源sock所在net命名空间
        struct net *net = sock_net(ssk);

        // 重新调整skb数据区大小
        skb = netlink_trim(skb, allocation);

        info.exclude_sk = ssk;
        info.net = net;
        info.portid = portid;
        info.group = group; 
        info.failure = 0; 
        info.delivery_failure = 0;
        info.congested = 0;
        info.delivered = 0;
        info.allocation = allocation;
        info.skb = skb; 
        info.skb2 = NULL; 
        info.tx_filter = filter; 
        info.tx_data = filter_data;

        // 遍历该netlink套接字所在协议类型中所有阅订了组播功能的套接字，然后尝试向其发送该组播消息
        sk_for_each_bound(sk, &nl_table[ssk->sk_protocol].mc_list)
            do_one_broadcast(sk, &info);

        // 至此，netlink组播消息已经发送完毕
        // 释放skb结构
        consume_skb(skb);

        // 发送失败处理
        if (info.delivery_failure){
            kfree_skb(info.skb2);
            return -ENOBUFS;
        }

        // 释放skb2结构
        consume_skb(info.skb2);

        // 发送成功处理
        if (info.delivered){
            if (info.congested && (allocation & __GFP_WAIT))
                yield();
            return 0;
        }
    }

    /* 尝试向指定用户进程netlink套接字发送组播消息
     * @sk  - 指向一个sock结构，对应一个用户进程netlink套接字
     * @p   - 指向一个netlink组播消息的管理块
     *
     * 备注：传入的netlink套接字跟组播消息属于同一种netlink协议类型，并且这个套接字开启了组播阅订，除了这些，其他信息(比如阅订了具体哪些组播)都是不确定的
     */
    static int do_one_broadcast(struct sock *sk,struct netlink_broadcast_data *p)
    {
        // 如果源sock和目的sock是同一个则直接返回
        if (p->exclude_sk == sk)
            goto out;
        // 如果目的单播地址就是该netlink套接字，或者目的组播地址超出了该netlink套接字的上限，或者该netlink套接字没有阅订这条组播消息，都直接返回
        if (nlk->portid == p->portid || p->group - 1 >= nlk->ngroups || !test_bit(p->group - 1, nlk->groups))
            goto out;
        // 如果两者不属于同一个net命名空间，则直接返回
        if (!net_eq(sock_net(sk), p->net))
            goto out;
        // 如果netlink组播消息的管理块携带了failure标志, 则对该netlink套接字设置缓冲区溢出状态
        if (p->failure){
            netlink_overrun(sk); 
            goto out;
        }

        // 设置skb2，其内容来自skb 
        if (p->skb2 == NULL){
            if (skb_shared(p->skb))
                p->skb2 = skb_clone(p->skb, p->allocation);
            else{
                p->skb2 = skb_get(p->skb);
                skb_orphan(p->skb2);
            }
        }

        if (p->skb2 == NULL){
            // 到这里如果skb2还是NULL，意味着上一步中clone失败
            netlink_overrun(sk);
            p->failure = 1;
            if (nlk->flags & NETLINK_BROADCAST_SEND_ERROR)
                p->delivery_failure = 1;
        }
        else if (p->tx_filter && p->tx_filter(sk, p->skb2, p->tx_data)){
            // 如果传入了发送过滤器，但是过滤不通过，释放skb2
            kfree_skb(p->skb2);
            p->skb2 = NULL;
        }
        else if (sk_filter(sk, p->skb2)){
            // 如果BPF过滤不通过，释放skb2 
            kfree_skb(p->skb2);
            p->skb2 = NULL;
        }
        else if ((val = netlink_broadcast_deliver(sk, p->skb2)) < 0){
            // 如果将承载了组播消息的skb发送到该用户进程netlink套接字失败
            netlink_overrun(sk);
            if (nlk->flags & NETLINK_BROADCAST_SEND_ERROR)
                p->delivery_failure = 1;
        }
        else{
            // 发送成功最终会进入这里
            p->congested |= val; 
            p->delivered = 1;
            p->skb2 = NULL;     // 这里没有释放skb2，而是在上层函数进行释放，原因是因为上层遍历时可能要反复进入本函数，所以skb2要被反复用到
        }
    }

    /* 将携带了netlink组播消息的skb发送到指定目的用户进程netlink套接字
     * @返回值      -1  :套接字接收条件不满足
     *              0   :netlink组播消息发送成功，套接字已经接收但尚未处理数据长度小于等于其接收缓冲的1/2
     *              1   :netlink组播消息发送成功，套接字已经接收但尚未处理数据长度大于其接收缓冲的1/2(这种情况似乎意味着套接字处于拥挤状态) 
     *
     * 备注：到本函数这里，已经确定了传入的netlink套接字跟组播消息匹配正确；
     *       netlink组播消息不支持阻塞
     */
    static int netlink_broadcast_deliver(struct sock *sk, struct sk_buff *skb) 
    {
        // 判断该netlink套接字是否满足接收条件
        if (atomic_read(&sk->sk_rmem_alloc) <= sk->sk_rcvbuf && !test_bit(NETLINK_CONGESTED, &nlk->state)){
            // 如果满足接收条件，则设置skb的所有者是该netlink套接字
            netlink_skb_set_owner_r(skb, sk);
            // 将skb发送到该netlink套接字，实际也就是将该skb放入了该套接字的接收队列中
            __netlink_sendskb(sk, skb);

            // 这里最后判断该netlink套接字的已经接收尚未处理数据长度是否大于其接收缓冲的1/2
            return atomic_read(&sk->sk_rmem_alloc) > (sk->sk_rcvbuf >> 1);
        }

        // 程序运行到这里意味着接收条件不满足
        return -1;
    }

    小结：至此，一个来自用户进程的netlink组播消息的传递流程基本分析完毕
          可以看出，流程的终点只有一个，就是目的用户进程netlink套接字的接收队列
          还可以看出，跟内核发往用户进程的netlink组播消息流程存在重合

以上就是用户进程向内核或其他进程发送netlink消息的完整流程，显然，其中重合了大量由内核发送netlink消息到用户进程的流程。
所以接下来内核发送netlink消息到用户进程的分析中，重合部分的代码将不再展开。
    /* 内核提供了API函数nlmsg_unicast供各个netlink协议调用，来向用户进程发送netlink单播消息(显然，这只是个封装)
     * @sk  - 指向源sock结构，也就是内核netlink套接字
     * @skb - 指向一个承载了单播netlink消息的skb
     * @portid  - 目的单播地址
     *
     * 备注：可以看出，内核只会以非阻塞的形式往用户进程发送netlink单播消息
     */
    static inline int nlmsg_unicast(struct sock *sk, struct sk_buff *skb, u32 portid)
    {
        netlink_unicast(sk, skb, portid, MSG_DONTWAIT);
    }

    /* 内核提供了API函数nlmsg_multicast供各个netlink协议调用，来向用户进程发送netlink组播消息(显然，这只是个封装)
     * 指向源sock结构
     * 指向一个承载了组播netlink消息的skb
     * 目的单播地址
     * 目的组播地址
     */
    static inline int nlmsg_multicast(struct sock *sk, struct sk_buff *skb,u32 portid, unsigned int group, gfp_t flags) 
    {
        NETLINK_CB(skb).dst_group = group;
        netlink_broadcast(sk, skb, portid, group, flags);
    }
