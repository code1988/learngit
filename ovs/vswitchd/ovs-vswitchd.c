/* Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Nicira, Inc.
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

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_MLOCKALL
#include <sys/mman.h>
#endif

#include "bridge.h"
#include "command-line.h"
#include "compiler.h"
#include "daemon.h"
#include "dirs.h"
#include "dpif.h"
#include "dummy.h"
#include "fatal-signal.h"
#include "memory.h"
#include "netdev.h"
#include "openflow/openflow.h"
#include "ovsdb-idl.h"
#include "poll-loop.h"
#include "simap.h"
#include "stream-ssl.h"
#include "stream.h"
#include "svec.h"
#include "timeval.h"
#include "unixctl.h"
#include "util.h"
#include "openvswitch/vconn.h"
#include "openvswitch/vlog.h"
#include "lib/vswitch-idl.h"

VLOG_DEFINE_THIS_MODULE(vswitchd);

/* --mlockall: If set, locks all process memory into physical RAM, preventing
 * the kernel from paging any of its memory to disk. 
 * 标识是否将进程的所有内存锁在物理RAM中,以阻止kernel将其交换到磁盘中
 * */
static bool want_mlockall;

static unixctl_cb_func ovs_vswitchd_exit;

static char *parse_options(int argc, char *argv[], char **unixctl_path);
OVS_NO_RETURN static void usage(void);

struct ovs_vswitchd_exit_args {
    bool *exiting;
    bool *cleanup;
};

// ovs-vswitchd守护进程起始入口
int
main(int argc, char *argv[])
{
    char *unixctl_path = NULL;
    struct unixctl_server *unixctl;
    char *remote;
    bool exiting, cleanup;
    struct ovs_vswitchd_exit_args exit_args = {&exiting, &cleanup};
    int retval;

    // 首先记录当前程序名和程序版本号
    set_program_name(argv[0]);

    // 预先对命令行参数进行处理,之后argv表中记录的都是拷贝的参数
    ovs_cmdl_proctitle_init(argc, argv);
    // linux平台直接跳过这函数
    service_start(&argc, &argv);
    // 解析程序传入的命令行参数
    remote = parse_options(argc, argv, &unixctl_path);
    // 忽略SIGPIPE信号
    fatal_ignore_sigpipe();

    // 启动守护进程
    daemonize_start(true);

    // 如果配置了将进程所有内存锁在物理RAM中选项,则在这里进行设置
    if (want_mlockall) {
#ifdef HAVE_MLOCKALL
        // 对所有已经或者将要映射到当前进程地址空间的内存页上锁
        if (mlockall(MCL_CURRENT | MCL_FUTURE)) {
            VLOG_ERR("mlockall failed: %s", ovs_strerror(errno));
        }
#else
        VLOG_ERR("mlockall not supported on this system");
#endif
    }

    // 如果配置了ovs-vswitchd本地套接字设置,则创建UNIX域套接字服务端并开启监听,该套接字将作为ovs-appctl控制ovs-vswitchd的接口
    retval = unixctl_server_create(unixctl_path, &unixctl);
    if (retval) {
        exit(EXIT_FAILURE);
    }
    // 为该UNIX域服务端套接字注册"exit"命令
    unixctl_command_register("exit", "[--cleanup]", 0, 1,
                             ovs_vswitchd_exit, &exit_args);

    // bridge的初始化入口
    bridge_init(remote);
    free(remote);

    exiting = false;
    cleanup = false;
    while (!exiting) {
        // 定期执行内存使用情况监视工作
        memory_run();
        // 如果调用者需要打印内存使用情况的日志,则执行打印
        if (memory_should_report()) {
            struct simap usage;

            simap_init(&usage);
            // 转储bridge相关的内存使用统计数据
            bridge_get_memory_usage(&usage);
            // 报告内存使用情况
            memory_report(&usage);
            simap_destroy(&usage);
        }
        bridge_run();
        unixctl_server_run(unixctl);
        netdev_run();

        memory_wait();
        bridge_wait();
        unixctl_server_wait(unixctl);
        netdev_wait();
        // 如果判定要终止进程,则设置立即唤醒poll模型
        if (exiting) {
            poll_immediate_wake();
        }
        // poll入口
        poll_block();
        if (should_service_stop()) {
            exiting = true;
        }
    }
    bridge_exit(cleanup);
    unixctl_server_destroy(unixctl);
    service_stop();

    return 0;
}

/* 解析ovs-vswitchd程序传入的命令行参数
 * @unixctl_pathp   用于存放ovs-vswitchd的本地套接字服务端地址
 * @返回值  ovsdb-server的本地套接字服务端地址
 */
