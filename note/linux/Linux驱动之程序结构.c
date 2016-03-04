#include <linux/init.h>
#include <linux/module.h>

// 模块许可证声明（必须）
MODULE_LISENSE("Dual BSD/GPL");
MODULE_AUTHOR("CODE");

static char *name = "CODE";
static int val = 1;
static int hello;

// 模块加载函数（必须）
// 函数名可以自定义，通过模块加载宏注册
// __init 宏，告诉内核该函数只会在加载时使用
static int __init hello_init(void)
{
	int i;
	for(i = 0;i < val;i++)
		printk(KERN_ALERT "Hello %s!\r\n",name);
	return 0;
}

// 模块卸载函数（必须）
// 函数名可以自定义，通过模块卸载宏注册
// __exit 宏，高数内核该函数只会在卸载时使用
static void __exit hello_exit(void)
{
	printk(KERN_ALERT "Byebye %s",name);	
}

// 模块加载宏（必须）
module_init(hello_init);	// insmod命令加载驱动时，宏内的加载函数会自动被内核运行，完成驱动的初始化

// 模块卸载宏（必须）
module_exit(hello_eixt);	// rmmod命令卸载驱动时，宏内的卸载函数会自动被内核运行，完成驱动的卸载

// 模块参数（可选）
module_param(name,charp,S_IRUGO);	// 驱动被加载时，可以被传入的参数，这些参数对应驱动内部的全局变量
module_param(val,int,S_IRUGO);		// 参数1 - 参数名
									// 参数2 - 参数类型，包括byte、short、ushort、int、uint、long、ulong、charp、bool
									// 参数3 - 参数读/写权限，包括S_IRUGO、S_IWUGO、S_IXUGO、S_IRWXUGO、S_IALLUGO

// 模块导出符号（可选）
EXPORT_SYMBOL(hello);		// 驱动模块可以导出函数/变量到内核，这样其他模块就可以使用这些导出的符号
							// 符号必须在驱动的全局部分导出，不能在函数部分导出								