/*
 * Copyright (c) 2013 Nicira, Inc.
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

#ifndef LATCH_H
#define LATCH_H 1

/* A thread-safe, signal-safe, pollable doorbell.
 *
 * This is a thin wrapper around a pipe that allows threads to notify each
 * other that an event has occurred in a signal-safe way  */

#include <stdbool.h>
#include "util.h"

/* 定义了一个线程安全、信号安全、可poll的通信接口结构
 * Linux下实质就是一对管道
 */
struct latch {
#ifndef _WIN32
    int fds[2];
#else
    HANDLE wevent;
    bool is_set;
#endif
};

void latch_init(struct latch *);
void latch_destroy(struct latch *);

bool latch_poll(struct latch *);
void latch_set(struct latch *);

bool latch_is_set(const struct latch *);
void latch_wait_at(const struct latch *, const char *where);
#define latch_wait(latch) latch_wait_at(latch, OVS_SOURCE_LOCATOR)

#endif /* latch.h */
