#ifndef __BEN_VLAN_802_1Q_INC__
#define __BEN_VLAN_802_1Q_INC__

#include <linux/if_vlan.h>
#include <linux/u64_stats_sync.h>
#include <linux/list.h>

/* if this changes, algorithm will have to be reworked because this
 * depends on completely exhausting the VLAN identifier space.  Thus
 * it gives constant time look-up, but in many cases it wastes memory.
 */
#define VLAN_GROUP_ARRAY_SPLIT_PARTS  8
#define VLAN_GROUP_ARRAY_PART_LEN     (VLAN_N_VID/VLAN_GROUP_ARRAY_SPLIT_PARTS)

// vlan协议序号定义(用于定位四维结构的第一维)
enum vlan_protos {
	VLAN_PROTO_8021Q	= 0,
	VLAN_PROTO_8021AD,
	VLAN_PROTO_NUM,
};

/* 定义同一个宿主设备下的vlan组模型
 * 存储的vlan设备呈现四维结构：VLAN_PROTO_NUM * VLAN_GROUP_ARRAY_SPLIT_PARTS * VLAN_GROUP_ARRAY_PART_LEN * struct net_device指针
 */
struct vlan_group {
	unsigned int		nr_vlan_devs;   // 记录了该vlan组中包含的vlan设备数量
	struct hlist_node	hlist;	/* linked list */
	struct net_device **vlan_devices_arrays[VLAN_PROTO_NUM]
					       [VLAN_GROUP_ARRAY_SPLIT_PARTS];  // 记录了该vlan组中包含的所有vlan设备
};

// 定义一个记录vlan设备信息的结构，保存了一个宿主设备上绑定的所有vlan设备的信息
struct vlan_info {
	struct net_device	*real_dev; /* The ethernet(like) device
					    * the vlan is attached to.
                        * 宿主设备
					    */
	struct vlan_group	grp;        // vlan组，记录了所有绑定在该宿主设备上的vlan设备
	struct list_head	vid_list;   // vlan id的链表头
	unsigned int		nr_vids;    // 记录了该vlan id链表中节点数量
	struct rcu_head		rcu;
};

// 根据vlan协议ID获取对应的协议序号(四维结构的第一维)
static inline unsigned int vlan_proto_idx(__be16 proto)
{
	switch (proto) {
	case __constant_htons(ETH_P_8021Q):
		return VLAN_PROTO_8021Q;
	case __constant_htons(ETH_P_8021AD):
		return VLAN_PROTO_8021AD;
	default:
		BUG();
		return 0;
	}
}

static inline struct net_device *__vlan_group_get_device(struct vlan_group *vg,
							 unsigned int pidx,
							 u16 vlan_id)
{
	struct net_device **array;

	array = vg->vlan_devices_arrays[pidx]
				       [vlan_id / VLAN_GROUP_ARRAY_PART_LEN];
	return array ? array[vlan_id % VLAN_GROUP_ARRAY_PART_LEN] : NULL;
}

static inline struct net_device *vlan_group_get_device(struct vlan_group *vg,
						       __be16 vlan_proto,
						       u16 vlan_id)
{
	return __vlan_group_get_device(vg, vlan_proto_idx(vlan_proto), vlan_id);
}

// 将指定的vlan设备记录到四维vlan组模型中
static inline void vlan_group_set_device(struct vlan_group *vg,
					 __be16 vlan_proto, u16 vlan_id,
					 struct net_device *dev)
{
	struct net_device **array;
	if (!vg)
		return;
	array = vg->vlan_devices_arrays[vlan_proto_idx(vlan_proto)]
				       [vlan_id / VLAN_GROUP_ARRAY_PART_LEN];
	array[vlan_id % VLAN_GROUP_ARRAY_PART_LEN] = dev;
}

/* Must be invoked with rcu_read_lock or with RTNL. 
 * 根据vlan id在该宿主设备上索引对应的vlan设备
 * */
static inline struct net_device *vlan_find_dev(struct net_device *real_dev,
					       __be16 vlan_proto, u16 vlan_id)
{
	struct vlan_info *vlan_info = rcu_dereference_rtnl(real_dev->vlan_info);

	if (vlan_info)
		return vlan_group_get_device(&vlan_info->grp,
					     vlan_proto, vlan_id);

	return NULL;
}

