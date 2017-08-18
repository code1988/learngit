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

        // 获取该netlink消息的目的单播和组播地址
        
    }

