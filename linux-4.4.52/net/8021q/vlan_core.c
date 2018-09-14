#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/if_vlan.h>
#include <linux/netpoll.h>
#include <linux/export.h>
#include "vlan.h"

// 接收处理携带了vlan信息的skb，主要就是根据vlan id，将skb重定向到对应的vlan设备
bool vlan_do_receive(struct sk_buff **skbp)
{
	struct sk_buff *skb = *skbp;
	__be16 vlan_proto = skb->vlan_proto;    // 提取vlan协议类型
	u16 vlan_id = skb_vlan_tag_get_id(skb); // 提取vlan id
	struct net_device *vlan_dev;
	struct vlan_pcpu_stats *rx_stats;

    // 根据vlan id在该宿主设备上索引对应的vlan设备
	vlan_dev = vlan_find_dev(skb->dev, vlan_proto, vlan_id);
	if (!vlan_dev)
		return false;

    // 检查该skb的数据包缓冲区是否被其他skb共用，如果是则clone一份出来
	skb = *skbp = skb_share_check(skb, GFP_ATOMIC);
	if (unlikely(!skb))
		return false;

    // 重定向该skb->dev为索引到的vlan设备
	skb->dev = vlan_dev;
    // 如果收到的数据包属于PACKET_OTHERHOST类型，这里需要做再次确认
	if (unlikely(skb->pkt_type == PACKET_OTHERHOST)) {
		/* Our lower layer thinks this is not local, let's make sure.
		 * This allows the VLAN to have a different MAC than the
		 * underlying device, and still route correctly. 
         * kernel在这里似乎还有另外一个目的:
         *          就是允许vlan帧携带和宿主设备不同、但和vlan设备相同的mac地址
         *          从中也就意味着vlan设备的mac地址可以和宿主设备不同
         * */
		if (ether_addr_equal_64bits(eth_hdr(skb)->h_dest, vlan_dev->dev_addr))
			skb->pkt_type = PACKET_HOST;
	}

    // 如果该vlan设备的vlan_dev_priv->flags中没有置位VLAN_FLAG_REORDER_HDR，则要在这里找回之前脱掉的vlan tag(默认vlan设备都设置了该标志)
	if (!(vlan_dev_priv(vlan_dev)->flags & VLAN_FLAG_REORDER_HDR) &&
	    !netif_is_macvlan_port(vlan_dev) &&
	    !netif_is_bridge_port(vlan_dev)) {
		unsigned int offset = skb->data - skb_mac_header(skb);

		/*
		 * vlan_insert_tag expect skb->data pointing to mac header.
		 * So change skb->data before calling it and change back to
		 * original position later
		 */
		skb_push(skb, offset);  // skb->data指针前移offset字节，指向mac字段  
		skb = *skbp = vlan_insert_tag(skb, skb->vlan_proto,
					      skb->vlan_tci);   // 插回之前脱掉的4字节vlan tag
		if (!skb)
			return false;
		skb_pull(skb, offset + VLAN_HLEN);  // skb->data指针后移offset+VLAN_HLEN字节，从而恢复到一开始进入本函数时指向的字段
		skb_reset_mac_len(skb);
	}

    // 根据vlan-TCI中的帧优先级，获取该vlan设备上对应的入口优先级
	skb->priority = vlan_get_ingress_priority(vlan_dev, skb->vlan_tci);
    // skb->vlan_tci的使命已经完成，所以这里清除
	skb->vlan_tci = 0;

	rx_stats = this_cpu_ptr(vlan_dev_priv(vlan_dev)->vlan_pcpu_stats);

	u64_stats_update_begin(&rx_stats->syncp);
	rx_stats->rx_packets++;
	rx_stats->rx_bytes += skb->len;
	if (skb->pkt_type == PACKET_MULTICAST)
		rx_stats->rx_multicast++;
	u64_stats_update_end(&rx_stats->syncp);

	return true;
}

/* Must be invoked with rcu_read_lock. */
struct net_device *__vlan_find_dev_deep_rcu(struct net_device *dev,
					__be16 vlan_proto, u16 vlan_id)
{
	struct vlan_info *vlan_info = rcu_dereference(dev->vlan_info);

	if (vlan_info) {
		return vlan_group_get_device(&vlan_info->grp,
					     vlan_proto, vlan_id);
	} else {
		/*
		 * Lower devices of master uppers (bonding, team) do not have
		 * grp assigned to themselves. Grp is assigned to upper device
		 * instead.
		 */
		struct net_device *upper_dev;

		upper_dev = netdev_master_upper_dev_get_rcu(dev);
		if (upper_dev)
			return __vlan_find_dev_deep_rcu(upper_dev,
						    vlan_proto, vlan_id);
	}

	return NULL;
}
EXPORT_SYMBOL(__vlan_find_dev_deep_rcu);

