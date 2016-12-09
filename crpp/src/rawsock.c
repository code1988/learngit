#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "libepoll.h"
#include "rawsock.h"

static void rawsock_recv(struct epoll_handler_s *h,uint32_t events)
{

}

struct epoll_handler_s *rawsock_init(struct sock_fprog *prog)
{
    int fd;

    fd = socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
    if(fd < 0)
        return NULL;

    struct ifreq ifr;
    strcpy(ifr.ifr_name,"eth0");
    if(ioctl(fd,SIOCGIFINDEX,&ifr))
        goto out_close;

    struct sockaddr_ll toaddr = {
        .sll_ifindex    = ifr.ifr_ifindex,
        .sll_family     = AF_PACKET,
        .sll_protocol   = htons(ETH_P_ALL), 
    };
    
    if(bind(fd,(struct sockaddr *)&toaddr,sizeof(toaddr)) < 0)
        goto out_close;

    if(prog != NULL && setsockopt(fd,SOL_SOCKET,SO_ATTACH_FILTER,prog,sizeof(struct sock_fprog)) < 0)
        goto out_close;

    fcntl(fd,F_SETFL,fcntl(fd,F_GETFL) | O_NONBLOCK);

    struct epoll_handler_s *h = malloc(sizeof(struct epoll_handler_s));
    h->fd = fd;
    h->cb = rawsock_recv;
    if(epoll_add(h,EPOLLIN) < 0)
        goto out_free;
    
    return h;

out_free:
    free(h);
out_close:
    LOG_ERROR("%m");
    close(fd);
    return NULL;
}

