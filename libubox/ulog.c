/*
 * ulog - simple logging functions
 *
 * Copyright (C) 2015 Jo-Philipp Wich <jow@openwrt.org>
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

#include "ulog.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static int _ulog_channels = -1;         // 打印通道：ULOG_KMSG ULOG_SYSLOG ULOG_STDIO
static int _ulog_facility = -1;         // 程序类型：LOG_DAEMON LOG_USER等
static int _ulog_threshold = LOG_DEBUG; // 打印阈值，优先级低于阈值的打印请求会被忽略(值越高优先级越低,LOG_DEBUG=7表示最低优先级，意味着执行全部打印请求)
static int _ulog_initialized = 0;       // 打印设置初始化标志：0-未初始化 1-初始化完毕
static const char *_ulog_ident = NULL;  // 打印信息头

// 获取缺省的消息头
static const char *ulog_default_ident(void)
{
	FILE *self;
	static char line[64];
	char *p = NULL;

    // 打开/proc/self/status，从中获取当前进程名，作为缺省的消息头
	if ((self = fopen("/proc/self/status", "r")) != NULL) {
		while (fgets(line, sizeof(line), self)) {
			if (!strncmp(line, "Name:", 5)) {
				strtok(line, "\t\n");
				p = strtok(NULL, "\t\n");
				break;
			}
		}
		fclose(self);
	}

	return p;
}

// 设置缺省的打印参数,其实就是执行初始化
static void ulog_defaults(void)
{
	char *env;

    // 如果之前已经作了初始化，直接退出
	if (_ulog_initialized)
		return;

    // 获取环境变量PREINIT（实际未声明）
	env = getenv("PREINIT");

    // 打印通道缺省值-1
	if (_ulog_channels < 0) {
		if (env && !strcmp(env, "1"))
			_ulog_channels = ULOG_KMSG;
		else if (isatty(1))     // 实际运行到这里,即缺省打印通道设为ULOG_STDIO
			_ulog_channels = ULOG_STDIO;
		else
			_ulog_channels = ULOG_SYSLOG;
	}

    // 程序模块缺省值-1
	if (_ulog_facility < 0) {
		if (env && !strcmp(env, "1"))
			_ulog_facility = LOG_DAEMON;
		else if (isatty(1))     // 实际运行到这里，即缺省程序模块设为LOG_USER
			_ulog_facility = LOG_USER;
		else
			_ulog_facility = LOG_DAEMON;
	}

    // 如果未设置信息头而且打印通道不是标准输入输出，则设置一个缺省的信息头
	if (_ulog_ident == NULL && _ulog_channels != ULOG_STDIO)
		_ulog_ident = ulog_default_ident();

    // 如果打印通道设为ULOG_SYSLOG,则需要调用openlog打开
	if (_ulog_channels & ULOG_SYSLOG)
		openlog(_ulog_ident, 0, _ulog_facility);

    // 初始化完毕
	_ulog_initialized = 1;
}

// 执行打印到/dev/kmsg
static void ulog_kmsg(int priority, const char *fmt, va_list ap)
{
	FILE *kmsg;

	if ((kmsg = fopen("/dev/kmsg", "r+")) != NULL) {
        // 添加优先级
		fprintf(kmsg, "<%u>", priority);

        // 添加信息头
		if (_ulog_ident)
			fprintf(kmsg, "%s: ", _ulog_ident);

        // 添加信息正文
		vfprintf(kmsg, fmt, ap);
		fclose(kmsg);
	}
}

// 执行打印到标准错误输出
static void ulog_stdio(int priority, const char *fmt, va_list ap)
{
	FILE *out = stderr;

    // 添加信息头
	if (_ulog_ident)
		fprintf(out, "%s: ", _ulog_ident);

    // 添加信息正文
	vfprintf(out, fmt, ap);
}

// 执行打印到系统日志
static void ulog_syslog(int priority, const char *fmt, va_list ap)
{
	vsyslog(priority, fmt, ap);
}

// 设置打印参数
void ulog_open(int channels, int facility, const char *ident)
{
	ulog_close();

	_ulog_channels = channels;
	_ulog_facility = facility;
	_ulog_ident = ident;
}

// 关闭打印设置
void ulog_close(void)
{
	if (!_ulog_initialized)
		return;

	if (_ulog_channels & ULOG_SYSLOG)
		closelog();

	_ulog_initialized = 0;
}

// 设置打印阈值
void ulog_threshold(int threshold)
{
	_ulog_threshold = threshold;
}

// 执行打印
void ulog(int priority, const char *fmt, ...)
{
	va_list ap;

    // 优先级低于阈值的打印请求直接忽略
	if (priority > _ulog_threshold)
		return;

    // 执行初始化
	ulog_defaults();

    // 打印通道指向KMSG
	if (_ulog_channels & ULOG_KMSG)
	{
		va_start(ap, fmt);
		ulog_kmsg(priority, fmt, ap);
		va_end(ap);
	}

    // 打印通道指向STDIO
	if (_ulog_channels & ULOG_STDIO)
	{
		va_start(ap, fmt);
		ulog_stdio(priority, fmt, ap);
		va_end(ap);
	}

    // 打印通道指向SYSLOG
	if (_ulog_channels & ULOG_SYSLOG)
	{
		va_start(ap, fmt);
		ulog_syslog(priority, fmt, ap);
		va_end(ap);
	}
}
