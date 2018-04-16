/*
 * Copyright (c) 2008, 2009, 2010 Nicira, Inc.
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

#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H 1

/* Utilities for command-line parsing. */

#include "compiler.h"

struct option;

/* Command handler context 
 * 定义了待解析命令的完整信息结构
 * */
struct ovs_cmdl_context {
    /* number of command line arguments  命令行参数数量(除程序名) */
    int argc;
    /* array of command line arguments 命令行参数表(除程序名) */
    char **argv;
    /* private context data defined by the API user */
    void *pvt;
};

typedef void (*ovs_cmdl_handler)(struct ovs_cmdl_context *);

// 定义了一条通用的命令抽象
struct ovs_cmdl_command {
    const char *name;   // 命令名(test中就是子模块名)
    const char *usage;  // 该命令用法
    int min_args;       // 该命令最少参数
    int max_args;       // 该命令最大参数
    ovs_cmdl_handler handler;   // 该命令的操作回调
    enum { OVS_RO, OVS_RW } mode;    /* Does this command modify things?  该命令的权限:只读/可读可写 */
};

char *ovs_cmdl_long_options_to_short_options(const struct option *options);
void ovs_cmdl_print_options(const struct option *options);
void ovs_cmdl_print_commands(const struct ovs_cmdl_command *commands);
void ovs_cmdl_run_command(struct ovs_cmdl_context *,
                          const struct ovs_cmdl_command[]);
void ovs_cmdl_run_command_read_only(struct ovs_cmdl_context *,
                                    const struct ovs_cmdl_command[]);

void ovs_cmdl_proctitle_init(int argc, char **argv);
#if defined(__FreeBSD__) || defined(__NetBSD__)
#define ovs_cmdl_proctitle_set setproctitle
#else
void ovs_cmdl_proctitle_set(const char *, ...)
    OVS_PRINTF_FORMAT(1, 2);
#endif
void ovs_cmdl_proctitle_restore(void);

#endif /* command-line.h */
