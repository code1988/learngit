#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include <linux/rtnetlink.h>

#include <unistd.h>
#include <getopt.h>
#include <libgen.h>
#include <syslog.h>

#include "crpp.h"
#include "libepoll.h"
#include "librtnl.h"
#include "rawsock.h"
#include "unixsock.h"


/**<    name rule: JW.year|month.01(main ver).01(sub ver) */
#define VERSION "JW.201612.02.01"    
#define MODULE_NAME "crppd"

static crpp_t *pCrpp = NULL;
static struct sock_filter crpp_filter[] = {
    { 0x20, 0, 0, 0x00000002 },
    { 0x15, 0, 3, 0x2a000006 },
    { 0x28, 0, 0, 0x00000000 },
    { 0x15, 0, 1, 0x00000da4 },
    { 0x6,  0, 0, 0x00040000 },
    { 0x6,  0, 0, 0x00000000 },
};

static int usage(const char *prog)
{
	LOG_ERROR("Usage: %s [options]\n"
		"Options:\n"
		"\t-d <level>\tEnable debug messages\n"
		"\t-u <level>\tSelect channel to commit log\n"
		"\n", prog);
	return 1;
}

static int crpp_init(void)
{
    pCrpp = (crpp_t *)calloc(1,sizeof(crpp_t));
    if(pCrpp == NULL)
    {
        LOG_ERROR("calloc crpp error");
        return -1;
    }
    pCrpp->state = CRPP_ENABLED;
    INIT_LIST_HEAD(&pCrpp->fd_l);

    struct sock_fprog prog = {
        .len = ARRAY_SIZE(crpp_filter),
        .filter = crpp_filter,
    };

    struct epoll_handler_s *h = rawsock_init(&prog);
    if(h == NULL)
    {
        LOG_ERROR("rawsock init error");
        return -1;
    }
    list_add(&h->list,&pCrpp->fd_l);

    h = rtnl_init(RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR);
    if(h == NULL)
    {
        LOG_ERROR("rawsock init error");
        return -1;
    }
    list_add(&h->list,&pCrpp->fd_l);

    return 0;
}

int main(int argc, char **argv)
{
	int ch;
    int level,channel;
    
	while ((ch = getopt(argc, argv, "vu:d:s:")) != -1) {
		switch (ch) {
		case 'd':
			level = atoi(optarg);
            log_threshold(level);
			break;
		case 'u':
			channel = atoi(optarg);
            log_set_channel(channel);
			break;
        case 'v':
            printf("crppd version: %s\n",VERSION);
            return 0;
		default:
			return usage(argv[0]);
		}
	}

    log_init(MODULE_NAME,LOG_DAEMON);
    //openlog("crppd",LOG_CONS | LOG_PID,LOG_LOCAL1);

    if(daemon(0,0) < 0)
    {
        LOG_ERROR("daemon error");
        return -1;
    }

	if(epoll_init() < 0)
    {
        LOG_ERROR("epoll init error");
        return -1;
    }
    
	if(crpp_init() < 0)
    {
        LOG_ERROR("crpp init error");
        return -1;
    }
    
    epoll_loop();
	return 0;
}
