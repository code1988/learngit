/*
 * Copyright (C) 2010-2012 Felix Fietkau <nbd@openwrt.org>
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
#include "blobmsg.h"

// blob层-->blobmsg层消息类型转换
static const int blob_type[__BLOBMSG_TYPE_LAST] = {
	[BLOBMSG_TYPE_INT8] = BLOB_ATTR_INT8,
	[BLOBMSG_TYPE_INT16] = BLOB_ATTR_INT16,
	[BLOBMSG_TYPE_INT32] = BLOB_ATTR_INT32,
	[BLOBMSG_TYPE_INT64] = BLOB_ATTR_INT64,
	[BLOBMSG_TYPE_STRING] = BLOB_ATTR_STRING,
	[BLOBMSG_TYPE_UNSPEC] = BLOB_ATTR_BINARY,
};

// 获取消息名长度
static uint16_t
blobmsg_namelen(const struct blobmsg_hdr *hdr)
{
	return be16_to_cpu(hdr->namelen);
}

// 消息合法性检测
bool blobmsg_check_attr(const struct blob_attr *attr, bool name)
{
	const struct blobmsg_hdr *hdr;
	const char *data;
	int id, len;

    // 检测消息长度
	if (blob_len(attr) < sizeof(struct blobmsg_hdr))
		return false;

    // 如果存在消息名，则需要检测消息名相关参数
	hdr = (void *) attr->data;
	if (!hdr->namelen && name)
		return false;

	if (blobmsg_namelen(hdr) > blob_len(attr) - sizeof(struct blobmsg_hdr))
		return false;

	if (hdr->name[blobmsg_namelen(hdr)] != 0)
		return false;

	id = blob_id(attr);
	len = blobmsg_data_len(attr);
	data = blobmsg_data(attr);

    // 检测消息类型
	if (id > BLOBMSG_TYPE_LAST)
		return false;

	if (!blob_type[id])
		return true;

	return blob_check_type(data, len, blob_type[id]);
}

// array/table类型消息的合法性检测(成功返回子消息个数)
int blobmsg_check_array(const struct blob_attr *attr, int type)
{
	struct blob_attr *cur;
	bool name;
	int rem;
	int size = 0;

	switch (blobmsg_type(attr)) {
	case BLOBMSG_TYPE_TABLE:
		name = true;
		break;
	case BLOBMSG_TYPE_ARRAY:
		name = false;
		break;
	default:
		return -1;
	}

    // 遍历父消息中的成员
	blobmsg_for_each_attr(cur, attr, rem) {
		if (type != BLOBMSG_TYPE_UNSPEC && blobmsg_type(cur) != type)
			return -1;

        // 对子消息作合法性检测
		if (!blobmsg_check_attr(cur, name))
			return -1;

		size++;
	}

	return size;
}

// array/table类型消息的合法性检测（成功返回1）
bool blobmsg_check_attr_list(const struct blob_attr *attr, int type)
{
	return blobmsg_check_array(attr, type) >= 0;
}

// 平行解析消息数组，根据指定的消息策略过滤，通过的消息存储得到指定数组tb
int blobmsg_parse_array(const struct blobmsg_policy *policy, int policy_len,
			struct blob_attr **tb, void *data, unsigned int len)
{
	struct blob_attr *attr;
	int i = 0;

	memset(tb, 0, policy_len * sizeof(*tb));
	__blob_for_each_attr(attr, data, len) {
		if (policy[i].type != BLOBMSG_TYPE_UNSPEC &&
		    blob_id(attr) != policy[i].type)
			continue;

		if (!blobmsg_check_attr(attr, false))
			return -1;

		if (tb[i])
			continue;

		tb[i++] = attr;
		if (i == policy_len)
			break;
	}

	return 0;
}

/* 解析父消息的下一级消息，根据指定的消息策略过滤，通过的子消息存储到指定数组tb
 * 由于有长度入参policy_len，所以本函数的使用场景是下一级消息数量为固定值的情况
 */
int blobmsg_parse(const struct blobmsg_policy *policy, int policy_len,
                  struct blob_attr **tb, void *data, unsigned int len)
{
	struct blobmsg_hdr *hdr;
	struct blob_attr *attr;
	uint8_t *pslen;         // 存储策略名长度
	int i;

	memset(tb, 0, policy_len * sizeof(*tb));

    // 申请的空间用于记录每个策略名称的长度,alloca是在栈上申请的空间，所以当前函数返回时自动释放内存
	pslen = alloca(policy_len);

    // 消息策略名称合法性检测,记录策略名长度
	for (i = 0; i < policy_len; i++) {
		if (!policy[i].name)
			continue;

		pslen[i] = strlen(policy[i].name);
	}

    // 遍历blob_attr结构，根据策略中的规则进行过滤
	__blob_for_each_attr(attr, data, len) {
        // 获取blob_attr数据区指针
		hdr = blob_data(attr);
		for (i = 0; i < policy_len; i++) {
            // 判断策略名是否存在
			if (!policy[i].name)
				continue;

            // 判断消息类型是否符合策略
			if (policy[i].type != BLOBMSG_TYPE_UNSPEC &&
			    blob_id(attr) != policy[i].type)
				continue;

            // 判断消息名长度是否符合策略
			if (blobmsg_namelen(hdr) != pslen[i])
				continue;

			if (!blobmsg_check_attr(attr, true))
				return -1;

			if (tb[i])
				continue;

            // 判断消息名是否匹配
			if (strcmp(policy[i].name, (char *) hdr->name) != 0)
				continue;

            // 将过滤后的消息放入目标数组
			tb[i] = attr;
		}
	}

