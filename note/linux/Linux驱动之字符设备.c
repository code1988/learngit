/*****************************************************************************************************************
										初始化字符设备驱动
*****************************************************************************************************************/	
// 字符设备驱动控制块定义如下：
struct cdev{
	struct kobject kobj;				// 嵌入了一个操作内核对象的通用接口
	struct module *owner;				// 所属模块
	const struct file_operations *ops;	// 定义了一组文件操作的函数指针，实现了跟该设备通信的具体操作
	struct list_head list;				// 嵌入一个链表模块以实现链表操作，linux链表模块另作分析
	dev_t dev;							// 设备号
	unsigned int count;					// 设备号范围
};

cdev结构有2种初始化定义方式：动态、静态
	static struct file_operations my_fops = {
		.owner = THIS_MODULE,  
	    .llseek = mychar_llseek,  
	    .read = mychar_read,  
	    .write = mychar_write,  
	    .ioctl = mychar_ioctl,  
	    .open = mychar_open,  
	    .release =mychar_release,  
		}
		
	// 静态方式
	static struct cdev my_cdev;
		
	cdev_init(&my_cdev,&my_fops);
	my_cdev.owner = THIS_MODULE;
	
	void cdev_init(struct cdev *cdev, const struct file_operations *fops)
	{
		memset(cdev, 0, sizeof *cdev);					// cdev内存区清零
		INIT_LIST_HEAD(&cdev->list);					
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
			INIT_LIST_HEAD(&p->list);
			kobject_init(&p->kobj, &ktype_cdev_dynamic);	// 建立file_operations与cdev的连接
		}
		
		// 返回分配成功的cdev
		return p;
	}
	
/*****************************************************************************************************************
										注册字符设备驱动
*****************************************************************************************************************/				 						
int cdev_add(struct cdev *p, dev_t dev, unsigned count)
{
	p->dev = dev;
	p->count = count;
	return kobj_map(cdev_map, dev, count, NULL, exact_match, exact_lock, p);
}

										
										注销字符设备编号
/*****************************************************************************************************************
相应的，内核提供了两个注销字符设备编号范围的函数，分别如下：
1.新式注销函数
void unregister_chrdev_region(dev_t from,unsigned int count)
{
	dev_t to = from + count;
	dev_t n,next;
	
	for(n=from;n<to;n=next)
	{
		next = MKDEV(MAJOR(n) + 1,0);
		
		if(next > to)
		{
			next = to;
		}
		
		kfree(__unregister_chrdev_region(MAJOR(n),MINOR(n),next - n));
	}
}

2.老式注销函数
void unregister_chrdev(unsigned int major,const *name)
{
	struct char_device_struct cd;
	
	cd = __unregister_chrdev_region(major,0,256);
	
	if(cd & cd->cdev)
		cdev_del(cd->cdev);
		
	kfree(cd);
}

备注：两个注销函数最终都调用了__unregister_chrdev_region()函数，该函数暂略
*****************************************************************************************************************/
