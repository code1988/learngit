/*
 * blob - library for generating/parsing tagged binary data
 *
 * Copyright (C) 2010 Felix Fietkau <nbd@openwrt.org>
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

#include "blob.h"

// blob缓冲区容量调整(minlen：需要增减的容量值)
static bool
blob_buffer_grow(struct blob_buf *buf, int minlen)
{
	struct blob_buf *new;
	int delta = ((minlen / 256) + 1) * 256;         // 容量调整步进值256
	new = realloc(buf->buf, buf->buflen + delta);   // 容量调整
	if (new) {
		buf->buf = new;
		memset(buf->buf + buf->buflen, 0, delta);
		buf->buflen += delta;
	}
	return !!new;
}

// blob属性空间的id_len初始化
static void
blob_init(struct blob_attr *attr, int id, unsigned int len)
{
	len &= BLOB_ATTR_LEN_MASK;
	len |= (id << BLOB_ATTR_ID_SHIFT) & BLOB_ATTR_ID_MASK;
	attr->id_len = cpu_to_be32(len);
}


// 根据偏移量计算属性空间地址
static inline struct blob_attr *
offset_to_attr(struct blob_buf *buf, int offset)
{
	void *ptr = (char *)buf->buf + offset - BLOB_COOKIE;
	return ptr;
}

// 计算属性空间到缓冲区的偏移量
static inline int attr_to_offset(struct blob_buf *buf, struct blob_attr *attr)
{
	return (char *)attr - (char *) buf->buf + BLOB_COOKIE;
}

// blob缓冲区容量调整
bool blob_buf_grow(struct blob_buf *buf, int required)
{
	int offset_head = attr_to_offset(buf, buf->head);

	if (!buf->grow || !buf->grow(buf, required))
		return false;

	buf->head = offset_to_attr(buf, offset_head);
	return true;
}

// 从blob缓冲区中分配一个属性空间并初始化(如果pos指向缓冲区首地址则意味着分配该blob第一个属性空间)
static struct blob_attr *blob_add(struct blob_buf *buf, struct blob_attr *pos, int id, int payload)
{
	int offset = attr_to_offset(buf, pos);
    // 以下计算缓冲区容量方法意味着其最小容量是包含一个struct blob_attr属性空间头的长度
	int required = (offset - BLOB_COOKIE + sizeof(struct blob_attr) + payload) - buf->buflen;
	struct blob_attr *attr;

    // 判断缓冲区的容量是否足够
	if (required > 0) 
    {
        // 如果缓冲区不足，则调整缓冲区容量
		if (!blob_buf_grow(buf, required))
			return NULL;
        // 在缓冲区中分配属性空间的地址
		attr = offset_to_attr(buf, offset);
	} 
    else 
    {
        // 如果缓冲区足够，则直接在缓冲区中分配属性空间的地址
		attr = pos;
	}

    // blob属性空间的id_len初始化
	blob_init(attr, id, payload + sizeof(struct blob_attr));
	blob_fill_pad(attr);
	return attr;
}

/* blob初始化(id - 数据类型)
 * 初始化完后的blob_buf拥有第一级属性空间(id = 0,空数据区)
 */
int blob_buf_init(struct blob_buf *buf, int id)
{
    // 确认容量调整函数是否已经注册
	if (!buf->grow)
		buf->grow = blob_buffer_grow;

    // 将属性空间地址初始化为缓冲区首地址
	buf->head = buf->buf;

    // 从缓冲区中分配第一级属性空间并初始化
	if (blob_add(buf, buf->buf, id, 0) == NULL)
		return -ENOMEM;

	return 0;
}

// blob销毁
void
blob_buf_free(struct blob_buf *buf)
{
	free(buf->buf);
	buf->buf = NULL;
	buf->buflen = 0;
}

// 如果BLOB属性空间没有4字节对齐，则需要对补充部分的空间清零
void
blob_fill_pad(struct blob_attr *attr)
{
	char *buf = (char *) attr;
	int len = blob_pad_len(attr);
	int delta = len - blob_raw_len(attr);

	if (delta > 0)
		memset(buf + len - delta, 0, delta);
}

// 设置属性头中数据区长度
void blob_set_raw_len(struct blob_attr *attr, unsigned int len)
{
	len &= BLOB_ATTR_LEN_MASK;
	attr->id_len &= ~cpu_to_be32(BLOB_ATTR_LEN_MASK);
	attr->id_len |= cpu_to_be32(len);
}

// 在当前嵌套级别的领袖blob属性空间中再开辟一个带数据类型的属性空间
struct blob_attr *blob_new(struct blob_buf *buf, int id, int payload)
{
	struct blob_attr *attr;
    
    // 从blob缓冲区中分配一个属性空间并初始化
	attr = blob_add(buf, blob_next(buf->head), id, payload);
	if (!attr)
		return NULL;

    // 更新当前嵌套级别的领袖blob的属性空间中记录的数据区长度
	blob_set_raw_len(buf->head, blob_pad_len(buf->head) + blob_pad_len(attr));
	return attr;
}

