目前为止，netlink协议族支持32种(MAX_LINKS)协议类型，其中已经被预定义的有22种。
在实际项目中，如果有定制化的需求时，最好不要去占用剩下的暂未定义的协议类型ID号，而是使用预定义的通用netlink协议类型NETLINK_GENERIC来进行扩展。
LINUX中跟netlink相关的核心代码位于net/netlink目录中，其中核心头文件主要有3个(这些都是所有协议类型的netllink共享的)：
        [1]. net/netlink/af_netlink.h   - 这个头文件主要包含了实现netlink的核心数据结构
        [2]. include/linux/netlink.h    - 这个头文件主要包含了netlink套接字相关的内容
        [3]. include/net/netlink.h      - 这个头文件主要包含了netlink消息相关的通用格式
各种协议类型的netlink代码位于其他相关目录(比如最常见的NETLINK_ROUTE相关代码在net/core/rtnetlink.c中)
注意：以下所有的变量和函数注解基本都没有区分不同的net命名空间，所以在考虑net命名空间时以下表述存在不准确的地方!

1. netlink模块涉及的主要结构和变量
    /* 以下是一张全局的netlink接口总表，是netlink模块的核心变量.
     * 表中包含了MAX_LINKS个元素，每个元素对应一种netlink协议，如NETLINK_ROUTE
     */
    struct netlink_table *nl_table;     

    /* 以下是nl_table每个表项的数据结构，是netlink模块的核心数据结构
     */
    struct netlink_table {
        struct nl_portid_hash   hash;       // hash表控制块，内部的hash表记录了已经创建的同种协议类型的所有netlink套接字
        struct hlist_head   mc_list;        // 这个hash表头节点用于记录同种协议类型下所有阅订了组播功能的套接字
        struct listeners __rcu  *listeners; // 记录了同种协议类型下所有被阅订了的组播消息集合
        unsigned int        flags;          // 这里的标志位来自配置netlink_kernel_cfg,目前主要记录了该协议类型允许的用户态操作权限
        unsigned int        groups;         // 记录了该协议类型支持的最大组播数量(通常就是32个)
        struct mutex        *cb_mutex;      // 记录了该协议类型创建的锁
        struct module       *module;
        void            (*bind)(int group); // 该协议类型私有的bind函数
        bool            (*compare)(struct net *net, struct sock *sock); // 该协议私有的net命名空间比较函数
        int         registered;             // 标记该协议类型是否已经注册，0表示未注册，>=1表示已经有注册过
    };

    /* 以下是netlink套接字的数据结构，所有的协议类型通用
     */
    struct netlink_sock {
        struct sock     sk;     // 该netlink套接字的sock结构
        u32         portid;     // 记录了该netlink套接字绑定的单播地址，对内核来说就是0
        u32         dst_portid;
        u32         dst_group; 
        u32         flags;      // 用来标识该netlink套接字的属性，比如NETLINK_KERNEL_SOCKE
        u32         subscriptions;  // 记录该netlink套接字当前阅订的组播数量
        u32         ngroups;        // 记录该netlink套接字支持的最大组播数量
        unsigned long       *groups;// 指向该netlink套接字的组播空间
        unsigned long       state;
        wait_queue_head_t   wait;
        bool            cb_running; // 用来标志该netlink套接字是否处于dump操作中
        struct netlink_callback cb; // 用来记录该netlink套接字当前有效的操作集合
        struct mutex        *cb_mutex;      // 这把锁在内核netlink套接字创建时传入，相同协议类型的netlink套接字共用一把锁
        struct mutex        cb_def_mutex;
        void            (*netlink_rcv)(struct sk_buff *skb);    // 指向所属的某个netlink协议私有的消息接收回调函数(input)
        void            (*netlink_bind)(int group);             // 指向某个netlink协议私有的bind操作(如果未指定，就采用netlink通用策略)
        struct module       *module; 
    };

    /* 以下是内核创建具体协议类型的netlink套接字时传入的参数配置数据结构
     */
    struct netlink_kernel_cfg {
        unsigned int    groups; // 该协议类型支持的最大多播组数量
        unsigned int    flags;  // 用来设置NL_CFG_F_NONROOT_SEND/NL_CFG_F_NONROOT_RECV这两个标志
        void        (*input)(struct sk_buff *skb);  // 用来配置消息接收函数，用户空间发送该协议类型的netlink消息给内核后，就会调用本函数
        struct mutex    *cb_mutex;      // 用来配置协议类型私有的互斥锁
        void        (*bind)(int group); // 用来配置协议类型私有的bind回调函数
        bool        (*compare)(struct net *net, struct sock *sk);   // 用来配置协议类型私有的compare回调函数
    };

