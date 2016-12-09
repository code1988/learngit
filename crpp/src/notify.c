#include "notify.h"
#include "libepoll.h"
#include "unixsock.h"

#define RSTP_SERVER "/var/run/rstpd.sock"
#define JRPP_SERVER "/var/run/jrppd.sock"

static void notify_server(struct list_head *head)
{

    struct epoll_handler_s *h = unixsock_init(JRPP_SERVER);
    if(h == NULL)
    {
        LOG_ERROR("unixsock init error");
        return;
    }
    list_add(&h->list,head);

}


int notify_init(void (*cb)(struct list_head *head))
{
    cb = notify_server;

    return 0;
}
