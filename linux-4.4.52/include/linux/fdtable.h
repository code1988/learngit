/*
 * descriptor table internals; you almost certainly want file.h instead.
 */

#ifndef __LINUX_FDTABLE_H
#define __LINUX_FDTABLE_H

#include <linux/posix_types.h>
#include <linux/compiler.h>
#include <linux/spinlock.h>
#include <linux/rcupdate.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/fs.h>

#include <linux/atomic.h>

/*
 * The default fd array needs to be at least BITS_PER_LONG,
 * as this is the granularity returned by copy_fdset().
 */
#define NR_OPEN_DEFAULT BITS_PER_LONG

// 定义了fd表结构，用于维护一个进程所有打开文件fd的信息
struct fdtable {
	unsigned int max_fds;           // 当前支持的打开fd数量上限，默认是NR_OPEN_DEFAULT，可动态扩增
	struct file __rcu **fd;      /* current fd array  
                                    指向当前有效的file表，这张表维护了该进程所有打开文件的信息
                                    默认指向fd_array */
	unsigned long *close_on_exec;   // 指向当前有效的设置了close-on-exec标识的fd位图，默认指向close_on_exec_init
	unsigned long *open_fds;        // 指向当前有效的所有打开文件fd位图，默认指向open_fds_init
	unsigned long *full_fds_bits;   
	struct rcu_head rcu;
};

static inline bool close_on_exec(int fd, const struct fdtable *fdt)
{
	return test_bit(fd, fdt->close_on_exec);
}

static inline bool fd_is_open(int fd, const struct fdtable *fdt)
{
	return test_bit(fd, fdt->open_fds);
}

/*
 * Open file table structure
 * 定义了文件表结构，用于维护一个进程所有打开文件的信息
 */
struct files_struct {
  /*
   * read mostly part
   */
	atomic_t count;                 // 该文件表的引用计数
	bool resize_in_progress;
	wait_queue_head_t resize_wait;

    /* 只有当该进程打开文件数量超过默认上限时，才会动态申请内存
     * 这是内核常用的一种优化策略，目的是避免频繁申请内存
     */
	struct fdtable __rcu *fdt;      // 指向当前有效的fd表，默认指向fdtab
	struct fdtable fdtab;           // 默认使用的fd表，一旦超出容量则改用动态分配
  /*
   * written part on a separate cache line in SMP
   * 将频繁写区域的起始位置按照cacheline对齐，使得文件表结构的上下两部分分别位于不同的cacheline
   * 这是内核常用的一种优化策略，目的是尽量减少多CPU同时访问同一文件表时的fail share问题
   */
	spinlock_t file_lock ____cacheline_aligned_in_smp;  // 用于维护该文件表的自旋锁
	int next_fd;                                        // 下一个空闲的fd
	unsigned long close_on_exec_init[1];                // 默认使用的设置了close-on-exec标识的fd位图，一旦超出容量则改用动态分配
	unsigned long open_fds_init[1];                     // 默认使用的所有打开文件fd位图，一旦超出容量则改用动态分配
	unsigned long full_fds_bits_init[1];
	struct file __rcu * fd_array[NR_OPEN_DEFAULT];      // 默认使用的file表，一旦超出容量则改用动态分配
};

struct file_operations;
struct vfsmount;
struct dentry;

#define rcu_dereference_check_fdtable(files, fdtfd) \
	rcu_dereference_check((fdtfd), lockdep_is_held(&(files)->file_lock))

#define files_fdtable(files) \
	rcu_dereference_check_fdtable((files), (files)->fdt)

/*
 * The caller must ensure that fd table isn't shared or hold rcu or file lock
 */
static inline struct file *__fcheck_files(struct files_struct *files, unsigned int fd)
{
	struct fdtable *fdt = rcu_dereference_raw(files->fdt);

	if (fd < fdt->max_fds)
		return rcu_dereference_raw(fdt->fd[fd]);
	return NULL;
}

static inline struct file *fcheck_files(struct files_struct *files, unsigned int fd)
{
	RCU_LOCKDEP_WARN(!rcu_read_lock_held() &&
			   !lockdep_is_held(&files->file_lock),
			   "suspicious rcu_dereference_check() usage");
	return __fcheck_files(files, fd);
}

/*
 * Check whether the specified fd has an open file.
 */
#define fcheck(fd)	fcheck_files(current->files, fd)

struct task_struct;

struct files_struct *get_files_struct(struct task_struct *);
void put_files_struct(struct files_struct *fs);
void reset_files_struct(struct files_struct *);
int unshare_files(struct files_struct **);
struct files_struct *dup_fd(struct files_struct *, int *);
void do_close_on_exec(struct files_struct *);
int iterate_fd(struct files_struct *, unsigned,
		int (*)(const void *, struct file *, unsigned),
		const void *);

extern int __alloc_fd(struct files_struct *files,
		      unsigned start, unsigned end, unsigned flags);
extern void __fd_install(struct files_struct *files,
		      unsigned int fd, struct file *file);
extern int __close_fd(struct files_struct *files,
		      unsigned int fd);

extern struct kmem_cache *files_cachep;

#endif /* __LINUX_FDTABLE_H */