2. 内核netlink模块初始化
    
    /* 以下是整个netlink模块的初始化入口
     *
     * 备注：内核在这里默认创建了两种协议类型的netlink：NETLINK_USERSOCK和NETLINK_ROUTE
     */
    static int __init netlink_proto_init(void)
    {
        // 向内核的proto_list链表注册所有netlink协议共有的netlink_proto，至此所有从socket层下来的netlink消息都可以通过该接口到达相应的传输层
        proto_register(&netlink_proto, 0);            
        // 创建了一张MAX_LINKS长度的nl_table表，这张表是整个netlink功能模块的核心，每种协议类型占一个表项
        nl_table = kcalloc(MAX_LINKS, sizeof(*nl_table), GFP_KERNEL);
        if (totalram_pages >= (128 * 1024))
            limit = totalram_pages >> (21 - PAGE_SHIFT);
        else
            limit = totalram_pages >> (23 - PAGE_SHIFT);

        order = get_bitmask_order(limit) - 1 + PAGE_SHIFT;
        limit = (1UL << order) / sizeof(struct hlist_head);
        order = get_bitmask_order(min(limit, (unsigned long)UINT_MAX)) - 1;

        // 给nl_table表每个表项申请一张hash表并且注册缺省的net命名空间比较函数
        for (i = 0; i < MAX_LINKS; i++)
        {
            struct nl_portid_hash *hash = &nl_table[i].hash;
            // 每张hash表默认都会创建1个初始bucket，实际也就是存放一个链表头指针
            hash->table = nl_portid_hash_zalloc(1 * sizeof(*hash->table));
            hash->max_shift = order;
            hash->shift = 0;
            hash->mask = 0;
            hash->rehash_time = jiffies;
            nl_table[i].compare = netlink_compare;
        }

        // 初始化netlink_tap_all链表头
        INIT_LIST_HEAD(&netlink_tap_all);

        // 创建并注册NETLINK_USERSOCK协议到nl_table中
        netlink_add_usersock_entry();

        /*  注册netlink协议族操作集合到内核中(主要包含netlink接口创建函数)
         *  后续，应用层创建netlink类型的socket时就会调用这里注册的create函数
         */
        sock_register(&netlink_family_ops);

        /* 将netlink模块注册到每一个网络命名空间，并且执行了netlink_net_init
         * 需要注意的是，由于netlink_net_ops.size = 0，意味着netlink模块没有私有空间
         */
        register_pernet_subsys(&netlink_net_ops);

        // 创建并注册NETLINK_ROUTE协议到nl_table中
        rtnetlink_init();
    }

    /* 以下就是具体的netlink功能初始化(实际就是创建proc文件系统下的netlink接口)
     */
    static int __net_init netlink_net_init(struct net *net)
    {
#ifdef CONFIG_PROC_FS
        // proc文件系统下创建/proc/net/netlink文件(文件属性:普通文件 + 只读)
        proc_create("netlink", 0, net->proc_net, &netlink_seq_fops)
#endif
    }

    小结：内核模块的上电初始化是分优先级的，netlink模块作为其他模块的依赖模块，需要确保优先完成初始化（通过core_initcall宏）
          netlink模块本身只提供了一个协议无关的通用平台，实际应用时需要结合具体的协议类型才能完成通信。
          以下的分析都是基于NETLINK_ROUTE协议来完成的。


