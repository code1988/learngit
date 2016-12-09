#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "libepoll.h"
#include "unixsock.h"

static void unixsock_recv(struct epoll_handler_s *h,uint32_t events)
{

}

struct epoll_handler_s *unixsock_init(char *s_name)
{
    int fd;

    fd = socket(AF_UNIX,SOCK_DGRAM,0);
    if(fd < 0)
        return NULL;

    struct sockaddr_un un;
    memset(&un,0,sizeof(struct sockaddr_un));
    un.sun_family = AF_UNIX;
    sprintf(un.sun_path + 1,"/var/run/%5d",getpid());
    unlink(un.sun_path);

    if (bind(fd, (struct sockaddr *)&un, sizeof(struct sockaddr_un)) < 0)
        goto out_err;

    memset(un.sun_path,0,sizeof(un.sun_path));
    strcpy(un.sun_path + 1,s_name);

    if (connect(fd, (struct sockaddr *)&un, sizeof(struct sockaddr_un)) < 0)
        goto out_err;

    struct epoll_handler_s *h = malloc(sizeof(struct epoll_handler_s));
    h->fd = fd;
    h->cb = unixsock_recv;
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
