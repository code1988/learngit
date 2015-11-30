/**************************************************************************
注意点：
		结构体指针memdevp指向具体的memdevp[i]时，memdevp[i]不再是指向结构体的地址值，而是结构体本身，所以下面在作为memdev[i]引用时，不能使用->指向其成员，切记！！！
		休眠函数wait_event_interruptible(xxx_queue,have_data)，传递的不是地址值，此为特例，切记！！！
		非阻塞函数mem_poll中的标志位mask定义时一定要进行初始化，切记！！！
		文件指针定位函数mem_llseek中的偏移值变量ret在switch结构中的default时必须赋值，否则需要在定义时初始化
**************************************************************************/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/poll.h>

#include "memdev.h"

int major_no=MEMDEV_MAJOR;
struct cdev cdev;
struct memdev *memdevp;
struct file_operations fops={
	.owner = THIS_MODULE,
	.read = mem_read,
	.write = mem_write,
	.open = mem_open,
	.release = mem_release,
	.llseek = mem_llseek,
	.poll = mem_poll,
};
bool have_data=false;


static int memdev_init(void)
{
	unsigned char i;
	int result;
	dev_t dev = MKDEV(major_no,0);
	
	if(major_no)
		register_chrdev_region(dev,MEMDEV_NR_DEVS,"memdev");
	else
	{
		alloc_chrdev_region(&dev,0,MEMDEV_NR_DEVS,"memdev");
		major_no = MAJOR(dev);	
	}
	
	cdev_init(&cdev,&fops);
	cdev.owner = THIS_MODULE;
	cdev.ops = &fops;
	
	cdev_add(&cdev,MKDEV(major_no,0),MEMDEV_NR_DEVS);
	
	memdevp = kmalloc(MEMDEV_NR_DEVS*sizeof(struct memdev),GFP_KERNEL);
	memset(memdevp,0,sizeof(struct memdev));
	if(!memdevp)
	{
		result = -ENOMEM;
		goto fail;	
	}	
	for(i=0;i<MEMDEV_NR_DEVS;i++)
	{
		memdevp[i].size = MEMDEV_SIZE;
		memdevp[i].data = kmalloc(MEMDEV_SIZE,GFP_KERNEL);
		memset(memdevp[i].data,0,sizeof(struct memdev));	
		init_waitqueue_head(&memdevp[i].read_queue);
		init_waitqueue_head(&memdevp[i].write_queue);
		init_MUTEX(&memdevp[i].sem);
		printk("the device %s%d is installed\n","memdev",i);
	}
	printk("the device major_no is %d\n",major_no);
	
	fail:unregister_chrdev_region(dev,2);
	return 0;	
}

static int mem_open(struct inode *inode,struct file *filp)
{
	int minor=MINOR(inode->i_rdev);
	struct memdev *devp = &memdevp[minor];
	 
	filp->private_data = devp;
	
	return 0;
}

static int mem_release(struct inode *inode,struct file *filp)
{
	return 0;	
}

static ssize_t mem_read(struct file *filp,char __user *buf,size_t size,loff_t *ppos)
{
	struct memdev *devp = filp->private_data;
	unsigned long p=*ppos;
	unsigned int counter=size;
	
	printk("in mem_read()\n");
	if(p>MEMDEV_SIZE)
		return 0;
	if(counter>(MEMDEV_SIZE-p))
		counter = MEMDEV_SIZE-p;
	
	down_interruptible(&devp->sem);
	
	while(!have_data)
	{
		up(&devp->sem);
		if(filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		wait_event_interruptible(devp->read_queue,have_data);
		down_interruptible(&devp->sem);	
	}
	
	copy_to_user(buf,(void*)devp->data,counter);
	*ppos += counter;
	up(&devp->sem);
	
	have_data = false;
	wake_up_interruptible(&devp->write_queue);
	printk("out of mem_read()\n");
	return counter;
}

static ssize_t mem_write(struct file *filp,char const __user *buf,size_t size,loff_t *ppos)
{
	unsigned long p=*ppos;
	unsigned int counter=size;
	struct memdev *devp = filp->private_data;
	
	printk("in mem_write()\n");
	if(p>MEMDEV_SIZE)
		return 0;
	if(counter>(MEMDEV_SIZE-p))
		counter = MEMDEV_SIZE-p;
	
	down_interruptible(&devp->sem);
	
	while(have_data)
	{
		up(&devp->sem);	
		if(filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		wait_event_interruptible(devp->write_queue,!have_data);
		down_interruptible(&devp->sem);
	}
	
	copy_from_user(devp->data,buf,counter);
	*ppos += counter;
	up(&devp->sem);
	
	have_data = true;
	wake_up_interruptible(&devp->read_queue);
	printk("out of mem_write()\n");
	return counter;	
}

static loff_t mem_llseek(struct file *filp,loff_t offest,int where)
{
	loff_t ret;
	
	switch(where)
	{
		case 0:
			ret = offest;
		break;
		case 1:
			ret = filp->f_pos + offest;
		break;
		case 2:
			ret = MEMDEV_SIZE -1-offest;
		default:
		return -EINVAL;	
	}		
	filp->f_pos = ret;
	return ret;
}

static unsigned int mem_poll(struct file *filp,poll_table *wait)
{
	unsigned int mask=0;
	struct memdev *devp = filp->private_data;
	
	down(&devp->sem);
	poll_wait(filp,&devp->read_queue,wait);
	poll_wait(filp,&devp->write_queue,wait);
	
	if(have_data)
	{
		mask |= POLLIN|POLLRDNORM;
		printk("the device can be read\n");	
	}
	if(!have_data)
	{
		mask |= POLLOUT|POLLWRNORM;
		printk("the device can be write\n");	
	}
	up(&devp->sem);
	return mask;
}

static void mem_exit(void)
{
	kfree(memdevp);
	cdev_del(&cdev);
	unregister_chrdev_region(MKDEV(major_no,0),MEMDEV_NR_DEVS);	
}

module_init(memdev_init);
module_exit(mem_exit);

MODULE_LICENSE("Dual BSD/GPL");