3. 内核创建基于具体协议类型的netlink套接字
   针对每种netlink协议类型，内核通常(但不绝对，比如NETLINK_USERSOCK)会创建2类netlink套接字：
            一类是内核主动创建的完全属于内核的netlink套接字，入口函数就是netlink_kernel_create;
            另一类是由用户进程调用socket()系统调用，从而在内核中创建的属于用户进程的netlink套接字，入口函数就是netlink_create(这个入口其实不完整，只包含了其中的sock结构的创建).
   基于netlink的用户空间和内核空间交互，实质就是用户进程netlink套接字和对应内核netlink套接字的交互！

    /* 以下就是创建属于内核的具体协议的netlink套接字(可以看出只是个封装函数)
     *
     * 备注：只要是跟用户态交互的netlink协议，就需要在初始化时调用本函数，以创建一个属于内核的netlink套接字
     *       之后，只要用户态发送了一个该协议类型的netlink消息到内核，就会执行本函数传入的input回调函数
     */
    static inline struct sock *netlink_kernel_create(struct net *net, int unit, struct netlink_kernel_cfg *cfg)
    {
        return __netlink_kernel_create(net, unit, THIS_MODULE, cfg);
    }    
    
    /* 以下就是创建属于内核的具体协议的netlink套接字(这个才是真正的执行函数)
     */
    struct sock *__netlink_kernel_create(struct net *net, int unit, struct module *module,struct netlink_kernel_cfg *cfg)
    {
        // 首先是创建并初始化一个协议类型相关的socket结构
        sock_create_lite(PF_NETLINK, SOCK_DGRAM, unit, &sock);
        
        /* 然后是创建并初始化一个协议类型相关的sock结构
         * 备注： 这里要注意的是，首先使用了init_net缺省网络命名空间来创建sock结构，然后再转回到当前的网络命名空间
         * 这么做的原因大概是无法对当前的网络命名空间执行get_net()操作(__netlink_create->sk_alloc中有调用到)
         */ 
        __netlink_create(&init_net, sock, cb_mutex, unit);
        sk = sock->sk;
        sk_change_net(sk, net);

        // 这里可以看出，内核默认支持最少32个组播地址
        if (!cfg || cfg->groups < 32)
            groups = 32;
        else
            groups = cfg->groups;

        // 分配linsters的空间
        listeners = kzalloc(sizeof(*listeners) + NLGRPSZ(groups), GFP_KERNEL);

        // 如果某个netlink协议配置了私有的消息处理函数，就将其注册到netlink套接字的对应位置
        if (cfg && cfg->input)
            nlk_sk(sk)->netlink_rcv = cfg->input;

        /* 将创建的netlink套接字添加到nl_table对应表项中的hash表中 
         * 由于本函数创建的都是内核nelink套接字，所以portid 固定为 0
         */
        netlink_insert(sk, net, 0);

        // 标记这是个内核主动创建的netlink套接字
        nlk = nlk_sk(sk);
        nlk->flags |= NETLINK_KERNEL_SOCKET;

        /* 判断该协议类型是否已经注册过了，如果没有则在这里初始化nl_table对应的表项，如果有就不再初始化该表项了
         *
         * 个人的初步猜测：基于不同的net命名空间，相同协议类型的内核netlink套接字可以对应创建多个
         */
        if (!nl_table[unit].registered)
        {
            nl_table[unit].groups = groups;
            rcu_assign_pointer(nl_table[unit].listeners, listeners);
            nl_table[unit].cb_mutex = cb_mutex;
            nl_table[unit].module = module;
            if (cfg)
            {
                nl_table[unit].bind = cfg->bind;
                nl_table[unit].flags = cfg->flags; 
                if (cfg->compare)
                    nl_table[unit].compare = cfg->compare; 
            }
            nl_table[unit].registered = 1;
        }
        else
        {
            // 多个相同协议类型的内核netlink套接字为何在这里取消listeners空间?
            kfree(listeners);
            nl_table[unit].registered++;
        }

        /* 至此，属于内核的跟协议类型相关的netlink套接字创建完成
         */
    }

    /* 以下就是用户进程调用socket()创建PF_NETLINK类型的套接字时，由内核创建属于用户进程的netlink套接字的过程
     */
    static int netlink_create(struct net *net, struct socket *sock, int protocol,int kern)
    {
        // 首先将套接字状态标记为未连接
        sock->state = SS_UNCONNECTED;

        // netlink接口只支持SOCK_RAW和SOCK_DGRAM这两种套接字类型
        if (sock->type != SOCK_RAW && sock->type != SOCK_DGRAM)
            return -ESOCKTNOSUPPORT;

        // 检查协议类型是否有效
        if (protocol < 0 || protocol >= MAX_LINKS)
            return -EPROTONOSUPPORT; 
           
        // 检查该协议类型的netlink是否已经注册了，只有nl_table中已经注册的协议类型才能继续创建下去
        if (nl_table[protocol].registered && try_module_get(nl_table[protocol].module))
            module = nl_table[protocol].module;
        else
            err = -EPROTONOSUPPORT;

        // 取出内核netlink创建时注册的私有锁和bind函数，接下来就要用到了
        cb_mutex = nl_table[protocol].cb_mutex;
        bind = nl_table[protocol].bind;

        /* 创建并初始化一个协议类型相关的sock结构
         * 
         * 备注：跟创建属于内核的netlink套接字时不同的是，这里似乎不再关心传入的net命名空间
         */
        __netlink_create(net, sock, cb_mutex, protocol);

        sock_prot_inuse_add(net, &netlink_proto, 1);

        // 最后对netlink套接字相关参数进行赋值
        nlk = nlk_sk(sock->sk); 
        nlk->module = module; 
        nlk->netlink_bind = bind;  

        /* 至此，属于用户进程的跟协议类型相关的netlink套接字创建完成
         */
    }

    小结： 以上就是内核创建2类netlink套接字的方法，需要注意的一点就是，用户进程netlink套接字的创建需要依赖对应的内核netlink套接字;
           另外，以上2类netlink套接字的创建过程中都调用了__netlink_create这个函数来执行核心的创建过程

    /* 以下就是创建并初始化一个netlink_sock结构(其中包含了sock结构) 
     */
    static int __netlink_create(struct net *net, struct socket *sock,struct mutex *cb_mutex, int protocol)
    {
        // netlink套接字和socket层netlink协议族的通用操作集合关联
        sock->ops = &netlink_ops;

        // 分配基于netlink协议的sock结构
        sk = sk_alloc(net, PF_NETLINK, GFP_KERNEL, &netlink_proto);

        // 初始化sock的发送接收队列、数据缓存、等待队列和互斥锁等
        sock_init_data(sock, sk);
        nlk = nlk_sk(sk);
        if (cb_mutex)
            nlk->cb_mutex = cb_mutex;
        else
        {
            nlk->cb_mutex = &nlk->cb_def_mutex;
            mutex_init(nlk->cb_mutex);
        }
        init_waitqueue_head(&nlk->wait);

        sk->sk_destruct = netlink_sock_destruct;    // 设置netlink sock结构的析构函数
        sk->sk_protocol = protocol;
    }

