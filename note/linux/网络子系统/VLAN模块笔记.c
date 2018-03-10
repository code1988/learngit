一. VLAN的核心概念
    1. 划分VLAN的核心目的只有一个：分割广播域。
       通过VLAN对广播域进行合理分割之后，一是可以缩小ARP攻击的范围，从而提高网络的安全性；二是可以缩小广播域的大小，从而提高网络的性能。

       所以要注意的是，划分VLAN的目的中根本没有隔离不同VLAN用户互访这一说法，这只是划分VLAN之后的一种应用，不然使用三层设备实现不同VLAN
       间互访就成了多此一举。

    2. 单独的一个VLAN模拟了一个常规的以太网交换机，因此VLAN实际上就是将一台物理交换机分割成了多台逻辑交换机，变相也节省了物理设备。
       站在二层的角度，不同VLAN间是无法通信的，VLAN间想要进行通信必须有三层参与。 
       
    3. 本地VLAN的实现方式有很多种，常见的有基于端口的VLAN、基于MAC地址的VLAN、基于IP地址的VLAN、基于用户的VLAN等，这里略过。
    
    4. 跨设备的VLAN，标准的实现方法主要就是802.1q，通过VID来识别不同的VLAN，本文主要就是记录了802.1q的相关内容。

    5. 802.1q的应用原则：通常在上行链路的第一台VLAN交换机打上tag，在下行链路的最后一台VLAN交换机去掉tag；
                         基于802.1q的VLAN划分既要合理，又要尽量简单，原则就是只有当一个数据帧不打tag就不能区分属于哪个VLAN时才打上tag，
                         能去掉时尽早去掉。


