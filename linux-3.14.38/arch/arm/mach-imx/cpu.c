#include <linux/err.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/sys_soc.h>

#include "hardware.h"
#include "common.h"

unsigned int __mxc_cpu_type;    // 记录了IMX家族具体的cpu型号
EXPORT_SYMBOL(__mxc_cpu_type);
unsigned int __mxc_arch_type;
EXPORT_SYMBOL(__mxc_arch_type);

static unsigned int imx_soc_revision;   // 记录了IMX家族soc版本号

void mxc_set_cpu_type(unsigned int type)
{
	__mxc_cpu_type = type;
}

void mxc_set_arch_type(unsigned int type)
{
	__mxc_arch_type = type;
}

void imx_set_soc_revision(unsigned int rev)
{
	imx_soc_revision = rev;
}

unsigned int imx_get_soc_revision(void)
{
	return imx_soc_revision;
}

void imx_print_silicon_rev(const char *cpu, int srev)
{
	if (srev == IMX_CHIP_REVISION_UNKNOWN)
		pr_info("CPU identified as %s, unknown revision\n", cpu);
	else
		pr_info("CPU identified as %s, silicon rev %d.%d\n",
				cpu, (srev >> 4) & 0xf, srev & 0xf);
}

void __init imx_set_aips(void __iomem *base)
{
	unsigned int reg;
/*
 * Set all MPROTx to be non-bufferable, trusted for R/W,
 * not forced to user-mode.
 */
	__raw_writel(0x77777777, base + 0x0);
	__raw_writel(0x77777777, base + 0x4);

/*
 * Set all OPACRx to be non-bufferable, to not require
 * supervisor privilege level for access, allow for
 * write access and untrusted master access.
 */
	__raw_writel(0x0, base + 0x40);
	__raw_writel(0x0, base + 0x44);
	__raw_writel(0x0, base + 0x48);
	__raw_writel(0x0, base + 0x4C);
	reg = __raw_readl(base + 0x50) & 0x00FFFFFF;
	__raw_writel(reg, base + 0x50);
}

/* IMX家族soc设备初始化
 * 成功则返回soc设备对应的device结构指针
 */
struct device * __init imx_soc_device_init(void)
{
	struct soc_device_attribute *soc_dev_attr;
	struct soc_device *soc_dev;
	struct device_node *root;
	const char *soc_id;
	int ret;

	soc_dev_attr = kzalloc(sizeof(*soc_dev_attr), GFP_KERNEL);
	if (!soc_dev_attr)
		return NULL;

	soc_dev_attr->family = "Freescale i.MX";

    // 获取板卡设备树中的root节点
	root = of_find_node_by_path("/");
    // 从root节点中获取"model"属性值，也就是板卡型号
	ret = of_property_read_string(root, "model", &soc_dev_attr->machine);
	of_node_put(root);
	if (ret)
		goto free_soc;

	switch (__mxc_cpu_type) {
	case MXC_CPU_MX1:
		soc_id = "i.MX1";
		break;
	case MXC_CPU_MX21:
		soc_id = "i.MX21";
		break;
	case MXC_CPU_MX25:
		soc_id = "i.MX25";
		break;
	case MXC_CPU_MX27:
		soc_id = "i.MX27";
		break;
	case MXC_CPU_MX31:
		soc_id = "i.MX31";
		break;
	case MXC_CPU_MX35:
		soc_id = "i.MX35";
		break;
	case MXC_CPU_MX51:
		soc_id = "i.MX51";
		break;
	case MXC_CPU_MX53:
		soc_id = "i.MX53";
		break;
	case MXC_CPU_IMX6SL:
		soc_id = "i.MX6SL";
		break;
	case MXC_CPU_IMX6DL:
		soc_id = "i.MX6DL";
		break;
	case MXC_CPU_IMX6SX:
		soc_id = "i.MX6SX";
		break;
	case MXC_CPU_IMX6UL:
		soc_id = "i.MX6UL";
		break;
	case MXC_CPU_IMX6Q:
		if (imx_get_soc_revision() == IMX_CHIP_REVISION_2_0)
			soc_id = "i.MX6QP";
		else
			soc_id = "i.MX6Q";
		break;
	case MXC_CPU_IMX7D:
		soc_id = "i.MX7D";
		break;
	default:
		soc_id = "Unknown";
	}
	soc_dev_attr->soc_id = soc_id;

	soc_dev_attr->revision = kasprintf(GFP_KERNEL, "%d.%d",
					   (imx_soc_revision >> 4) & 0xf,
					   imx_soc_revision & 0xf);
	if (!soc_dev_attr->revision)
		goto free_soc;

    // 注册该IMX家族的soc设备
	soc_dev = soc_device_register(soc_dev_attr);
	if (IS_ERR(soc_dev))
		goto free_rev;

    // 返回该IMX家族的soc设备对应的通用意义上设备对象
	return soc_device_to_device(soc_dev);

free_rev:
	kfree(soc_dev_attr->revision);
free_soc:
	kfree(soc_dev_attr);
	return NULL;
}
