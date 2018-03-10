一. Bridge的基本概念
    bridge是一种用于实现TCP/IP二层协议转发的设备，类似现实中的交换机。
    Linux中的bridge是一种特殊的虚拟设备(类似的还有vlan、tap等)，bridge设备通过关联多个从设备，并将从设备虚拟化为端口，从而实现从设备间二层转发以及跟本地协议栈的数据交互。
    Linux中的bridge除了基本的二层转发功能外，还包含了STP、IGMP、ebtables功能(本文主要分析基础功能，这些扩展功能将在其他文档中单独分析)。
    Linux中跟bridge相关的代码主要位于net/bridge目录中。

二. Bridge基础功能涉及的数据结构

三. Bridge模块全局初始化
    /* 定义了bridge模块在整个命名空间的操作集合
     * 显然，bridge模块在proc文件系统中没有专门的节点，并且没有私有空间
     */
    static struct pernet_operations br_net_ops = {
        .exit   = br_net_exit,
    }
    /* 以下是整个bridge模块的初始化入口
     * 
     */
    static int __init br_init(void)
    {
        // 注册生成树协议(stp/rstp/mstp，内核只实现了stp)到LLC层，当收到BPDU时就会调用注册的br_stp_rcv接收钩子
        stp_proto_register(&br_stp_proto);
        // 初始化供网桥使用的二层转发表项的缓存池
        br_fdb_init();
        // 将网桥模块添加到每一个网络命名空间
        register_pernet_subsys(&br_net_ops);
        // 网桥的netfilter功能初始化,用于实现ebtables功能
        br_netfilter_init();
        // 向通知链中注册一个网桥事件通知块
        register_netdevice_notifier(&br_device_notifier);
        // 初始化用于操作网桥的netlink接口
        br_netlink_init();
        // 设置用于操作网桥的ioctl钩子函数
        brioctl_set(br_ioctl_deviceless_stub);
    }

    /* 初始化网桥使用的二层转发表项缓存池，实际就是为其分配一个slab缓存池
     */
    int __init br_fdb_init(void)
    {
        // 创建一片新的slab缓存池，单元长度net_bridge_fdb_entry
        br_fdb_cache = kmem_cache_create("bridge_fdb_cache",
                                         sizeof(struct net_bridge_fdb_entry),
                                         0,
                                         SLAB_HWCACHE_ALIGN, NULL);
        // 生成一个随机数，将用于转发表的hash算法计算
        get_random_bytes(&fdb_salt, sizeof(fdb_salt));
    }
    小结：bridge模块的全局初始化中，基础功能部分的核心就是预先分配了一个slab缓存池，用于后续分配二层转发表项
    
四. Bridge模块的功能管理
    对bridge模块的功能管理主要基于2种接口方式：ioctl和netlink 
    [1]. ioctl方式分析
        bridge模块全局初始化过程中注册了br_ioctl_deviceless_stub到对应的ioctl钩子br_ioctl_hook，
        这样，当用户空间执行ioctl调用SIOCGIFVLAN/SIOCSIFVLAN这两条命令时，br_ioctl_deviceless_stub将被执行
        /* 操作网桥设备的ioctl钩子函数
         * @net     指向当前的网络命名空间
         * @cmd     用户层调用ioctl的命令ID号
         * @uarg    用户层调用ioctl时传入的参数
         */
        int br_ioctl_deviceless_stub(struct net *net, unsigned int cmd, void __user *uarg)
        {
            switch (cmd){
            case SIOCGIFBR:
            case SIOCSIFBR:
                return old_deviceless(net, uarg);
            case SIOCBRADDBR:   // 创建一个网桥
            case SIOCBRDELBR:   // 删除一个网桥
                // 先对该用户命名空间进行CAP_NET_ADMIN权限测试
                if (!ns_capable(net->user_ns, CAP_NET_ADMIN))
                    return -EPERM; 
                // 转储ioctl传入的参数uarg，这里就是桥名
                if (copy_from_user(buf, uarg, IFNAMSIZ))
                    return -EFAULT;
                if (cmd == SIOCBRADDBR) 
                    return br_add_bridge(net, buf); // 如果是SIOCBRADDBR则创建网桥
                return br_del_bridge(net, buf);     // 如果是SIOCBRDELBR则删除网桥
            }
        }
