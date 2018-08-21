/*
 * Copyright (c) 2008, 2009, 2010, 2012, 2013, 2014, 2015 Nicira, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <config.h>
#include "stream-fd.h"
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "fatal-signal.h"
#include "poll-loop.h"
#include "socket-util.h"
#include "util.h"
#include "stream-provider.h"
#include "stream.h"
#include "openvswitch/vlog.h"

VLOG_DEFINE_THIS_MODULE(stream_fd);

/* Active file descriptor stream. */
// 基于描述符的stream封装结构
struct stream_fd
{
    struct stream stream;   // 封装的stream基类
    int fd;                 // 关联的描述符
    int fd_type;            // 描述符类型
};

static const struct stream_class stream_fd_class;

static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(10, 25);

static void maybe_unlink_and_free(char *path);

/* Creates a new stream named 'name' that will send and receive data on 'fd'
 * and stores a pointer to the stream in '*streamp'.  Initial connection status
 * 'connect_status' is interpreted as described for stream_init(). 'fd_type'
 * tells whether the socket is TCP or Unix domain socket.
 * 创建一个基于描述符的stream管理块
 *
 * Takes ownership of 'name'.
 *
 * Returns 0 if successful, otherwise a positive errno value.  (The current
 * implementation never fails.) */
int
new_fd_stream(char *name, int fd, int connect_status, int fd_type,
              struct stream **streamp)
{
    struct stream_fd *s;

    s = xmalloc(sizeof *s);
    stream_init(&s->stream, &stream_fd_class, connect_status, name);
    s->fd = fd;
    s->fd_type = fd_type;
    *streamp = &s->stream;
    return 0;
}

static struct stream_fd *
stream_fd_cast(struct stream *stream)
{
    stream_assert_class(stream, &stream_fd_class);
    return CONTAINER_OF(stream, struct stream_fd, stream);
}

static void
fd_close(struct stream *stream)
{
    struct stream_fd *s = stream_fd_cast(stream);
    closesocket(s->fd);
    free(s);
}

static int
fd_connect(struct stream *stream)
{
    struct stream_fd *s = stream_fd_cast(stream);
    int retval = check_connection_completion(s->fd);
    if (retval == 0 && s->fd_type == AF_INET) {
        setsockopt_tcp_nodelay(s->fd);
    }
    return retval;
}

static ssize_t
fd_recv(struct stream *stream, void *buffer, size_t n)
{
    struct stream_fd *s = stream_fd_cast(stream);
    ssize_t retval;
    int error;

    retval = recv(s->fd, buffer, n, 0);
    if (retval < 0) {
        error = sock_errno();
#ifdef _WIN32
        if (error == WSAEWOULDBLOCK) {
           error = EAGAIN;
        }
#endif
        if (error != EAGAIN) {
            VLOG_DBG_RL(&rl, "recv: %s", sock_strerror(error));
        }
        return -error;
    }
    return retval;
}

// 描述符类型stream的发送数据接口(实际unix-socket和tcp-socket都是调用该接口发送数据)
static ssize_t
fd_send(struct stream *stream, const void *buffer, size_t n)
{
    struct stream_fd *s = stream_fd_cast(stream);
    ssize_t retval;
    int error;

    retval = send(s->fd, buffer, n, 0);
    if (retval < 0) {
        error = sock_errno();
#ifdef _WIN32
        if (error == WSAEWOULDBLOCK) {
           error = EAGAIN;
        }
#endif
        if (error != EAGAIN) {
            VLOG_DBG_RL(&rl, "send: %s", sock_strerror(error));
        }
        return -error;
    }
    return (retval > 0 ? retval : -EAGAIN);
}

static void
fd_wait(struct stream *stream, enum stream_wait_type wait)
{
    struct stream_fd *s = stream_fd_cast(stream);
    switch (wait) {
    case STREAM_CONNECT:
    case STREAM_SEND:
        poll_fd_wait(s->fd, POLLOUT);
        break;

    case STREAM_RECV:
        poll_fd_wait(s->fd, POLLIN);
        break;

    default:
        OVS_NOT_REACHED();
    }
}

// 定义了属于stream的通用文件类(显然stream的TCP套接字类和UNIX域套接字类都属于通用文件类)
static const struct stream_class stream_fd_class = {
    "fd",                       /* name */
    false,                      /* needs_probes */
    NULL,                       /* open */
    fd_close,                   /* close */
    fd_connect,                 /* connect */
    fd_recv,                    /* recv */
    fd_send,                    /* send */
    NULL,                       /* run */
    NULL,                       /* run_wait */
    fd_wait,                    /* wait */
};

/* Passive file descriptor stream. 
 * 定义了文件描述符风格的pstream管理块
 * */
