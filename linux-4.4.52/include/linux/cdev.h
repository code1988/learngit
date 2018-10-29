#ifndef _LINUX_CDEV_H
#define _LINUX_CDEV_H

#include <linux/kobject.h>
#include <linux/kdev_t.h>
#include <linux/list.h>

struct file_operations;
struct inode;
struct module;

// 用于描述一个字符设备的结构 
struct cdev {
	struct kobject kobj;                // 该字符设备关联的kobject对象
	struct module *owner;
	const struct file_operations *ops;  // 该字符设备关联的文件形式操作接口集合
	struct list_head list;
	dev_t dev;                          // 分配给该字符设备的起始设备号
	unsigned int count;                 // 分配给该字符设备的连续设备号长度
};

void cdev_init(struct cdev *, const struct file_operations *);

struct cdev *cdev_alloc(void);

void cdev_put(struct cdev *p);

int cdev_add(struct cdev *, dev_t, unsigned);

void cdev_del(struct cdev *);

void cd_forget(struct inode *);

#endif