struct net_device *vlan_dev_real_dev(const struct net_device *dev)
{
	struct net_device *ret = vlan_dev_priv(dev)->real_dev;

	while (is_vlan_dev(ret))
		ret = vlan_dev_priv(ret)->real_dev;

	return ret;
}
EXPORT_SYMBOL(vlan_dev_real_dev);

// 获取该vlan设备的vid
u16 vlan_dev_vlan_id(const struct net_device *dev)
{
    // 从该vlan设备附属的私有空间获取vid
	return vlan_dev_priv(dev)->vlan_id;
}
EXPORT_SYMBOL(vlan_dev_vlan_id);

__be16 vlan_dev_vlan_proto(const struct net_device *dev)
{
	return vlan_dev_priv(dev)->vlan_proto;
}
EXPORT_SYMBOL(vlan_dev_vlan_proto);

/*
 * vlan info and vid list
 */

static void vlan_group_free(struct vlan_group *grp)
{
	int i, j;

	for (i = 0; i < VLAN_PROTO_NUM; i++)
		for (j = 0; j < VLAN_GROUP_ARRAY_SPLIT_PARTS; j++)
			kfree(grp->vlan_devices_arrays[i][j]);
}

static void vlan_info_free(struct vlan_info *vlan_info)
{
	vlan_group_free(&vlan_info->grp);
	kfree(vlan_info);
}

static void vlan_info_rcu_free(struct rcu_head *rcu)
{
	vlan_info_free(container_of(rcu, struct vlan_info, rcu));
}

// 为宿主设备dev创建一个vlan_info管理块
static struct vlan_info *vlan_info_alloc(struct net_device *dev)
{
	struct vlan_info *vlan_info;

	vlan_info = kzalloc(sizeof(struct vlan_info), GFP_KERNEL);
	if (!vlan_info)
		return NULL;

	vlan_info->real_dev = dev;
	INIT_LIST_HEAD(&vlan_info->vid_list);
	return vlan_info;
}

// 定义一个记录vlan id信息的节点结构
struct vlan_vid_info {
	struct list_head list;
	__be16 proto;   // 使用的vlan类型ID
	u16 vid;        // vlan id
	int refcount;   // 对该节点的引用计数
};

// 检查该vlan设备是否具有过滤vlan TAG的能力
static bool vlan_hw_filter_capable(const struct net_device *dev,
				     const struct vlan_vid_info *vid_info)
{
	if (vid_info->proto == htons(ETH_P_8021Q) &&
	    dev->features & NETIF_F_HW_VLAN_CTAG_FILTER)
		return true;
	if (vid_info->proto == htons(ETH_P_8021AD) &&
	    dev->features & NETIF_F_HW_VLAN_STAG_FILTER)
		return true;
	return false;
}

// 遍历记录了所有vlan id的链表，根据指定vlan id和vlan类型ID索引对应的节点
static struct vlan_vid_info *vlan_vid_info_get(struct vlan_info *vlan_info,
					       __be16 proto, u16 vid)
{
	struct vlan_vid_info *vid_info;

	list_for_each_entry(vid_info, &vlan_info->vid_list, list) {
		if (vid_info->proto == proto && vid_info->vid == vid)
			return vid_info;
	}
	return NULL;
}

// 创建一个vlan_vid_info节点
static struct vlan_vid_info *vlan_vid_info_alloc(__be16 proto, u16 vid)
{
	struct vlan_vid_info *vid_info;

	vid_info = kzalloc(sizeof(struct vlan_vid_info), GFP_KERNEL);
	if (!vid_info)
		return NULL;
	vid_info->proto = proto;
	vid_info->vid = vid;

	return vid_info;
}

// 创建一个vlan_vid_info节点并插入到管理vlan id信息的链表中
static int __vlan_vid_add(struct vlan_info *vlan_info, __be16 proto, u16 vid,
			  struct vlan_vid_info **pvid_info)
{
	struct net_device *dev = vlan_info->real_dev;
	const struct net_device_ops *ops = dev->netdev_ops;
	struct vlan_vid_info *vid_info;
	int err;

	vid_info = vlan_vid_info_alloc(proto, vid);
	if (!vid_info)
		return -ENOMEM;

    // 检查该vlan设备是否具有过滤vlan TAG的能力
	if (vlan_hw_filter_capable(dev, vid_info)) {
		if (netif_device_present(dev))
			err = ops->ndo_vlan_rx_add_vid(dev, proto, vid);
		else
			err = -ENODEV;
		if (err) {
			kfree(vid_info);
			return err;
		}
	}
	list_add(&vid_info->list, &vlan_info->vid_list);
	vlan_info->nr_vids++;
	*pvid_info = vid_info;
	return 0;
}

