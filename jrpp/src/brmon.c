
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/if_bridge.h>
#include <string.h>

#include "log.h"
#include "utils.h"
#include "libnetlink.h"
#include "epoll_loop.h"
#include "jrpp_if.h"
#include "jrppd.h"

/* RFC 2863 operational status */
enum {
    IF_OPER_UNKNOWN,
    IF_OPER_NOTPRESENT,
    IF_OPER_DOWN,
    IF_OPER_LOWERLAYERDOWN,
    IF_OPER_TESTING,
    IF_OPER_DORMANT,
    IF_OPER_UP,
};

/* link modes */
enum {
    IF_LINK_MODE_DEFAULT,
    IF_LINK_MODE_DORMANT, /* limit upward transition to dormant */
};

static const char *port_states[] = {
    [BR_STATE_DISABLED] = "disabled",
    [BR_STATE_LISTENING] = "listening",
    [BR_STATE_LEARNING] = "learning",
    [BR_STATE_FORWARDING] = "forwarding",
    [BR_STATE_BLOCKING] = "blocking",
};


struct rtnl_handle rth;
struct rtnl_handle rth_state;
struct epoll_event_handler br_handler;

static int br_filter_message(const struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
    //FILE *fp = arg;
	char buf[250];
	int offset = 0;
    struct ifinfomsg *ifi = NLMSG_DATA(n);
    struct rtattr * tb[IFLA_MAX + 1];
    int len = n->nlmsg_len;
    //char b1[IFNAMSIZ];
    int newlink;
    int br_index;

	//log_info("br_filter_message ... enter1\n");
	//fprintf(fp, "br_filter_message ... enter2\n");
	
	if (n->nlmsg_type == NLMSG_DONE)
    {
        log_info("this is in [%s],recv done message!\n",__func__);
		return 0;
    }

	len -= NLMSG_LENGTH(sizeof(*ifi));
	if (len < 0)
		return -1;

    if (ifi->ifi_family != AF_BRIDGE && ifi->ifi_family != AF_UNSPEC)
      return 0;

    if (n->nlmsg_type != RTM_NEWLINK && n->nlmsg_type != RTM_DELLINK)
      return 0;

	parse_rtattr(tb, IFLA_MAX, IFLA_RTA(ifi), len);

    if (tb[IFLA_MASTER] && ifi->ifi_family != AF_BRIDGE)
       return 0;

	if (tb[IFLA_IFNAME] == NULL) 
	   return -1;

#if 1
	if(n->nlmsg_type == RTM_DELLINK)
		offset += sprintf(buf+offset, "Deleted ");

	offset += sprintf(buf+offset, "Port-%02d %-6s ", ifi->ifi_index, (char*)RTA_DATA(tb[IFLA_IFNAME]));

	if(tb[IFLA_OPERSTATE]) {
		int state = *(int*)RTA_DATA(tb[IFLA_OPERSTATE]);
		switch (state) {
			case IF_OPER_UNKNOWN:
				offset += sprintf(buf+offset, "Unknown ");
				break;
			case IF_OPER_NOTPRESENT:
				offset += sprintf(buf+offset, "Not Present ");
				break;
			case IF_OPER_DOWN:
				offset += sprintf(buf+offset, "Down ");
				break;
			case IF_OPER_LOWERLAYERDOWN:
				//offset += sprintf(buf+offset, "Lowerlayerdown ");
				offset += sprintf(buf+offset, "[link-down]");
				break;
			case IF_OPER_TESTING:
				offset += sprintf(buf+offset, "Testing ");
				break;
			case IF_OPER_DORMANT:
				offset += sprintf(buf+offset, "Dormant ");
				break;
			case IF_OPER_UP:
				//offset += sprintf(buf+offset, "Up ");
				offset += sprintf(buf+offset, "[link-up]");
				break;
			default:
				offset += sprintf(buf+offset, "OperState(%d) ", state);
				break;
		}
	}
	#if 0
	if(tb[IFLA_MTU])
		offset += sprintf(buf+offset, "mtu %u ", *(int*)RTA_DATA(tb[IFLA_MTU]));

	if(tb[IFLA_MASTER])
		offset += sprintf(buf+offset, "master %s ", if_indextoname(*(int*)RTA_DATA(tb[IFLA_MASTER]), b1));
	#endif
	
	if(tb[IFLA_PROTINFO]) {
		uint8_t state = *(uint8_t *)RTA_DATA(tb[IFLA_PROTINFO]);
		
		if(state <= BR_STATE_BLOCKING)
			offset += sprintf(buf+offset, "State: %s", port_states[state]);
		else
			offset += sprintf(buf+offset, "State: (%d)", state);
	}

	offset += sprintf(buf+offset, "\n");

	newlink = (n->nlmsg_type == RTM_NEWLINK);

	if(tb[IFLA_MASTER])
		br_index = *(int*)RTA_DATA(tb[IFLA_MASTER]);
	else if(is_bridge((char*)RTA_DATA(tb[IFLA_IFNAME])))
		br_index = ifi->ifi_index;
	else
		br_index = -1;

	//log_info("%s", buf);
	
#endif
	
	br_notify(br_index, ifi->ifi_index, newlink, ifi->ifi_flags);
	//dump_hex(n, n->nlmsg_len);
	return 0;
}

static void br_event_handler(uint32_t events, struct epoll_event_handler *h)
{
	if (rtnl_listen(&rth, br_filter_message, stdout) < 0) {
		fprintf(stderr, "Error on bridge monitoring socket\n");
		exit(-1);
	}
}

int br_init_ops(void)
{
    // 创建一个netlink套接字，协议类型为NETLINK_ROUTE，多播
	if (rtnl_open(&rth, ~RTMGRP_TC) < 0) {
		LOG_ERROR("rtnl_open failed\n");
		return -1;
	}

    // 再创建一个netlink套接字，协议类型为NETLINK_ROUTE，单播
	if (rtnl_open(&rth_state, 0) < 0) {
		LOG_ERROR("rtnl_open failed for setting state\n");
		return -1;
	}
  
#if 0
	if (rtnl_wilddump_request(&rth, PF_BRIDGE, RTM_GETLINK) < 0) {
		LOG_ERROR("rtnl_wilddump_request failed\n");
		return -1;
	}

	if (rtnl_dump_filter(&rth, br_filter_message, stdout, NULL, NULL) < 0) {
		LOG_ERROR("rtnl_dump_filter terminated\n");
		return -1;
	}
#endif

	if (fcntl(rth.fd, F_SETFL, O_NONBLOCK) < 0) {
		LOG_ERROR("error: fcntl setting O_NONBLOCK, %m\n");
		return -1;
	}

	br_handler.fd = rth.fd;
	br_handler.arg = NULL;
	br_handler.handler = br_event_handler;

	if (add_epoll(&br_handler) < 0)
		return -1;

	return 0;
}

// 主动获取link状态
int br_get_config(void)
{
	return (rtnl_wilddump_request(&rth, PF_BRIDGE, RTM_GETLINK) < 0)?-1:0; 
}

