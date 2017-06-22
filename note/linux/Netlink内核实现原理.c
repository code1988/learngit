目前为止，netlink协议族支持32种(MAX_LINKS)协议类型，其中已经被预定义的有22种。
在实际项目中，如果有定制化的需求时，最好不要去占用剩下的暂未定义的协议类型ID号，而是使用预定义的通用netlink协议类型NETLINK_GENERIC来进行扩展。
LINUX中跟netlink相关的核心代码位于net/netlink目录中，其中核心头文件主要有3个(这些都是所有协议类型的netllink共享的)：
        [1]. net/netlink/af_netlink.h   - 这个头文件主要包含了实现netlink的核心数据结构
        [2]. include/linux/netlink.h    - 这个头文件主要包含了netlink套接字相关的内容
        [3]. include/net/netlink.h      - 这个头文件主要包含了netlink消息相关的通用格式
各种协议类型的netlink代码位于其他相关目录(比如最常见的NETLINK_ROUTE相关代码在net/core/rtnetlink.c中)

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
        u32         flags;
        u32         subscriptions;  // 记录该netlink套接字当前阅订的组播数量
        u32         ngroups;        // 记录该netlink套接字支持的最大组播数量
        unsigned long       *groups;// 指向该netlink套接字的组播空间
        unsigned long       state;
        wait_queue_head_t   wait;
        bool            cb_running; // 用来标志该netlink套接字是否处于dump操作中
        struct netlink_callback cb; // 用来记录该netlink套接字当前有效的操作集合
        struct mutex        *cb_mutex; 
        struct mutex        cb_def_mutex;
        void            (*netlink_rcv)(struct sk_buff *skb);    // 指向所属的某个netlink协议的input回调函数
        void            (*netlink_bind)(int group);             // 指向某个netlink协议自身特有的bind操作(如果未指定，就采用netlink通用策略)
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
    }

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
    /* 以下就是创建属于内核的具体协议的netlink套接字
     *
     * 备注：只要是跟用户态交互的netlink协议，就需要在初始化时调用本函数，以创建一个属于内核的netlink套接字
     *       之后，只要用户态发送了一个该协议类型的netlink消息到内核，就会执行本函数传入的input回调函数
     */
    static inline struct sock *netlink_kernel_create(struct net *net, int unit, struct netlink_kernel_cfg *cfg)
    {
        return __netlink_kernel_create(net, unit, THIS_MODULE, cfg);
    }    
    