static char *
parse_options(int argc, char *argv[], char **unixctl_pathp)
{
    enum {
        OPT_PEER_CA_CERT = UCHAR_MAX + 1,
        OPT_MLOCKALL,
        OPT_UNIXCTL,
        VLOG_OPTION_ENUMS,
        OPT_BOOTSTRAP_CA_CERT,
        OPT_ENABLE_DUMMY,
        OPT_DISABLE_SYSTEM,
        DAEMON_OPTION_ENUMS,
        OPT_DPDK,
        SSL_OPTION_ENUMS,
        OPT_DUMMY_NUMA,
    };
    // 定义了支持的长选项
    static const struct option long_options[] = {
        {"help",        no_argument, NULL, 'h'},
        {"version",     no_argument, NULL, 'V'},
        {"mlockall",    no_argument, NULL, OPT_MLOCKALL},
        {"unixctl",     required_argument, NULL, OPT_UNIXCTL},
        DAEMON_LONG_OPTIONS,
        VLOG_LONG_OPTIONS,
        STREAM_SSL_LONG_OPTIONS,
        {"peer-ca-cert", required_argument, NULL, OPT_PEER_CA_CERT},
        {"bootstrap-ca-cert", required_argument, NULL, OPT_BOOTSTRAP_CA_CERT},
        {"enable-dummy", optional_argument, NULL, OPT_ENABLE_DUMMY},
        {"disable-system", no_argument, NULL, OPT_DISABLE_SYSTEM},
        {"dpdk", optional_argument, NULL, OPT_DPDK},
        {"dummy-numa", required_argument, NULL, OPT_DUMMY_NUMA},
        {NULL, 0, NULL, 0},
    };
    // 根据支持的长选项得到对应的短选项
    char *short_options = ovs_cmdl_long_options_to_short_options(long_options);

    // 命令行长选项和短选项参数解析
    for (;;) {
        int c;

        c = getopt_long(argc, argv, short_options, long_options, NULL);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'h':
            usage();

        case 'V':
            ovs_print_version(0, 0);
            exit(EXIT_SUCCESS);

        case OPT_MLOCKALL:      // 将进程的内存全部锁在RAM中
            want_mlockall = true;
            break;

        case OPT_UNIXCTL:
            *unixctl_pathp = optarg;
            break;

        VLOG_OPTION_HANDLERS
        DAEMON_OPTION_HANDLERS
        STREAM_SSL_OPTION_HANDLERS

        case OPT_PEER_CA_CERT:
            stream_ssl_set_peer_ca_cert_file(optarg);
            break;

        case OPT_BOOTSTRAP_CA_CERT:
            stream_ssl_set_ca_cert_file(optarg, true);
            break;

        case OPT_ENABLE_DUMMY:
            dummy_enable(optarg);
            break;

        case OPT_DISABLE_SYSTEM:
            dp_blacklist_provider("system");
            break;

        case '?':
            exit(EXIT_FAILURE);

        case OPT_DPDK:
            ovs_fatal(0, "Using --dpdk to configure DPDK is not supported.");
            break;

        case OPT_DUMMY_NUMA:
            ovs_numa_set_dummy(optarg);
            break;

        default:
            abort();
        }
    }
    free(short_options);

    argc -= optind;
    argv += optind;

    // 计算得到ovsdb-server的本地套接字服务端地址
    switch (argc) {
    case 0:
        return xasprintf("unix:%s/db.sock", ovs_rundir());

    case 1:
        return xstrdup(argv[0]);

    default:
        VLOG_FATAL("at most one non-option argument accepted; "
                   "use --help for usage");
    }
}

static void
usage(void)
{
    printf("%s: Open vSwitch daemon\n"
           "usage: %s [OPTIONS] [DATABASE]\n"
           "where DATABASE is a socket on which ovsdb-server is listening\n"
           "      (default: \"unix:%s/db.sock\").\n",
           program_name, program_name, ovs_rundir());
    stream_usage("DATABASE", true, false, true);
    daemon_usage();
    vlog_usage();
    printf("\nDPDK options:\n"
           "Configuration of DPDK via command-line is removed from this\n"
           "version of Open vSwitch. DPDK is configured through ovsdb.\n"
          );
    printf("\nOther options:\n"
           "  --unixctl=SOCKET          override default control socket name\n"
           "  -h, --help                display this help message\n"
           "  -V, --version             display version information\n");
    exit(EXIT_SUCCESS);
}

// "exit"命令的回调函数
static void
ovs_vswitchd_exit(struct unixctl_conn *conn, int argc,
                  const char *argv[], void *exit_args_)
{
    struct ovs_vswitchd_exit_args *exit_args = exit_args_;
    *exit_args->exiting = true;
    *exit_args->cleanup = argc == 2 && !strcmp(argv[1], "--cleanup");
    unixctl_command_reply(conn, NULL);
}
