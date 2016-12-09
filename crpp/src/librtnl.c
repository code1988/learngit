#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <net/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "libepoll.h"
#include "librtnl.h"
#include "utils.h"

static void parse_rtattr(struct rtattr **tb,int max,struct rtattr *attr,int len)
{
    for(;RTA_OK(attr,len);attr = RTA_NEXT(attr,len))
    {
        if(attr->rta_type <= max)
            tb[attr->rta_type] = attr;
    }
}

void rtnl_ifinfomsg(struct nlmsghdr *nlh)
{
    struct rtattr *tb[IFLA_MAX + 1];
    struct ifinfomsg *ifinfo = NLMSG_DATA(nlh);

    memset(tb,0,sizeof(tb));

    int len = IFLA_PAYLOAD(nlh);

    parse_rtattr(tb,IFLA_MAX,IFLA_RTA(ifinfo),len);

    LOG_ERROR("link %s ",(ifinfo->ifi_flags & IFF_RUNNING)?"up":"down");
    if(tb[IFLA_IFNAME])
        LOG_ERROR("%s",RTA_DATA(tb[IFLA_IFNAME]));
}

void rtnl_ifaddrmsg(struct nlmsghdr *nlh)
{
    struct rtattr *tb[IFA_MAX + 1];
    struct ifaddrmsg *ifinfo = NLMSG_DATA(nlh);

    memset(tb,0,sizeof(tb));

    int len = IFLA_PAYLOAD(nlh);

    parse_rtattr(tb,IFA_MAX,IFA_RTA(ifinfo),len);

}

void rtnl_rtmsg(struct nlmsghdr *nlh)
{
    struct rtattr *tb[IFA_MAX + 1];
    struct rtmsg *ifinfo = NLMSG_DATA(nlh);

    memset(tb,0,sizeof(tb));

    int len = IFLA_PAYLOAD(nlh);

    parse_rtattr(tb,IFA_MAX,IFA_RTA(ifinfo),len);

}

static void rtnl_recv(struct epoll_handler_s *h,uint32_t events)
{
    int len;
    unsigned char buffer[4096];
    struct nlmsghdr *nlh;

    len = recv(h->fd,buffer,sizeof(buffer),0);
    if(len <= 0)
    {
        LOG_ERROR("recv error");
        close(h->fd);
        return;
    }
    
    for(nlh = (struct nlmsghdr *)buffer;NLMSG_OK(nlh,len);nlh = NLMSG_NEXT(nlh,len))
    {
        //LOG_ERROR("get msg type%d",nlh->nlmsg_type);
        switch(nlh->nlmsg_type)
        {
            case NLMSG_DONE:
            case NLMSG_ERROR:
                LOG_ERROR("get msg %d",nlh->nlmsg_type);
                break;
            case RTM_NEWLINK:
            case RTM_DELLINK:
                rtnl_ifinfomsg(nlh);
                break;
            case RTM_NEWADDR:
            case RTM_DELADDR:
                rtnl_ifaddrmsg(nlh);
                break;
            case RTM_NEWROUTE:
            case RTM_DELROUTE:
                rtnl_rtmsg(nlh);
                break;
            default:
                LOG_ERROR("get msg %d",nlh->nlmsg_type);
                break;
        }
    }
}

struct epoll_handler_s *rtnl_init(uint32_t groups)
{
    int fd = socket(AF_NETLINK,SOCK_RAW,NETLINK_ROUTE);
    if(fd <= 0)
    {
        LOG_ERROR("create socket error");
        return NULL;
    }

    struct timeval t = {10,0};
    int ret = setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(char *)&t,sizeof(t));
    if(ret < 0)
        goto out_err;

    struct sockaddr_nl addr;
    addr.nl_family  = AF_NETLINK;
    addr.nl_pid     = getpid();
    addr.nl_groups  = groups;
    ret = bind(fd,(struct sockaddr *)&addr,sizeof(addr));
    if(ret < 0)
        goto out_err;

    struct epoll_handler_s *h = malloc(sizeof(struct epoll_handler_s));
    h->fd = fd;
    h->cb = rtnl_recv;
    if(epoll_add(h,EPOLLIN) < 0)
        goto out_free;
    
    return h;

out_free:
    free(h);
out_err:
    LOG_ERROR("%m");
    close(fd);
    return NULL;
}

