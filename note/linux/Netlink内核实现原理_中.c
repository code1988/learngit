netlink支持用户进程和内核相互交互（两边都可以主动发起），同时还支持用户进程之间相互交互（虽然这种应用通常都采用unix-sock）
以下将从代码级别对每种交互原理进行分析：

    /* 用户进程对netlink套接字调用sendmsg()系统调用后，内核执行netlink操作的总入口函数
     */
    static int netlink_sendmsg(struct kiocb *kiocb, struct socket *sock,struct msghdr *msg, size_t len)
    {
    
    }

1. 用户进程向内核发送netlink消息