// 将指定vlan协议ID的vlan id添加到宿主设备的vlan_info管理块中
int vlan_vid_add(struct net_device *dev, __be16 proto, u16 vid)
{
	struct vlan_info *vlan_info;
	struct vlan_vid_info *vid_info;
	bool vlan_info_created = false;
	int err;

	ASSERT_RTNL();

    // 获取宿主设备的vlan_info管理块
	vlan_info = rtnl_dereference(dev->vlan_info);
    // 如果宿主设备dev还不存在vlan_info，则创建一个
	if (!vlan_info) {
		vlan_info = vlan_info_alloc(dev);
		if (!vlan_info)
			return -ENOMEM;
		vlan_info_created = true;
	}
    // 根据vlan协议ID和vlan id查找对应的vlan_vid_info节点
	vid_info = vlan_vid_info_get(vlan_info, proto, vid);
    // 如果管理vlan id信息的链表中还不存在匹配的节点，则创建一个
	if (!vid_info) {
		err = __vlan_vid_add(vlan_info, proto, vid, &vid_info);
		if (err)
			goto out_free_vlan_info;
	}
    // 将匹配的vlan id节点引用计数加1
	vid_info->refcount++;

    // 如果vlan_info管理块是新创建的，则需要跟所属的宿主设备进行关联
	if (vlan_info_created)
		rcu_assign_pointer(dev->vlan_info, vlan_info);

	return 0;

out_free_vlan_info:
	if (vlan_info_created)
		kfree(vlan_info);
	return err;
}
EXPORT_SYMBOL(vlan_vid_add);

static void __vlan_vid_del(struct vlan_info *vlan_info,
			   struct vlan_vid_info *vid_info)
{
	struct net_device *dev = vlan_info->real_dev;
	const struct net_device_ops *ops = dev->netdev_ops;
	__be16 proto = vid_info->proto;
	u16 vid = vid_info->vid;
	int err;

	if (vlan_hw_filter_capable(dev, vid_info)) {
		if (netif_device_present(dev))
			err = ops->ndo_vlan_rx_kill_vid(dev, proto, vid);
		else
			err = -ENODEV;
		if (err) {
			pr_warn("failed to kill vid %04x/%d for device %s\n",
				proto, vid, dev->name);
		}
	}
	list_del(&vid_info->list);
	kfree(vid_info);
	vlan_info->nr_vids--;
}

void vlan_vid_del(struct net_device *dev, __be16 proto, u16 vid)
{
	struct vlan_info *vlan_info;
	struct vlan_vid_info *vid_info;

	ASSERT_RTNL();

	vlan_info = rtnl_dereference(dev->vlan_info);
	if (!vlan_info)
		return;

	vid_info = vlan_vid_info_get(vlan_info, proto, vid);
	if (!vid_info)
		return;
	vid_info->refcount--;
	if (vid_info->refcount == 0) {
		__vlan_vid_del(vlan_info, vid_info);
		if (vlan_info->nr_vids == 0) {
			RCU_INIT_POINTER(dev->vlan_info, NULL);
			call_rcu(&vlan_info->rcu, vlan_info_rcu_free);
		}
	}
}
EXPORT_SYMBOL(vlan_vid_del);

int vlan_vids_add_by_dev(struct net_device *dev,
			 const struct net_device *by_dev)
{
	struct vlan_vid_info *vid_info;
	struct vlan_info *vlan_info;
	int err;

	ASSERT_RTNL();

	vlan_info = rtnl_dereference(by_dev->vlan_info);
	if (!vlan_info)
		return 0;

	list_for_each_entry(vid_info, &vlan_info->vid_list, list) {
		err = vlan_vid_add(dev, vid_info->proto, vid_info->vid);
		if (err)
			goto unwind;
	}
	return 0;

unwind:
	list_for_each_entry_continue_reverse(vid_info,
					     &vlan_info->vid_list,
					     list) {
		vlan_vid_del(dev, vid_info->proto, vid_info->vid);
	}

	return err;
}
EXPORT_SYMBOL(vlan_vids_add_by_dev);

void vlan_vids_del_by_dev(struct net_device *dev,
			  const struct net_device *by_dev)
{
	struct vlan_vid_info *vid_info;
	struct vlan_info *vlan_info;

	ASSERT_RTNL();

	vlan_info = rtnl_dereference(by_dev->vlan_info);
	if (!vlan_info)
		return;

	list_for_each_entry(vid_info, &vlan_info->vid_list, list)
		vlan_vid_del(dev, vid_info->proto, vid_info->vid);
}
EXPORT_SYMBOL(vlan_vids_del_by_dev);

bool vlan_uses_dev(const struct net_device *dev)
{
	struct vlan_info *vlan_info;

	ASSERT_RTNL();

	vlan_info = rtnl_dereference(dev->vlan_info);
	if (!vlan_info)
		return false;
	return vlan_info->grp.nr_vlan_devs ? true : false;
}
EXPORT_SYMBOL(vlan_uses_dev);
