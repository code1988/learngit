适用于linux 3.14及以后
---------------------------------------------------------------------------------------------------
/usr/include目录下包含了linux环境编程时可以使用的所有头文件，这些头文件大致可以分为以下几类：
    [1]. linux内核支持的API接口，这类头文件通常位于/usr/include/linux目录下
    [2]. 用户态安装的程序通常会在这里生成相关的头文件，比如python2.7的头文件被放在了/usr/include/pythonb2.7目录中
    [3]. 剩下的头文件通常就是glibc提供的API接口
需要注意的就是，这几类头文件中往往定义了一些相同的API，所以用户态编程时需要注意重复定义的问题，一下列出了一些常用的头文件。

#include <linux/if_ether.h> - 该头文件是802.3标准以太网物理层接口的全局定义。(要注意的一点是，该头文件跟 net/ethernet.h 存在冲突，避免同时使用)
                            - 主要定义了一些以太网帧相关的常量；
                            - 以太网帧头struct ethhdr
                            - 以太网帧协议类型ID
                            - 套接字过滤用的ID，比如ETH_P_ALL等

#include <linux/if_packet.h>   - 该头文件用于linux AF_PACKET套接字。(要注意的一点是，该头文件跟 netpacket/packet.h 存在冲突，避免同时使用)
                                - 主要定义了链路层套接字地址 struct sockaddr_ll；
                                - 包类型(用于sll_pkttype)；
                                - setsockopt设置SOL_PACKET层的套接字选项；
                                - setsockopt设置SOL_PACKET层的数据结构struct packet_mreq;
                                - setsockopt设置内容(用于mr_type)

#include <linux/if_vlan.h>  - 该头文件定义了通过ioctl接口操作内核vlan模块的结构和参数

#include <linux/if.h>   - 该头文件用于网络接口的查询，比如ioctl。(要注意的一点是，该头文件跟 net/if.h 存在冲突，避免同时使用)
                        - 主要定义了struct ifreq(用于ioctl)
                        - 接口返回的标准状态

#include <netinet/in.h> - 该头文件是IP层的全局定义(要注意的一点是，不能用 linux/in.h 来替换该头文件，因为netinet/in.h定义了更多相关内容)
                        - 主要定义了IP层套接字地址 struct sockaddr_in;
                        - IP层的协议类型ID
                        - IP层地址结构struct in_addr，常用于IP地址格式转换
                        - setsockopt设置SOL_IP层的套接字选项
                        - setsockopt设置SOL_IP层的数据结构struct ip_mreq;

#include <net/if.h> - 该头文件内容跟 linux/if.h 基本类似，主要的差别在于多定义了几个POSIX标准的API接口(比如if_indextoname等)，所以要根据实际使用情况对这两个头文件二选一

#include <netdb.h>  - 该头文件通常用于获取主机信息
                    - 主要定义了struct hostent (主要用于gethostbyaddr和gethostbyname，存放了一个主机包含的完整信息) 
                    - strcut addrinfo (主要用于getaddrinfo，同样是存放了一个主机包含的完整信息)
                    - 相关的参数定义

#include <sys/socket.h> - 该头文件用于socket编程，定义了所有socket编程相关的API接口

#include <sys/ioctl.h>  - 顾名思义，该头文件用于ioctl接口，凡是需要调用ioctl的地方都需要该头文件

#include <sys/epoll.h>  - 顾名思义，该头文件用于epoll接口，凡是需要调用epoll相关API的地方都需要该头文件


#include <arpa/inet.h>  - 该头文件定义了inet_*系列的API(比如inet_aton)

备注：对于存在冲突的头文件，本人的偏好是尽量使用第一类头文件，只有当涉及第一类头文件中没有支持的API时，才使用其他两类的头文件

