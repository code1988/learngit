/*****************************************************************************************************************
										初始化字符设备驱动
*****************************************************************************************************************/	
// 字符设备驱动控制块定义如下：
struct cdev{
	struct kobject kobj;				// 嵌入了一个操作内核对象的通用接口
	struct module *owner;				// 所属模块
	const struct file_operations *ops;	// 定义了一组文件操作的函数指针，实现了跟该设备通信的具体操作
	struct list_head list;				// 嵌入一个双向循环链表模块以实现链表操作，linux链表模块另作分析
	dev_t dev;							// 设备号
	unsigned int count;					// 设备号范围
};

cdev结构有2种初始化定义方式：动态、静态
	// 静态方式
	static struct cdev my_cdev;
		
	cdev_init(&my_cdev,&my_fops);
	my_cdev.owner = THIS_MODULE;
	
	void cdev_init(struct cdev *cdev, const struct file_operations *fops)
	{
		memset(cdev, 0, sizeof *cdev);					// cdev内存区清零
		INIT_LIST_HEAD(&cdev->list);					// 生成一个双向循环链表头节点
		kobject_init(&cdev->kobj, &ktype_cdev_default);	// 初始化kobject结构
		cdev->ops = fops;								// 建立file_operations与cdev的连接
	}	
	
	// 动态方式
	struct cdev *my_cdev = cdev_alloc();
	my_cdev->owner = THIS_MODULE;
	my_cdev->ops = &my_fops;		// 将file_operations与cdev关联
	
	struct cdev *cdev_alloc(void)
	{
		// 分配cdev内存区并完成清零
		struct cdev *p = kzalloc(sizeof(struct cdev), GFP_KERNEL);
		if (p) 
		{
			INIT_LIST_HEAD(&p->list);						// 生成一个双向循环链表头节点
			kobject_init(&p->kobj, &ktype_cdev_dynamic);	// 建立file_operations与cdev的连接
		}
		
		// 返回分配成功的cdev
		return p;
	}
	
	
/*****************************************************************************************************************
										注册字符设备驱动
*****************************************************************************************************************/	
// 参数*p 		cdev结构的指针
// 参数dev		起始设备编号
// 参数count	设备编号范围		 						
int cdev_add(struct cdev *p, dev_t dev, unsigned count)
{
	p->dev = dev;
	p->count = count;
	
	// 把字符设备编号和cdev一起保存到cdev_map散列表里
	return kobj_map(cdev_map, dev, count, NULL, exact_match, exact_lock, p);
}

备注：
	对系统而言，成功调用了cdev_add之后，就意味着一个字符设备对象已经加入到了系统，在需要的时候，系统就可以找到它
	对用户态的程序而言，成功调用了cdev_add之后，就已经可以通过文件系统的接口呼叫到驱动程序
	
	
/*****************************************************************************************************************
										file_operations结构
*****************************************************************************************************************/	
file_operations就是把系统调用和驱动程序关联起来的关键数据结构，是一系列指针的集合
Linux设备驱动程序工作的基本原理如下：
	1. 用户进程利用系统调用对设备文件进行read/write等操作
	2. 系统调用通过设备文件的主设备号找到相应的设备驱动cdev
	3. 读取和cdev关联的file_operations中相应的函数指针
	4. 调用该函数

所以，编写设备驱动程序的主要工作就是填充file_operations的各个成员，其定义如下：
struct file_operations {
	struct module *owner;																// 指向拥有这个结构的模块的指针，用于它的操作还在被使用时阻止模块被卸载，一般被初始化为THIS_MODULE	
	
	loff_t (*llseek) (struct file *, loff_t, int);										// 改变文件中的当前读写位置	
																						// 参数loff_t - 成功时返回新位置（正值），失败时返回负值

	ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);					// 从设备中获取数据（同步读）
																						// 参数ssize_t - 成功时返回读取的字节数（正值），失败时返回-EINVAL
																						
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);			// 向设备中写入数据（同步写）
																						// 参数ssize_t - 成功时返回写入的字节数（正值），失败时返回-EINVAL
																						
	ssize_t (*aio_read) (struct kiocb *, const struct iovec *, unsigned long, loff_t);	// 从设备总获取数据（异步读）
	ssize_t (*aio_write) (struct kiocb *, const struct iovec *, unsigned long, loff_t);	// 向设备中写入数据（异步写）
	
	int (*readdir) (struct file *, void *, filldir_t);									// 仅对文件系统有效，对于设备文件，该指针必须为NULL
	
	unsigned int (*poll) (struct file *, struct poll_table_struct *);					// poll对应3个系统调用：poll、epoll、select，用于查询对一个或多个文件描述符是否可以进行非阻塞的读取或写入
	
	int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);			// 用于提供设备相关控制命令的实现（既非读也非写），内核本身识别部分控制命令，而不必调用驱动中的ioctl（）
																						// 对于内核不能识别的命令，驱动中又没有实现ioctl（），用户进行ioctl（）系统调用时将获得-EINVAL返回值
																						
	long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
	long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
	
	int (*mmap) (struct file *, struct vm_area_struct *);								// 用于请求将设备内存映射到进程地址空间
	int (*open) (struct inode *, struct file *);
	int (*flush) (struct file *, fl_owner_t id);
	int (*release) (struct inode *, struct file *);
	int (*fsync) (struct file *, struct dentry *, int datasync);
	int (*aio_fsync) (struct kiocb *, int datasync);
	int (*fasync) (int, struct file *, int);
	int (*lock) (struct file *, int, struct file_lock *);
	ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int);
	unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
	int (*check_flags)(int);
	int (*flock) (struct file *, int, struct file_lock *);
	ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int);
	ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int);
	int (*setlease)(struct file *, long, struct file_lock **);
};
/*****************************************************************************************************************
										注销字符设备驱动
*****************************************************************************************************************/	
void cdev_del(struct cdev *p)
{
	cdev_unmap(p->dev, p->count);	// 释放cdev_map散列表里的当前对象
	kobject_put(&p->kobj);			// 释放cdev结构本身
}								
										


