											设备号
/*****************************************************************************************************************
设备号用来标识一个设备，包含主设备号和次设备号
相关宏定义如下：
#define MINORBITS	20
#define MINORMASK	((1 << MINORBITS) - 1)	

#define MKDEV(major,minor)	((major << MINORBITS) | minor)		// 将主设备号和次设备号转换成dev_t类型，即高12位为主设备号，低20位为次设备号
#define MAJOR(dev)			((unsigned int)(dev >> MINORBITS))	// 从dev_t类型中提取主设备号
#define MINOR(dev)			((unsigned int)(dev & MINORBITS))	// 从dev_t类型中提取次设备号
*****************************************************************************************************************/

/*****************************************************************************************************************											
											注册字符设备
*****************************************************************************************************************/
一. 注册字符设备编号： 内核提供了三个函数来注册一组字符设备编号，分别如下：
	1.静态申请和注册设备 (可以注册任意一段主设备号范围，每个主设备号包含了20位数量的次设备号)
	int register_chrdev_region(dev_t from,unsigned int count,const char *name)
	{
		struct char_device_struct *cd;
		dev_t to = from + count;	// 意味着可用的设备号范围from~to
		dev_t n,next;
		
		for(n=from;n<to;n=next)
		{
			// 主设备号循环自增
			next = MKDEV(MAJOR(n) + 1,0);
			
			// 当count=0时才会出现的情况
			if(next > to)
				next = to;
			
			// 注册，次设备号最大数量为2^20,远超老式的256个限制
			cd = __register_chrdev_region(MAJOR(n),MINOR(n),next - n,name);
			
			// 如果有一次分配失败，会把之前成功分配的全部释放
			if(IS_ERR(cd))
				goto fail;
		}
		
		return 0;
		
		fail:
			to = n;
			for(n=from;n<to;n=next)
			{
				next = MKDEV(MAJOR(n) + 1,0);
				kfree(__unregister_chrdev_region(MAJOR(n),MINOR(n),next - n));
			}
			
			return PTR_ERR(cd);
	}

	2.动态申请和注册设备(注册的设备主设备号内核动态分配，次设备号范围0~2^20任取)
	int alloc_chrdev_region(dev_t *dev,unsigned int baseminor,unsigned int count,const char *name)
	{
		struct char_device_struct *cd;
		
		// 注册
		cd = __register_chrdev_region(0,baseminor,count,name);
		
		if(IS_ERR(cd))
			return PTR_ERR(cd);
			
		// 生成dev_t类型设备号
		*dev = MKDEV(cd->major,cd->baseminor);
		
		return 0;
	}

	3.老式的申请和注册设备(可以注册任意一个主设备号，包含了256个次设备号)
	int register_chrdev(unsigned int major,const char *name,const struct file_operations *fops)
	{
		struct char_device_struct *cd;
		struct cdev *cdev;
		char *s;
		int err = -ENOMEM;
		
		// 注册，次设备号固定256个
		cd = __register_chrdev_region(major,0,256,name);
		
		if(IS_ERR(cd))
			return PTR_ERR(cd);
			
		cdev = cdev_alloc();
		if(!cdev)
			goto out2;
			
		cdev->owner = fops->owner;
		cdev->ops = fops;
		kobject_set_name(&cdev->kobj,"%s",name);
		
		for(s=strchr(kobject_name(&cdev->kobj),'/');s;s=strchr(s,'/'))
		{
			*s = '!';
		}
		
		err = cdev_add(cdev,MKDEV(cd->major,0),256);
		if(err)
			goto out;
			
		cd->cdev = cdev;
		
		return major?0:cd->major;
		
		out:
			kobject_put(&cdev->kobj);
			
		out2:
			kfree(__unregister_chrdev_region(cd->major,0,256));
			
		return err;
	}

	备注：三个注册函数最终都调用了__register_chrdev_region()函数，完成(一个主设备号 + 一段次设备号 )的注册
	
	注册设备号的实质：
		内核中所有已分配的字符设备编号都记录在一个全局的结构体数组指针chrdevs中，数组长度255
		该数组每个元素存放的是一个地址，该地址指向一张链表表头，每张链表的单元都是一个char_device_struct结构
		一个主设备号占用一个数组元素，也就是最多支持分配255个主设备号，其下的次设备号组成一张链表
	
	// 字符设备控制块定义如下：
	struct char_device_struct
	{
		struct char_device_struct *next;	// 指向下一段次设备号区域
		unsigned int major;					// 主设备号
		unsigned int baseminor;				// 起始次设备号
		int minorct;						// 次设备号范围
		char name[64];						// 处理该次设备编号范围内的设备驱动的名称
		struct file_operations *fops;		// 老式的register_chrdev中使用
		struct cdev *cdev;					// 指向字符设备驱动描述符的指针
	}*chrdevs[CHRDEV_MAJOR_HASH_SIZE];
	
	__register_chrdev_region()函数的工作就是申请一个字符设备控制块，然后注册到chrdevs数组的对应链表中
	struct char_device_struct * __register_chrdev_region(unsigned int major, unsigned int baseminor, int minorct, const char *name)
	{
		struct char_device_struct *cd, **cp;
		int ret = 0;
		int i;
		
		// 申请一个字符设备控制块
		cd = kzalloc(sizeof(struct char_device_struct), GFP_KERNEL);
		if (cd == NULL)
			return ERR_PTR(-ENOMEM);
		
		// 上锁
		mutex_lock(&chrdevs_lock);
		
		// 针对动态申请方式，这里获取一个未被使用的主设备号
		if (major == 0) 
		{
			for (i = ARRAY_SIZE(chrdevs)-1; i > 0; i--) 
			{
				if (chrdevs[i] == NULL)
					break;
			}
			if (i == 0) 
			{
				ret = -EBUSY;
				goto out;
			}
			major = i;
			ret = major;
		}
		
		// 填充新申请的字符设备控制块各字段
		cd->major = major;
		cd->baseminor = baseminor;
		cd->minorct = minorct;
		strlcpy(cd->name, name, sizeof(cd->name));
		
		// 根据主设备号找到chrdevs数组中相应的链表头，继而为新申请的字符设备控制块找到合适的插入点（这里当主设备号相同时，对于链表节点的插入管理代码可能存在问题）
		i = major_to_index(major);
		for (cp = &chrdevs[i]; *cp; cp = &(*cp)->next)
			if ((*cp)->major > major ||((*cp)->major == major && (((*cp)->baseminor >= baseminor) || ((*cp)->baseminor + (*cp)->minorct > baseminor))))
				break;
		
		// 对于主设备号相同的插入节点，需要做重叠区域检测		
		if (*cp && (*cp)->major == major) 
	    {
			int old_min = (*cp)->baseminor;
			int old_max = (*cp)->baseminor + (*cp)->minorct - 1;
			int new_min = baseminor;
			int new_max = baseminor + minorct - 1;
	
			// 新次设备号跟老次设备号头重叠，则非法
			if (new_max >= old_min && new_max <= old_max) {
				ret = -EBUSY;
				goto out;
			}
			// 新次设备号跟老次设备号尾重叠，则非法
			if (new_min <= old_max && new_min >= old_min) {
				ret = -EBUSY;
				goto out;
			}
		}
		
		// 插入链表
		cd->next = *cp;
		*cp = cd;
	
	    // 解锁
		mutex_unlock(&chrdevs_lock);
		
		// 返回新的字符设备控制块地址
		return cd;
out:
		mutex_unlock(&chrdevs_lock);
		kfree(cd);
		return ERR_PTR(ret);
	}
二. 初始化字符设备驱动
	// 字符设备驱动控制块定义如下：
	struct cdev{
		struct kobject kobj;
		struct module *owner;
		const struct file_operations *ops;
		struct list_head list;
		dev_t dev;
		unsigned int count;
	};
	
/*****************************************************************************************************************
										字符设备驱动
*****************************************************************************************************************/										


										
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