4. 绑定netlink套接字
   由于内核netlink套接字在创建时就固定绑在了portid=0的位置，并且内核套接字目前没有反向监听组播的功能，所以以下的绑定操作就是针对用户进程netlink套接字。
    
    /* 以下就是用户进程对netlink套接字调用bind()系统调用后，内核会执行的一部分操作
     * @sock    - 要绑定的socket结构，也可以认为是netlink套接字
     * @addr    - 要绑定的套接字地址
     * @addr_len- 套接字地址长度
     *
     * 备注： netlink套接字在创建的过程中(具体是在__netlink_create函数开头)，已经和netlink_ops(socket层netlink协议族的通用操作集合)关联,
     *        其中注册的bind回调就是指向本函数
     */
    static int netlink_bind(struct socket *sock, struct sockaddr *addr,int addr_len)
    {
        /* 如果传入的套接字地址中指定了要监听的组播，需要判断该套接字是否具有监听组播的权限
         */
        if (nladdr->nl_groups)
        {
            if (!netlink_allowed(sock, NL_CFG_F_NONROOT_RECV))
                return -EPERM;
            // 为该套接字分配组播空间
            netlink_realloc_groups(sk);
        }

        /* 需要注意的一点就是，如果netlink套接字已经绑定在一个地址上，就不能再绑到一个新的地址. 
         *      对于尚未绑定的netlink套接字，这时候如果传入的套接字地址中指定了要绑定的单播地址就用该地址绑定; 
         *      如果没有指定就自动绑定一个
         */
        if (nlk->portid)
        {
            if (nladdr->nl_pid != nlk->portid) 
                return -EINVAL;
        }
        else
        {
            // 绑定netlink套接字的过程，其实就是将其加入nl_table的过程
            err = nladdr->nl_pid ? netlink_insert(sk, net, nladdr->nl_pid) :netlink_autobind(sock);
        }

        // 如果用户态没有指定组播地址，并且没有分配组播的内存，那么bind工作就到此结束了
        if (!nladdr->nl_groups && (nlk->groups == NULL || !(u32)nlk->groups[0])) 
            return 0;

        // 程序运行到这里意味着用户态指定了需要绑定的组播地址
        // 将指定套接字加入所属netlink协议类型的多播hash链表
        netlink_update_subscriptions(sk, nlk->subscriptions +hweight32(nladdr->nl_groups) -hweight32(nlk->groups[0]));

        // 保存组播地址mask值到开头申请的组播空间
        nlk->groups[0] = (nlk->groups[0] & ~0xffffffffUL) | nladdr->nl_groups;
        // 更新套接字所属协议类型的监听集合
        netlink_update_listeners(sk);

        // 如果具体的netlink协议类型注册了私有的bind函数，并且用户态指定了要绑定的组播地址，则在这里调用该私有的bind函数
        if (nlk->netlink_bind && nlk->groups[0])
        {
            for (i=0; i<nlk->ngroups; i++)
                if (test_bit(i, nlk->groups))
                    nlk->netlink_bind(i);
        }
    }