二. Linux中VLAN的实现原理
    Linux中跟VLAN相关的代码主要位于net/8021q目录中。
    Linux中的VLAN是一种特殊的虚拟设备，所有的VLAN设备必须依赖于它的宿主设备才能存在。

    1. VLAN模块涉及的主要结构

        /* 以下定义了整个VLAN模块公用的私有空间(当然公用的前提是同一个网络命名空间下)
         * 具体就是记录了有关VLAN的proc文件系统信息
         */
        struct vlan_net {
            struct proc_dir_entry *proc_vlan_dir;   // proc文件系统中VLAN的顶层目录节点
            struct proc_dir_entry *proc_vlan_conf;  // proc文件系统中VLAN的配置文件节点
            unsigned short name_type;               // vlan设备名字显示风格，默认就是eth0.10的风格
        }

        /* ioctl调用SIOCGIFVLAN / SIOCSIFVLAN这两条命令时传入的参数结构
         * 用户空间和kernel中都使用到本结构
         */
        struct vlan_ioctl_args {
            int cmd;    // 具体vlan命令
            char device1[24];       /* 用户传入的设备名，用于内核查找实际对应的设备
                                       调用ADD_VLAN_CMD时，该参数传入的是vlan的宿主设备名
                                       调用SET_VLAN_NAME_TYPE_CMD时，该参数不用填
                                       调用其他命令时，该参数传入的是vlan设备名
                                    */ 
            union {
                char device2[24];   // 用于返回vlan设备名
                int VID;
                unsigned int skb_priority;
                unsigned int name_type;
                unsigned int bind_type;
                unsigned int flag;  // 注意点，想要开启某功能时需要同时设置vlan_qos为非0
            }u;
            short vlan_qos;
        }

        /* 定义了每个VLAN设备附属的私有空间结构
         * 每个VLAN设备都有一个私有空间vlan_dev_priv，同时整个VLAN模块的私有空间vlan_net是所有VLAN设备共有的
         */
        struct vlan_dev_priv {
            unsigned int                nr_ingress_mappings; 
            u32					        ingress_priority_map[8];
            unsigned int				nr_egress_mappings;
            struct vlan_priority_tci_mapping	*egress_priority_map[16];

            __be16					vlan_proto; // vlan协议类型(大端)
            u16					    vlan_id;    // vlan id
            u16					    flags;      // VLAN_FLAG_* 用来记录该vlan设备的几个特征标志，默认总是设置了REORDER_HDR

            struct net_device			*real_dev;                  // 指向宿主设备
            unsigned char				real_dev_addr[ETH_ALEN];    // 宿主设备mac

            struct proc_dir_entry			    *dent;
            struct vlan_pcpu_stats __percpu		*vlan_pcpu_stats;
        #ifdef CONFIG_NET_POLL_CONTROLLER
            struct netpoll				*netpoll;
        #endif
            unsigned int				nest_level;
        }

        /* 定义一个记录vlan设备信息的结构，保存了一个宿主设备上绑定的所有vlan设备的信息
         * 这个结构是宿主设备用来管理其下辖的所有vlan设备的
         */
        struct vlan_info {
            struct net_device   *real_dev;  // 指向宿主设备
            struct vlan_group   grp;        // vlan组，记录了所有绑定在该宿主设备上的vlan设备
            struct list_head    vid_list;   // vlan id的链表头
            unsigned int        nr_vids;    // 记录了该vlan id链表中节点数量
            struct rcu_head     rcu;
        }

        /* 定义同一个宿主设备下的vlan组模型
         * 存储的vlan设备呈现四维结构：VLAN_PROTO_NUM * VLAN_GROUP_ARRAY_SPLIT_PARTS * VLAN_GROUP_ARRAY_PART_LEN * struct net_device指针
         */
        struct vlan_group {
            unsigned int		nr_vlan_devs;   // 记录了该vlan组中包含的vlan设备数量
            struct hlist_node	hlist;	
            struct net_device **vlan_devices_arrays[VLAN_PROTO_NUM]
                                   [VLAN_GROUP_ARRAY_SPLIT_PARTS];  // 记录了该vlan组中包含的所有vlan设备
        }

    2. VLAN模块的初始化
        int vlan_net_id;  // 用于记录vlan模块在所有网络命名空间中的ID号（注册到网络命名空间时分配）

        // 以下是整个VLAN模块的初始化入口
        int __init vlan_proto_init(void)
        {
            // 将vlan模块注册到每一个网络命名空间，注意就是在这里完成了模块私有空间的申请，对于VLAN来说就是vlan_net，并且执行了vlan_init_net
            register_pernet_subsys(&vlan_net_ops);
            // 向通知链中注册一个vlan事件通知块
            register_netdevice_notifier(&vlan_notifier_block);
            // 初始化操作vlan用的netlink接口
            vlan_netlink_init();
            // 设置用于vlan操作的ioctl钩子函数
            vlan_ioctl_set(vlan_ioctl_handler);
        }
        小结：以上4步操作基本是加载一个顶层网络模块的通用格式，
              而模块特有的那部分初始化通常就是放在pernet_operations->init指向的函数中，以VLAN为例，就是vlan_init_net

        // 以下就是VLAN模块特有的那部分初始化
        int __net_init vlan_init_net(struct net *net)
        {
            // 根据vlan_net_id索引对应的私有空间，也就是vlan_net
            struct vlan_net *vn = net_generic(net, vlan_net_id);
            // 设置vlan设备名的默认显示风格，类似 eth0.10
            vn->name_type = VLAN_NAME_TYPE_RAW_PLUS_VID_NO_PAD;
            // 在proc文件系统中创建vlan接口(/proc/net/vlan)
            vlan_proc_init(net);
        }
        int __net_init vlan_proc_init(struct net *net)
        {
            // 根据vlan_net_id索引得到对应的vlan_net结构
            struct vlan_net *vn = net_generic(net, vlan_net_id);
            // proc文件系统下创建/proc/net/vlan目录
            vn->proc_vlan_dir = proc_net_mkdir(net, name_root, net->proc_net);
            // proc文件系统下创建/proc/net/vlan/config文件(文件属性：普通文件 + 可读可写)
            vn->proc_vlan_conf = proc_create(name_conf, S_IFREG|S_IRUSR|S_IWUSR,vn->proc_vlan_dir, &vlan_fops); 
        }
        小结：这部分VLAN特有的初始化，具体主要就是创建了proc文件系统下的VLAN节点

    3. VLAN模块的功能管理
        对VLAN模块的功能管理主要基于2种接口方式：ioctl和netlink 

        3.1 ioctl方式分析
            VLAN模块初始化过程中注册了vlan_ioctl_handler到对应的ioctl钩子vlan_ioctl_hook，
            这样，当用户空间执行ioctl调用SIOCGIFVLAN/SIOCSIFVLAN这两条命令时，vlan_ioctl_handler将被执行
            int vlan_ioctl_handler(struct net *net, void __user *arg)
            {
                // 将ioctl的vlan传入参数从用户空间拷贝到内核
                copy_from_user(&args, arg, sizeof(struct vlan_ioctl_args));
                switch (args.cmd)
                {
                    case SET_VLAN_INGRESS_PRIORITY_CMD:
                    case SET_VLAN_EGRESS_PRIORITY_CMD:
                    case SET_VLAN_FLAG_CMD:
                    case ADD_VLAN_CMD:
                    case DEL_VLAN_CMD:
                    case GET_VLAN_REALDEV_NAME_CMD:
                    case GET_VLAN_VID_CMD:
                        // 根据用户传入的设备名查找对应的设备管理块
                        dev = __dev_get_by_name(net, args.device1);
                        // 以上这些命令中除了ADD_VLAN_CMD，必须确保device1传入的设备名指的是vlan设备
                        if (args.cmd != ADD_VLAN_CMD && !is_vlan_dev(dev))
                            goto out;
                }       
                switch (args.cmd)
                {
                    case SET_VLAN_INGRESS_PRIORITY_CMD: // 暂略
                        break;
                    case SET_VLAN_EGRESS_PRIORITY_CMD:  // 暂略
                        break;
                    case SET_VLAN_FLAG_CMD:
                        // 设置vlan标志位命令的用户需要进行管理员级别权限测试
                        if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
                            break;
                        // 如果传入的vlan设备名风格有效则更新
                        if ((args.u.name_type >= 0) &&(args.u.name_type < VLAN_NAME_TYPE_HIGHEST))
                        {
                            struct vlan_net *vn;
                            vn = net_generic(net, vlan_net_id);
                            vn->name_type = args.u.name_type;
                        }
                        break;
                    case ADD_VLAN_CMD:
                        // 添加vlan设备的用户需要进行管理员级别权限测试
                        if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
                            break;
                        // 在宿主设备上注册一个新的vlan设备
                        register_vlan_device(dev, args.u.VID);
                        break;
                    case DEL_VLAN_CMD:
                        // 删除vlan设备的用户需要进行管理员级别权限测试
                        if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
                            break;
                        // 注销vlan设备
                        unregister_vlan_dev(dev, NULL);
                        break;
                    case GET_VLAN_REALDEV_NAME_CMD: 
                        // 获取vlan设备对应的宿主设备名
                        vlan_dev_get_realdev_name(dev, args.u.device2);
                        // 拷贝回用户空间
                        copy_to_user(arg, &args,sizeof(struct vlan_ioctl_args));
                        break;
                    case GET_VLAN_VID_CMD:
                        // 获取该vlan设备的vid
                        args.u.VID = vlan_dev_vlan_id(dev);
                        // 拷贝回用户空间
                        copy_to_user(arg, &args,sizeof(struct vlan_ioctl_args));
                        break;
                }
            }
            
            3.1.1 VLAN设备的创建
                /* 在宿主设备上注册一个新的VLAN设备
                 * @real_dev - 宿主设备(一般就是真实的网卡，但对交换机来说，可能是DSA中创建的虚拟端口)
                 * @vlan_id  - 要创建的VLAN设备的ID
                 */
                int register_vlan_device(struct net_device *real_dev, u16 vlan_id)
                {
                    // 首先获取宿主设备所在的网络命名空间
                    struct net *net = dev_net(real_dev); 
                    // 然后根据vlan_net_id在索引得到对应的私有空间，也就是vlan_net
                    struct vlan_net *vn = net_generic(net, vlan_net_id);
                    // vlan id号不能超过4096
                    if (vlan_id >= VLAN_VID_MASK)
                        return -ERANGE;
                    // 检查宿主设备是否支持vlan协议以及要创建的vlan id在该设备上是否已经存在
                    vlan_check_real_dev(real_dev, htons(ETH_P_8021Q), vlan_id);

                    // 根据vlan设备不同的显示风格生成相应的vlan设备名
                    switch(vn->name_type)
                    {
                        case VLAN_NAME_TYPE_RAW_PLUS_VID_NO_PAD:
                            snprintf(name, IFNAMSIZ, "%s.%i", real_dev->name, vlan_id);
                            break;
                        ...
                    }

                    // 创建一个vlan设备，并执行vlan_setup完成基本的初始化
                    new_dev = alloc_netdev(sizeof(struct vlan_dev_priv), name, vlan_setup); 
                    // 将新创建的vlan设备关联到当前的网络命名空间
                    dev_net_set(new_dev, net);
                    new_dev->mtu = real_dev->mtu;   // 将宿主设备的mtu继承到vlan设备
                    new_dev->priv_flags |= (real_dev->priv_flags & IFF_UNICAST_FLT);    // 如果宿主设备支持单播过滤功能，则同样继承到vlan设备

                    // 初始化vlan设备附属的私有空间vlan_dev_priv
                    vlan = vlan_dev_priv(new_dev);
                    vlan->vlan_proto = htons(ETH_P_8021Q);  
                    vlan->vlan_id = vlan_id;
                    vlan->real_dev = real_dev;
                    vlan->dent = NULL;
                    vlan->flags = VLAN_FLAG_REORDER_HDR;

                    new_dev->rtnl_link_ops = &vlan_link_ops;// 注册netlink接口操作集合用于管理该vlan设备

                    // 进一步注册该vlan设备
                    register_vlan_dev(new_dev);
                }

                /* 每个新创建的vlan设备初始化回调函数
                 */
                void vlan_setup(struct net_device *dev)
                {
                    // 为vlan设备设置一些链路层基本参数
                    ether_setup(dev);
                    dev->priv_flags     |= IFF_802_1Q_VLAN; // 表明这是一个vlan设备
                    dev->priv_flags     &= ~(IFF_XMIT_DST_RELEASE | IFF_TX_SKB_SHARING);
                    dev->tx_queue_len   = 0;

                    dev->netdev_ops		= &vlan_netdev_ops;     // 注册vlan设备管理操作回调函数集
                    dev->destructor		= vlan_dev_free;        // 注册vlan设备注销回调函数
                    dev->ethtool_ops	= &vlan_ethtool_ops;    // 注册使用ethtool工具操作vlan设备的回调函数集

                    memset(dev->broadcast, 0, ETH_ALEN);        // 清零广播mac
                }

                /* 进一步注册该vlan设备
                 */
                int register_vlan_dev(struct net_device *dev)
                {
                    struct vlan_dev_priv *vlan = vlan_dev_priv(dev);    // 获取vlan设备附属的私有空间
                    struct net_device *real_dev = vlan->real_dev;       // 获取宿主设备
                    u16 vlan_id = vlan->vlan_id;    // 获取vlan id
                    
                    // 将指定vlan协议ID的vlan id添加到宿主设备的vlan_info管理块中
                    vlan_vid_add(real_dev, vlan->vlan_proto, vlan_id);
                    // 确保程序运行到这里时宿主设备的vlan_info成员不再为空        
                    vlan_info = rtnl_dereference(real_dev->vlan_info);
                    // 对四维结构的vlan组模型在第三维度上进行预分配
                    vlan_group_prealloc_vid(grp, vlan->vlan_proto, vlan_id);
                    // 设置该vlan设备的嵌套级别
                    vlan->nest_level = dev_get_nest_level(real_dev, is_vlan_dev) + 1;
                    // 注册该网络设备到内核中，注册结果会从通知链中反馈
                    register_netdevice(dev);
                    // 将vlan设备加入宿主设备的upper邻接链表中
                    netdev_upper_dev_link(real_dev, dev);
                    // 宿主设备的引用计数加1
                    dev_hold(real_dev); 
                    // 从宿主设备继承一些状态(比如设备的链路状态) 
                    netif_stacked_transfer_operstate(real_dev, dev);
                    // 将vlan设备加入lweventlist通知链
                    linkwatch_fire_event(dev);
                    // 最后一步，将指定的vlan设备记录到四维vlan组模型中
                    vlan_group_set_device(grp, vlan->vlan_proto, vlan_id, dev);
                    grp->nr_vlan_devs++;
                }

        3.2 netlink方式分析
            VLAN模块初始化过程中还通过vlan_netlink_init注册了一组netlink接口vlan_link_ops用于操作VLAN模块

            3.2.1 VLAN设备的创建
                /* 注册并配置一个新的vlan设备
                 * @src_net     - 所处的网络命名空间
                 * @dev         - 新创建的网络设备
                 * @tb[]        - ?
                 * @data[]      - ?
                 * 
                 * 备注：对应ioctl接口的函数register_vlan_device，主要的区别在于调用本函数前vlan设备管理块已经被创建
                 */
                int vlan_newlink(struct net *src_net, struct net_device *dev,struct nlattr *tb[], struct nlattr *data[])
                {
                    struct vlan_dev_priv *vlan = vlan_dev_priv(dev);
                    // 检查是否有传入IFLA_VLAN_ID属性
                    if (!data[IFLA_VLAN_ID])
                        return -EINVAL;
                    // 检查是否有传入IFLA_LINK属性
                    if (!tb[IFLA_LINK])
                        return -EINVAL;

                    // 从IFLA_LINK属性中获取宿主设备接口序号，再根据接口序号索引得到对应的宿主设备
                    real_dev = __dev_get_by_index(src_net, nla_get_u32(tb[IFLA_LINK])); 
                    // 如果传入了IFLA_VLAN_PROTOCOL属性则从中获取vlan的协议类型，否则采用缺省的vlan协议类型
                    if (data[IFLA_VLAN_PROTOCOL])
                        proto = nla_get_be16(data[IFLA_VLAN_PROTOCOL]); 
                    else
                        proto = htons(ETH_P_8021Q);

                    // 初始化vlan设备附属的私有空间vlan_dev_priv
                    vlan->vlan_proto = proto;
                    vlan->vlan_id    = nla_get_u16(data[IFLA_VLAN_ID]);
                    vlan->real_dev   = real_dev;
                    vlan->flags  = VLAN_FLAG_REORDER_HDR;

                    // 检查宿主设备是否支持vlan协议以及要创建的vlan id在该设备上是否已经存在
                    vlan_check_real_dev(real_dev, vlan->vlan_proto, vlan->vlan_id);
                    // 如果没有传入IFLA_MTU属性则从宿主设备继承mtu值，否则意味着使用了自定义的mtu值，这时候需要确保自定义值不大于宿主设备上的mtu值
                    if (!tb[IFLA_MTU])
                        dev->mtu = real_dev->mtu;
                    else if (dev->mtu > real_dev->mtu)
                        return -EINVAL;

                    // 该vlan设备在这里继续处理来自用户空间设置的参数
                    vlan_changelink(dev, tb, data);

                    // 进一步注册该vlan设备
                    return register_vlan_dev(dev);
                }
    