struct fd_pstream
{
    struct pstream pstream; // 封装的pstream基类
    int fd;                 // 该pstream关联的fd
    int (*accept_cb)(int fd, const struct sockaddr_storage *, size_t ss_len,
                     struct stream **); /* 该pstream在调用pfd_accept时，会递归调用本回调函数
                                         * 两者区别在于本回调函数跟具体类型的文件描述符相关(inet-socket/unix-socket)
                                         */
    char *unlink_path;      // 通常就是该pstream关联fd的绑定地址
};

static struct fd_pstream *
fd_pstream_cast(struct pstream *pstream)
{
    pstream_assert_class(pstream, &fd_pstream_class);
    return CONTAINER_OF(pstream, struct fd_pstream, pstream);
}

/* Creates a new pstream named 'name' that will accept new socket connections
 * on 'fd' and stores a pointer to the stream in '*pstreamp'.
 * 创建一个pstream管理块,实际上这里创建的是一个进一步封装过的fd_pstream
 * @name        "punix:$ADDR"格式,其中"$ADDR"代表该UNIX域服务端套接字要绑定的地址
 * @fd          该pstream管理块关联的fd
 * @accept_cb   该pstream接受远端连接请求时的回调函数
 * @unlink_path 该pstream管理块关联fd的绑定地址 
 * @pstreamp    用于存放创建的pstream管理块
 *
 * When a connection has been accepted, 'accept_cb' will be called with the new
 * socket fd 'fd' and the remote address of the connection 'sa' and 'sa_len'.
 * accept_cb must return 0 if the connection is successful, in which case it
 * must initialize '*streamp' to the new stream, or a positive errno value on
 * error.  In either case accept_cb takes ownership of the 'fd' passed in.
 *
 * When '*pstreamp' is closed, then 'unlink_path' (if nonnull) will be passed
 * to fatal_signal_unlink_file_now() and freed with free().
 *
 * Takes ownership of 'name'.
 *
 * Returns 0 if successful, otherwise a positive errno value.  (The current
 * implementation never fails.) */
int
new_fd_pstream(char *name, int fd,
               int (*accept_cb)(int fd, const struct sockaddr_storage *ss,
                                size_t ss_len, struct stream **streamp),
               char *unlink_path, struct pstream **pstreamp)
{
    struct fd_pstream *ps = xmalloc(sizeof *ps);
    pstream_init(&ps->pstream, &fd_pstream_class, name);
    ps->fd = fd;
    ps->accept_cb = accept_cb;
    ps->unlink_path = unlink_path;
    *pstreamp = &ps->pstream;
    return 0;
}

static void
pfd_close(struct pstream *pstream)
{
    struct fd_pstream *ps = fd_pstream_cast(pstream);
    closesocket(ps->fd);
    maybe_unlink_and_free(ps->unlink_path);
    free(ps);
}

/* 描述符类型pstream的accept接口(实际unix-socket和tcp-socket都是调用该接口接受连接)
 * @pstream     指向描述符类型的pstream
 * @new_streamp 用于存放创建的stream
 */
static int
pfd_accept(struct pstream *pstream, struct stream **new_streamp)
{
    struct fd_pstream *ps = fd_pstream_cast(pstream);
    struct sockaddr_storage ss;
    socklen_t ss_len = sizeof ss;
    int new_fd;
    int retval;

    new_fd = accept(ps->fd, (struct sockaddr *) &ss, &ss_len);
    if (new_fd < 0) {
        retval = sock_errno();
#ifdef _WIN32
        if (retval == WSAEWOULDBLOCK) {
            retval = EAGAIN;
        }
#endif
        if (retval != EAGAIN) {
            VLOG_DBG_RL(&rl, "accept: %s", sock_strerror(retval));
        }
        return retval;
    }

    retval = set_nonblocking(new_fd);
    if (retval) {
        closesocket(new_fd);
        return retval;
    }

    // 进一步执行套接字类型相关的accept回调函数
    return ps->accept_cb(new_fd, &ss, ss_len, new_streamp);
}

static void
pfd_wait(struct pstream *pstream)
{
    struct fd_pstream *ps = fd_pstream_cast(pstream);
    poll_fd_wait(ps->fd, POLLIN);
}

/* 定义了文件描述符类型的pstream对象
 * 备注：不管是unix-socket还是tcp-socket实际底层关联的都是文件描述符类型的pstream对象
 */
static const struct pstream_class fd_pstream_class = {
    "pstream",
    false,
    NULL,
    pfd_close,
    pfd_accept,
    pfd_wait,
};

/* Helper functions. */
static void
maybe_unlink_and_free(char *path)
{
    if (path) {
        fatal_signal_unlink_file_now(path);
        free(path);
    }
}
