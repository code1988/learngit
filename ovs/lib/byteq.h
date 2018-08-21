/* Copyright (c) 2008, 2009, 2013 Nicira, Inc.
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

#ifndef BYTEQ_H
#define BYTEQ_H 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* General-purpose circular queue of bytes. 
 * 通用的环形缓冲区结构
 * 备注：数据从头部写入，从尾部读取
 * */
struct byteq {
    uint8_t *buffer;            /* Circular queue.  环形缓冲区 */
    unsigned int size;          /* Number of bytes allocated for 'buffer'. 环形缓冲区长度，必须确保是2的次方 */
    unsigned int head;          /* Head of queue.   数据头，这个值可以超过size大小 */
    unsigned int tail;          /* Chases the head. 数据尾，这个值可以超过size大小 */
};

void byteq_init(struct byteq *, uint8_t *buffer, size_t size);
int byteq_used(const struct byteq *);
int byteq_avail(const struct byteq *);
bool byteq_is_empty(const struct byteq *);
bool byteq_is_full(const struct byteq *);
void byteq_put(struct byteq *, uint8_t c);
void byteq_putn(struct byteq *, const void *, size_t n);
void byteq_put_string(struct byteq *, const char *);
uint8_t byteq_get(struct byteq *);
int byteq_write(struct byteq *, int fd);
int byteq_read(struct byteq *, int fd);

uint8_t *byteq_head(struct byteq *);
int byteq_headroom(const struct byteq *);
void byteq_advance_head(struct byteq *, unsigned int n);
int byteq_tailroom(const struct byteq *);
const uint8_t *byteq_tail(const struct byteq *);
void byteq_advance_tail(struct byteq *, unsigned int n);

#endif /* byteq.h */