// 在已有的blob属性空间后面再开辟一个属性空间,然后将源属性空间整个拷贝过来
struct blob_attr *
blob_put_raw(struct blob_buf *buf, const void *ptr, unsigned int len)
{
	struct blob_attr *attr;

	if (len < sizeof(struct blob_attr) || !ptr)
		return NULL;

    // 从blob缓冲区中分配一个属性空间并初始化(不包含数据类型)
	attr = blob_add(buf, blob_next(buf->head), 0, len - sizeof(struct blob_attr));
	if (!attr)
		return NULL;

    // 更新已有的blob属性空间中数据区长度
	blob_set_raw_len(buf->head, blob_pad_len(buf->head) + len);

    // 将源属性空间的数据拷贝过来
	memcpy(attr, ptr, len);
	return attr;
}

// 在当前嵌套级别的领袖blob属性空间中再开辟一个属性空间，并设置数据类型、填入数据
struct blob_attr *
blob_put(struct blob_buf *buf, int id, const void *ptr, unsigned int len)
{
	struct blob_attr *attr;

    // 在当前嵌套级别的领袖blob属性空间中再开辟一个属性空间
	attr = blob_new(buf, id, len);
	if (!attr)
		return NULL;

    // 将数据填入新开辟的属性空间所属的数据区
	if (ptr)
		memcpy(blob_data(attr), ptr, len);
	return attr;
}

/* 创建下一级嵌套并打开
 * 返回上一级嵌套到整个缓冲区的偏移值，该偏移值用于返回上一级嵌套
 */
void *blob_nest_start(struct blob_buf *buf, int id)
{
	unsigned long offset = attr_to_offset(buf, buf->head);
    
    // 将head指针指向新开辟的属性空间
	buf->head = blob_new(buf, id, 0);
	if (!buf->head)
		return NULL;
	return (void *) offset;
}

// 关闭嵌套区
void blob_nest_end(struct blob_buf *buf, void *cookie)
{
    // 得到上一级嵌套区领袖blob属性地址
	struct blob_attr *attr = offset_to_attr(buf, (unsigned long) cookie);

    // 更新上一级嵌套区领袖blob属性空间中记录的数据区长度
	blob_set_raw_len(attr, blob_pad_len(attr) + blob_len(buf->head));

    // 将head指针指向上一级嵌套区领袖blob属性空间
	buf->head = attr;
}

static const int blob_type_minlen[BLOB_ATTR_LAST] = {
	[BLOB_ATTR_STRING] = 1,
	[BLOB_ATTR_INT8] = sizeof(uint8_t),
	[BLOB_ATTR_INT16] = sizeof(uint16_t),
	[BLOB_ATTR_INT32] = sizeof(uint32_t),
	[BLOB_ATTR_INT64] = sizeof(uint64_t),
};

// 数据类型合法性检测
bool
blob_check_type(const void *ptr, unsigned int len, int type)
{
	const char *data = ptr;

	if (type >= BLOB_ATTR_LAST)
		return false;

	if (type >= BLOB_ATTR_INT8 && type <= BLOB_ATTR_INT64) {
		if (len != blob_type_minlen[type])
			return false;
	} else {
		if (len < blob_type_minlen[type])
			return false;
	}

	if (type == BLOB_ATTR_STRING && data[len - 1] != 0)
		return false;

	return true;
}

// 解析当前这级数据的下一级格式
int
blob_parse(struct blob_attr *attr, struct blob_attr **data, const struct blob_attr_info *info, int max)
{
	struct blob_attr *pos;
	int found = 0;
	int rem;

	memset(data, 0, sizeof(struct blob_attr *) * max);
	blob_for_each_attr(pos, attr, rem) {
		int id = blob_id(pos);
		int len = blob_len(pos);

		if (id >= max)
			continue;

		if (info) {
			int type = info[id].type;

			if (type < BLOB_ATTR_LAST) {
				if (!blob_check_type(blob_data(pos), len, type))
					continue;
			}

			if (info[id].minlen && len < info[id].minlen)
				continue;

			if (info[id].maxlen && len > info[id].maxlen)
				continue;

			if (info[id].validate && !info[id].validate(&info[id], pos))
				continue;
		}

		if (!data[id])
			found++;

		data[id] = pos;
	}
	return found;
}

// 比较两个属性空间
bool
blob_attr_equal(const struct blob_attr *a1, const struct blob_attr *a2)
{
	if (!a1 && !a2)
		return true;

	if (!a1 || !a2)
		return false;

	if (blob_pad_len(a1) != blob_pad_len(a2))
		return false;

	return !memcmp(a1, a2, blob_pad_len(a1));
}

// 将整个属性空间拷贝一份到新申请的内存
struct blob_attr *
blob_memdup(struct blob_attr *attr)
{
	struct blob_attr *ret;
	int size = blob_pad_len(attr);

	ret = malloc(size);
	if (!ret)
		return NULL;

	memcpy(ret, attr, size);
	return ret;
}