#define vlan_group_for_each_dev(grp, i, dev) \
	for ((i) = 0; i < VLAN_PROTO_NUM * VLAN_N_VID; i++) \
		if (((dev) = __vlan_group_get_device((grp), (i) / VLAN_N_VID, \
							    (i) % VLAN_N_VID)))

/* found in vlan_dev.c */
void vlan_dev_set_ingress_priority(const struct net_device *dev,
				   u32 skb_prio, u16 vlan_prio);
int vlan_dev_set_egress_priority(const struct net_device *dev,
				 u32 skb_prio, u16 vlan_prio);
int vlan_dev_change_flags(const struct net_device *dev, u32 flag, u32 mask);
void vlan_dev_get_realdev_name(const struct net_device *dev, char *result);

int vlan_check_real_dev(struct net_device *real_dev,
			__be16 protocol, u16 vlan_id);
void vlan_setup(struct net_device *dev);
int register_vlan_dev(struct net_device *dev);
void unregister_vlan_dev(struct net_device *dev, struct list_head *head);

// 根据传入的vlan-TCI中的帧优先级，返回vlan设备上对应的入口优先级
static inline u32 vlan_get_ingress_priority(struct net_device *dev,
					    u16 vlan_tci)
{
	struct vlan_dev_priv *vip = vlan_dev_priv(dev);

	return vip->ingress_priority_map[(vlan_tci >> VLAN_PRIO_SHIFT) & 0x7];
}

#ifdef CONFIG_VLAN_8021Q_GVRP
int vlan_gvrp_request_join(const struct net_device *dev);
void vlan_gvrp_request_leave(const struct net_device *dev);
int vlan_gvrp_init_applicant(struct net_device *dev);
void vlan_gvrp_uninit_applicant(struct net_device *dev);
int vlan_gvrp_init(void);
void vlan_gvrp_uninit(void);
#else
static inline int vlan_gvrp_request_join(const struct net_device *dev) { return 0; }
static inline void vlan_gvrp_request_leave(const struct net_device *dev) {}
static inline int vlan_gvrp_init_applicant(struct net_device *dev) { return 0; }
static inline void vlan_gvrp_uninit_applicant(struct net_device *dev) {}
static inline int vlan_gvrp_init(void) { return 0; }
static inline void vlan_gvrp_uninit(void) {}
#endif

#ifdef CONFIG_VLAN_8021Q_MVRP
int vlan_mvrp_request_join(const struct net_device *dev);
void vlan_mvrp_request_leave(const struct net_device *dev);
int vlan_mvrp_init_applicant(struct net_device *dev);
void vlan_mvrp_uninit_applicant(struct net_device *dev);
int vlan_mvrp_init(void);
void vlan_mvrp_uninit(void);
#else
static inline int vlan_mvrp_request_join(const struct net_device *dev) { return 0; }
static inline void vlan_mvrp_request_leave(const struct net_device *dev) {}
static inline int vlan_mvrp_init_applicant(struct net_device *dev) { return 0; }
static inline void vlan_mvrp_uninit_applicant(struct net_device *dev) {}
static inline int vlan_mvrp_init(void) { return 0; }
static inline void vlan_mvrp_uninit(void) {}
#endif

extern const char vlan_fullname[];
extern const char vlan_version[];
int vlan_netlink_init(void);
void vlan_netlink_fini(void);

extern struct rtnl_link_ops vlan_link_ops;

extern int vlan_net_id;

struct proc_dir_entry;

/* 定义了整个VLAN模块公用的私有空间(当然公用的前提是同一个网络命名空间下)
 * 这里具体就是记录了有关vlan的proc文件系统信息
 */
struct vlan_net {
	/* /proc/net/vlan */
	struct proc_dir_entry *proc_vlan_dir;
	/* /proc/net/vlan/config */
	struct proc_dir_entry *proc_vlan_conf;
	/* Determines interface naming scheme. */
	unsigned short name_type;   // vlan设备名字显示风格，通常选择eth0.10的风格
};

#endif /* !(__BEN_VLAN_802_1Q_INC__) */
