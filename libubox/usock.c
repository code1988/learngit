/*
 * usock - socket helper functions
 *
 * Copyright (C) 2010 Steven Barth <steven@midlink.org>
 * Copyright (C) 2011-2012 Felix Fietkau <nbd@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <poll.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "usock.h"

// 套接字close-on-exec和非阻塞功能设置
static void usock_set_flags(int sock, unsigned int type)
{
	if (!(type & USOCK_NOCLOEXEC))
		fcntl(sock, F_SETFD, fcntl(sock, F_GETFD) | FD_CLOEXEC);

	if (type & USOCK_NONBLOCK)
		fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK);
}

// 服务器/客户端创建（底层封装）,支持unix socket、inet socket
static int usock_connect(int type, struct sockaddr *sa, int sa_len, int family, int socktype, bool server)
{
	int sock;

	sock = socket(family, socktype, 0);
	if (sock < 0)
		return -1;

	usock_set_flags(sock, type);    // 套接字close-on-exec和非阻塞功能设置

	if (server) 
    {
        // 创建服务器
		const int one = 1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

		if (!bind(sock, sa, sa_len) &&
		    (socktype != SOCK_STREAM || !listen(sock, SOMAXCONN)))
			return sock;
	} 
    else 
    {
        // 创建客户端
		if (!connect(sock, sa, sa_len) || errno == EINPROGRESS)
			return sock;
	}

	close(sock);
	return -1;
}

// 创建unix socket
static int usock_unix(int type, const char *host, int socktype, bool server)
{
	struct sockaddr_un sun = {.sun_family = AF_UNIX};

    // 本地socket文件合法性检测
	if (strlen(host) >= sizeof(sun.sun_path)) {
		errno = EINVAL;
		return -1;
	}
	strcpy(sun.sun_path, host);

    // 服务器/客户端创建(底层封装)
	return usock_connect(type, (struct sockaddr*)&sun, sizeof(sun), AF_UNIX, socktype, server);
}

// 创建inet socket
static int usock_inet(int type, const char *host, const char *service, int socktype, bool server)
{
	struct addrinfo *result, *rp;
	struct addrinfo hints = {
		.ai_family = (type & USOCK_IPV6ONLY) ? AF_INET6 :
			(type & USOCK_IPV4ONLY) ? AF_INET : AF_UNSPEC,
		.ai_socktype = socktype,
		.ai_flags = AI_ADDRCONFIG
			| ((type & USOCK_SERVER) ? AI_PASSIVE : 0)
			| ((type & USOCK_NUMERIC) ? AI_NUMERICHOST : 0),
	};
	int sock = -1;

	if (getaddrinfo(host, service, &hints, &result))
		return -1;

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sock = usock_connect(type, rp->ai_addr, rp->ai_addrlen, rp->ai_family, socktype, server);
		if (sock >= 0)
			break;
	}

	freeaddrinfo(result);
	return sock;
}

// 对端口做整形转字符串操作
const char *usock_port(int port)
{
	static char buffer[sizeof("65535\0")];

	if (port < 0 || port > 65535)
		return NULL;

	snprintf(buffer, sizeof(buffer), "%u", port);

	return buffer;
}

// 创建服务器/客户端(二层封装)
int usock(int type, const char *host, const char *service) {
	int socktype = ((type & 0xff) == USOCK_TCP) ? SOCK_STREAM : SOCK_DGRAM;
	bool server = !!(type & USOCK_SERVER);
	int sock;

	if (type & USOCK_UNIX)
		sock = usock_unix(type, host, socktype, server);            // unix socket(即本地socket)
	else
		sock = usock_inet(type, host, service, socktype, server);   // inet socket(即网络socket)

	if (sock < 0)
		return -1;

	return sock;
}

int usock_wait_ready(int fd, int msecs) {
	struct pollfd fds[1];
	int res;

	fds[0].fd = fd;
	fds[0].events = POLLOUT;

	res = poll(fds, 1, msecs);
	if (res < 0) {
		return errno;
	} else if (res == 0) {
		return -ETIMEDOUT;
	} else {
		int err = 0;
		socklen_t optlen = sizeof(err);

		res = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &optlen);
		if (res)
			return errno;
		if (err)
			return err;
	}

	return 0;
}
