#include <linux/init.h>
#include <linux/module.h>

// ģ�����֤���������룩
MODULE_LISENSE("Dual BSD/GPL");
MODULE_AUTHOR("CODE");

static char *name = "CODE";
static int val = 1;
static int hello;

// ģ����غ��������룩
// �����������Զ��壬ͨ��ģ����غ�ע��
// __init �꣬�����ں˸ú���ֻ���ڼ���ʱʹ��
static int __init hello_init(void)
{
	int i;
	for(i = 0;i < val;i++)
		printk(KERN_ALERT "Hello %s!\r\n",name);
	return 0;
}

// ģ��ж�غ��������룩
// �����������Զ��壬ͨ��ģ��ж�غ�ע��
// __exit �꣬�����ں˸ú���ֻ����ж��ʱʹ��
static void __exit hello_exit(void)
{
	printk(KERN_ALERT "Byebye %s",name);	
}

// ģ����غ꣨���룩
module_init(hello_init);	// insmod�����������ʱ�����ڵļ��غ������Զ����ں����У���������ĳ�ʼ��

// ģ��ж�غ꣨���룩
module_exit(hello_eixt);	// rmmod����ж������ʱ�����ڵ�ж�غ������Զ����ں����У����������ж��

// ģ���������ѡ��
module_param(name,charp,S_IRUGO);	// ����������ʱ�����Ա�����Ĳ�������Щ������Ӧ�����ڲ���ȫ�ֱ���
module_param(val,int,S_IRUGO);		// ����1 - ������
									// ����2 - �������ͣ�����byte��short��ushort��int��uint��long��ulong��charp��bool
									// ����3 - ������/дȨ�ޣ�����S_IRUGO��S_IWUGO��S_IXUGO��S_IRWXUGO��S_IALLUGO

// ģ�鵼�����ţ���ѡ��
EXPORT_SYMBOL(hello);		// ����ģ����Ե�������/�������ںˣ���������ģ��Ϳ���ʹ����Щ�����ķ���
							// ���ű�����������ȫ�ֲ��ֵ����������ں������ֵ���								