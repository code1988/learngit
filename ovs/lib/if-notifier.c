/*
 * Copyright (c) 2015 Red Hat, Inc.
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
#include "if-notifier.h"
#include "rtnetlink.h"
#include "util.h"

// 定义了一个网络接口变化通知结构
struct if_notifier {
    struct nln_notifier *notifier;  // 指向基类netlink通知结构
    if_notify_func *cb;     // 指向用户自定义的收到通知后的回调函数
    void *aux;              // 用户自定义的附加数据
};

/* 网络接口变化后的基础回调函数,实际就是进一步跳转执行用户定义的回调函数
 * @change  显然没用到
 * @aux     指向一个阅订者的struct if_notifier结构
 */
static void
if_notifier_cb(const struct rtnetlink_change *change OVS_UNUSED, void *aux)
{
    struct if_notifier *notifier;
    notifier = aux;
    notifier->cb(notifier->aux);
}

/* 创建一个网络接口变化通知实例
 * @cb  用户自定义的收到通知后的回调函数
 * @aux 用户自定义的附加数据
 */
struct if_notifier *
if_notifier_create(if_notify_func *cb, void *aux)
{
    struct if_notifier *notifier;
    notifier = xmalloc(sizeof *notifier);
    notifier->cb = cb;
    notifier->aux = aux;
    notifier->notifier = rtnetlink_notifier_create(if_notifier_cb, notifier);
    return notifier;
}

void
if_notifier_destroy(struct if_notifier *notifier)
{
    if (notifier) {
        rtnetlink_notifier_destroy(notifier->notifier);
        free(notifier);
    }
}

void
if_notifier_run(void)
{
    rtnetlink_run();
}

void
if_notifier_wait(void)
{
    rtnetlink_wait();
}