	return 0;
}


// 在当前消息后新增一条消息
static struct blob_attr *
blobmsg_new(struct blob_buf *buf, int type, const char *name, int payload_len, void **data)
{
	struct blob_attr *attr;
	struct blobmsg_hdr *hdr;
	int attrlen, namelen;
	char *pad_start, *pad_end;

	if (!name)
		name = "";

    // 计算新消息的长度
	namelen = strlen(name);
	attrlen = blobmsg_hdrlen(namelen) + payload_len;

    // 当前消息后面开辟一个新的消息空间
	attr = blob_new(buf, type, attrlen);
	if (!attr)
		return NULL;

    // 填充该新消息
	attr->id_len |= be32_to_cpu(BLOB_ATTR_EXTENDED);
	hdr = blob_data(attr);
	hdr->namelen = cpu_to_be16(namelen);
	strcpy((char *) hdr->name, (const char *)name);

    // 对于消息名和消息值之间的空隙（消息名（假如存在）4字节对齐导致）,需要用0填充
	pad_end = *data = blobmsg_data(attr);
	pad_start = (char *) &hdr->name[namelen];
	if (pad_start < pad_end)
		memset(pad_start, 0, pad_end - pad_start);

	return attr;
}

// 计算当前消息指向的属性空间跟缓冲区首地址的偏移量
static inline int
attr_to_offset(struct blob_buf *buf, struct blob_attr *attr)
{
	return (char *)attr - (char *) buf->buf + BLOB_COOKIE;
}

// 开启消息嵌套
void *
blobmsg_open_nested(struct blob_buf *buf, const char *name, bool array)
{
	struct blob_attr *head;
	int type = array ? BLOBMSG_TYPE_ARRAY : BLOBMSG_TYPE_TABLE;
	unsigned long offset = attr_to_offset(buf, buf->head);
	void *data;

	if (!name)
		name = "";

	head = blobmsg_new(buf, type, name, 0, &data);
	if (!head)
		return NULL;
	blob_set_raw_len(buf->head, blob_pad_len(buf->head) - blobmsg_hdrlen(strlen(name)));
	buf->head = head;
	return (void *)offset;
}

// 格式化生成一条字符串类型的消息(可变参数),内部调用，所以这个函数定义为static更合适
void
blobmsg_vprintf(struct blob_buf *buf, const char *name, const char *format, va_list arg)
{
	va_list arg2;
	char cbuf;
	int len;

	va_copy(arg2, arg);
	len = vsnprintf(&cbuf, sizeof(cbuf), format, arg2);
	va_end(arg2);

	vsprintf(blobmsg_alloc_string_buffer(buf, name, len + 1), format, arg);
	blobmsg_add_string_buffer(buf);
}

// 格式化生成一条字符串类型的消息(可变参数)
void
blobmsg_printf(struct blob_buf *buf, const char *name, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	blobmsg_vprintf(buf, name, format, ap);
	va_end(ap);
}

// 新分配一条指定长度的字符串类型消息空间,内部调用，所以这个函数定义为static更合适
void *
blobmsg_alloc_string_buffer(struct blob_buf *buf, const char *name, unsigned int maxlen)
{
	struct blob_attr *attr;
	void *data_dest;

	attr = blobmsg_new(buf, BLOBMSG_TYPE_STRING, name, maxlen, &data_dest);
	if (!attr)
		return NULL;

	blob_set_raw_len(buf->head, blob_pad_len(buf->head) - blob_pad_len(attr));
	blob_set_raw_len(attr, blob_raw_len(attr) - maxlen);

	return data_dest;
}

// 调整字符串消息的数据空间长度
void *
blobmsg_realloc_string_buffer(struct blob_buf *buf, unsigned int maxlen)
{
	struct blob_attr *attr = blob_next(buf->head);
	int offset = attr_to_offset(buf, blob_next(buf->head)) + blob_pad_len(attr) - BLOB_COOKIE;
	int required = maxlen - (buf->buflen - offset);

	if (required <= 0)
		goto out;

	blob_buf_grow(buf, required);
	attr = blob_next(buf->head);

out:
	return blobmsg_data(attr);
}

void
blobmsg_add_string_buffer(struct blob_buf *buf)
{
	struct blob_attr *attr;
	int len, attrlen;

	attr = blob_next(buf->head);
	len = strlen(blobmsg_data(attr)) + 1;

	attrlen = blob_raw_len(attr) + len;
	blob_set_raw_len(attr, attrlen);
	blob_fill_pad(attr);

	blob_set_raw_len(buf->head, blob_raw_len(buf->head) + blob_pad_len(attr));
}

// 在当前消息后面新增一条指定类型的消息，并填充消息名和消息值
int
blobmsg_add_field(struct blob_buf *buf, int type, const char *name,
                  const void *data, unsigned int len)
{
	struct blob_attr *attr;
	void *data_dest;

    // 在当前消息后名新增一条指定类型的消息
	attr = blobmsg_new(buf, type, name, len, &data_dest);
	if (!attr)
		return -1;

    // 填充消息值
	if (len > 0)
		memcpy(data_dest, data, len);

	return 0;
}
