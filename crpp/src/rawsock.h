#ifndef _RAWSOCK_H_
#define _RAWSOCK_H_

#include <linux/filter.h>
#include "utils.h"

struct epoll_handler_s *rawsock_init(struct sock_fprog *prog);
#endif
