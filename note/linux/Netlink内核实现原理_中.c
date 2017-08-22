netlink支持用户进程和内核相互交互（两边都可以主动发起），同时还支持用户进程之间相互交互（虽然这种应用通常都采用unix-sock）

1. 用户进程向内核或其他用户进程发送netlink消息
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
        if (sk_filter(sk, skb)){
            err = skb->len;
            kfree_skb(skb);
            sock_put(sk);
            return err; 
        }

        err = netlink_attachskb(sk, skb, &timeo, ssk);
        if (err == 1)
            goto retry;
        if (err)
            return err;

        return netlink_sendskb(sk, skb);
    }

    /* 以下是内核执行netlink单播消息发往内核netlink套接字的流程
     * 
     *
     */
    
