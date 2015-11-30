#ifndef __memdev_h__
#define __memdev_h__

struct memdev{
	unsigned char *data;
	unsigned int size;
	wait_queue_head_t read_queue;
	wait_queue_head_t write_queue;
	struct semaphore sem;
};

#ifndef MEMDEV_MAJOR
#define MEMDEV_MAJOR 0
#endif

#ifndef	MEMDEV_NR_DEVS
#define MEMDEV_NR_DEVS 2
#endif

#ifndef MEMDEV_SIZE 
#define MEMDEV_SIZE 4096
#endif

static int memdev_init(void);
static int mem_open(struct inode *inode,struct file *filp);
static int mem_release(struct inode *inode,struct file *filp);
static ssize_t mem_read(struct file *filp,char __user *buf,size_t size,loff_t *ppos);
static ssize_t mem_write(struct file *filp,char const __user *buf,size_t size,loff_t *ppos);
static loff_t mem_llseek(struct file *filp,loff_t offest,int where);
static unsigned int mem_poll(struct file *filp,poll_table *wait);
static void mem_exit(void);

#endif
