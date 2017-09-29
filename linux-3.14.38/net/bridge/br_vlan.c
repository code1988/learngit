#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <linux/slab.h>

#include "br_private.h"

static void __vlan_add_pvid(struct net_port_vlans *v, u16 vid)
{
	if (v->pvid == vid)
		return;

	smp_wmb();
	v->pvid = vid;
}

static void __vlan_delete_pvid(struct net_port_vlans *v, u16 vid)
{
	if (v->pvid != vid)
		return;

	smp_wmb();
	v->pvid = 0;
}

static void __vlan_add_flags(struct net_port_vlans *v, u16 vid, u16 flags)
{
	if (flags & BRIDGE_VLAN_INFO_PVID)
		__vlan_add_pvid(v, vid);

	if (flags & BRIDGE_VLAN_INFO_UNTAGGED)
		set_bit(vid, v->untagged_bitmap);
}

static int __vlan_add(struct net_port_vlans *v, u16 vid, u16 flags)
{
	struct net_bridge_port *p = NULL;
	struct net_bridge *br;
	struct net_device *dev;
	int err;

	if (test_bit(vid, v->vlan_bitmap)) {
		__vlan_add_flags(v, vid, flags);
		return 0;
	}

	if (v->port_idx) {
		p = v->parent.port;
		br = p->br;
		dev = p->dev;
	} else {
		br = v->parent.br;
		dev = br->dev;
	}

	if (p) {
		/* Add VLAN to the device filter if it is supported.
		 * Stricly speaking, this is not necessary now, since
		 * devices are made promiscuous by the bridge, but if
		 * that ever changes this code will allow tagged
		 * traffic to enter the bridge.
		 */
		err = vlan_vid_add(dev, htons(ETH_P_8021Q), vid);
		if (err)
			return err;
	}

	err = br_fdb_insert(br, p, dev->dev_addr, vid);
	if (err) {
		br_err(br, "failed insert local address into bridge "
		       "forwarding table\n");
		goto out_filt;
	}

	set_bit(vid, v->vlan_bitmap);
	v->num_vlans++;
	__vlan_add_flags(v, vid, flags);

	return 0;

out_filt:
	if (p)
		vlan_vid_del(dev, htons(ETH_P_8021Q), vid);
	return err;
}

static int __vlan_del(struct net_port_vlans *v, u16 vid)
{
	if (!test_bit(vid, v->vlan_bitmap))
		return -EINVAL;

	__vlan_delete_pvid(v, vid);
	clear_bit(vid, v->untagged_bitmap);

	if (v->port_idx)
		vlan_vid_del(v->parent.port->dev, htons(ETH_P_8021Q), vid);

	clear_bit(vid, v->vlan_bitmap);
	v->num_vlans--;
	if (bitmap_empty(v->vlan_bitmap, VLAN_N_VID)) {
		if (v->port_idx)
			rcu_assign_pointer(v->parent.port->vlan_info, NULL);
		else
			rcu_assign_pointer(v->parent.br->vlan_info, NULL);
		kfree_rcu(v, rcu);
	}
	return 0;
}

static void __vlan_flush(struct net_port_vlans *v)
{
	smp_wmb();
	v->pvid = 0;
	bitmap_zero(v->vlan_bitmap, VLAN_N_VID);
	if (v->port_idx)
		rcu_assign_pointer(v->parent.port->vlan_info, NULL);
	else
		rcu_assign_pointer(v->parent.br->vlan_info, NULL);
	kfree_rcu(v, rcu);
}

/* 网桥对报文进行检测，功能基本类似与br_allowed_egress
 * @br  - 指向进行检测的网桥
 * @v   - 指向一个允许通过的vlan集合(通常就是对应的桥端口配置的vlan集合)
 * @skb - 指向一个需要被检测的skb
 * 
 * 备注：本函数运行前提是配置了CONFIG_BRIDGE_VLAN_FILTERING
 */
struct sk_buff *br_handle_vlan(struct net_bridge *br,
			       const struct net_port_vlans *pv,
			       struct sk_buff *skb)
{
	u16 vid;

	/* If this packet was not filtered at input, let it pass 
     * 如果该skb在入口处已经经过了vlan过滤，这里直接放行
     * */
	if (!BR_INPUT_SKB_CB(skb)->vlan_filtered)
		goto out;

	/* Vlan filter table must be configured at this point.  The
	 * only exception is the bridge is set in promisc mode and the
	 * packet is destined for the bridge device.  In this case
	 * pass the packet as is.
	 */
	if (!pv) {
		if ((br->dev->flags & IFF_PROMISC) && skb->dev == br->dev) {
			goto out;
		} else {
			kfree_skb(skb);
			return NULL;
		}
	}

	/* At this point, we know that the frame was filtered and contains
	 * a valid vlan id.  If the vlan id is set in the untagged bitmap,
	 * send untagged; otherwise, send tagged.
	 */
	br_vlan_get_tag(skb, &vid);
	if (test_bit(vid, pv->untagged_bitmap))
		skb->vlan_tci = 0;

out:
	return skb;
}

/* Called under RCU 
 * 网桥对入口报文进行检测，实质就是检查指定skb所属的vlan是否包含在指定的vlan集合中
 * @br  - 指向进行检测的网桥
 * @v   - 指向一个允许通过的vlan集合(通常就是对应的桥端口配置的vlan集合)
 * @skb - 指向一个需要被检测的skb
 * @vid - 用于存放该skb关联的vlan
 *
 * 备注：本函数运行前提是配置了CONFIG_BRIDGE_VLAN_FILTERING
 *       本函数会将进入网桥的普通包也标上vlan，意味着桥上的所有数据包都是带vlan了的
 *       对于检测不通过的skb，本函数会直接释放
 *       网桥被作为一个特殊的桥端口，所以v可能来自网桥
 * */
bool br_allowed_ingress(struct net_bridge *br, struct net_port_vlans *v,
			struct sk_buff *skb, u16 *vid)
{
	int err;

	/* If VLAN filtering is disabled on the bridge, all packets are
	 * permitted.
     * 如果该网桥没有开启vlan过滤功能，则直接通过
	 */
	if (!br->vlan_enabled) {
		BR_INPUT_SKB_CB(skb)->vlan_filtered = false;
		return true;
	}

	/* If there are no vlan in the permitted list, all packets are
	 * rejected.
     *
     * 在使能了vlan过滤功能的前提下，如果传入的允许通过vlan集合为空，意味着拒绝接收任何报文
	 */
	if (!v)
		goto drop;

    // 程序运行到这里意味着该skb经过了网桥的vlan过滤(经过不代表成功通过)
	BR_INPUT_SKB_CB(skb)->vlan_filtered = true;

	/* If vlan tx offload is disabled on bridge device and frame was
	 * sent from vlan device on the bridge device, it does not have
	 * HW accelerated vlan tag.
     *
     * 如果该skb没有携带vlan信息，但是其数据包的协议类型却是802.1q或802.1ad，则去掉报文中的4字节vlan-tag
	 */
	if (unlikely(!vlan_tx_tag_present(skb) &&
		     (skb->protocol == htons(ETH_P_8021Q) ||
		      skb->protocol == htons(ETH_P_8021AD)))) {
		skb = skb_vlan_untag(skb);
		if (unlikely(!skb))
			return false;
	}

    // 从skb中获取vlan id
	err = br_vlan_get_tag(skb, vid);
    // 如果该skb中没有携带vlan id，则尝试获取该桥端口的pvid
	if (!*vid) {
		u16 pvid = br_get_pvid(v);

		/* Frame had a tag with VID 0 or did not have a tag.
		 * See if pvid is set on this port.  That tells us which
		 * vlan untagged or priority-tagged traffic belongs to.
         * 如果pvid不存在则丢弃直接丢弃
		 */
		if (pvid == VLAN_N_VID)
			goto drop;

		/* PVID is set on this port.  Any untagged or priority-tagged
		 * ingress frame is considered to belong to this vlan.
         * 如果pvid存在，则所有收到的不带tag或者只带了802.1p优先级tag的报文都属于该vlan
		 */
		*vid = pvid;
		if (likely(err))
			/* Untagged Frame. 
             * 表示收到的是不带tag的普通报文
             * 这里用pvid来设置该skb的vlan信息，并且将数据包的协议类型改为802.1q
             * */
			__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), pvid);
		else
			/* Priority-tagged Frame.
			 * At this point, We know that skb->vlan_tci had
			 * VLAN_TAG_PRESENT bit and its VID field was 0x000.
			 * We update only VID field and preserve PCP field.
             * 表示收到的是只带802.1p优先级tag的报文
			 */
			skb->vlan_tci |= pvid;

        // 显然，携带pvid的skb必定是有效的
		return true;
	}

	/* Frame had a valid vlan tag.  See if vlan is allowed 
     * 除了pvid，其他vlan都需要检查是否包含在该桥端口配置的vlan集合中
     * */
	if (test_bit(*vid, v->vlan_bitmap))
		return true;
drop:
	kfree_skb(skb);
	return false;
}

/* Called under RCU. 
 * 网桥对出口报文进行检测，实质就是检查指定skb所属的vlan是否包含在指定的vlan集合中
 * @br  - 指向进行检测的网桥
 * @v   - 指向一个允许通过的vlan集合(桥端口/网桥配置的vlan集合)
 * @skb - 指向一个需要被检测的skb
 * @vid - 用于存放该skb关联的vlan
 *
 * 备注：本函数运行前提是配置了CONFIG_BRIDGE_VLAN_FILTERING
 *       网桥被作为一个特殊的桥端口，所以v可能来自网桥
 * */
bool br_allowed_egress(struct net_bridge *br,
		       const struct net_port_vlans *v,
		       const struct sk_buff *skb)
{
	u16 vid;

	/* If this packet was not filtered at input, let it pass 
     * 如果该skb在入口处已经经过了vlan过滤，这里直接放行
     * */
	if (!BR_INPUT_SKB_CB(skb)->vlan_filtered)
		return true;

    // 在使能了vlan过滤功能的前提下，如果传入的允许通过vlan集合为空，意味着拒绝任何报文
	if (!v)
		return false;

    // 从skb中获取vlan id，看是否包含在允许通过的vlan集合中
	br_vlan_get_tag(skb, &vid);
	if (test_bit(vid, v->vlan_bitmap))
		return true;

	return false;
}

/* Called under RCU 
 * 检查指定桥端口是否允许学习指定skb，实质就是检查指定skb所属的vlan是否包含在该桥端口配置的vlan集合中
 *
 * 备注：本函数运行前提是配置了CONFIG_BRIDGE_VLAN_FILTERING
 * */
bool br_should_learn(struct net_bridge_port *p, struct sk_buff *skb, u16 *vid)
{
	struct net_bridge *br = p->br;
	struct net_port_vlans *v;

	/* If filtering was disabled at input, let it pass. 
     * 如果该网桥没有开启vlan过滤功能，则直接通过
     * */
	if (!br->vlan_enabled)
		return true;

    // 获取该桥端口的vlan信息
	v = rcu_dereference(p->vlan_info);
	if (!v)
		return false;

    // 从skb中获取vlan id
	br_vlan_get_tag(skb, vid);
    // 如果该skb中没有携带vlan id，则尝试获取该桥端口的pvid
	if (!*vid) {
		*vid = br_get_pvid(v);
		if (*vid == VLAN_N_VID)
			return false;

		return true;
	}

    // 检查获取到的vlan id是否包含在该桥端口配置的vlan集合中
	if (test_bit(*vid, v->vlan_bitmap))
		return true;

	return false;
}

/* Must be protected by RTNL.
 * Must be called with vid in range from 1 to 4094 inclusive.
 */
int br_vlan_add(struct net_bridge *br, u16 vid, u16 flags)
{
	struct net_port_vlans *pv = NULL;
	int err;

	ASSERT_RTNL();

	pv = rtnl_dereference(br->vlan_info);
	if (pv)
		return __vlan_add(pv, vid, flags);

	/* Create port vlan infomration
	 */
	pv = kzalloc(sizeof(*pv), GFP_KERNEL);
	if (!pv)
		return -ENOMEM;

	pv->parent.br = br;
	err = __vlan_add(pv, vid, flags);
	if (err)
		goto out;

	rcu_assign_pointer(br->vlan_info, pv);
	return 0;
out:
	kfree(pv);
	return err;
}

/* Must be protected by RTNL.
 * Must be called with vid in range from 1 to 4094 inclusive.
 */
int br_vlan_delete(struct net_bridge *br, u16 vid)
{
	struct net_port_vlans *pv;

	ASSERT_RTNL();

	pv = rtnl_dereference(br->vlan_info);
	if (!pv)
		return -EINVAL;

	br_fdb_find_delete_local(br, NULL, br->dev->dev_addr, vid);

	__vlan_del(pv, vid);
	return 0;
}

void br_vlan_flush(struct net_bridge *br)
{
	struct net_port_vlans *pv;

	ASSERT_RTNL();
	pv = rtnl_dereference(br->vlan_info);
	if (!pv)
		return;

	__vlan_flush(pv);
}

bool br_vlan_find(struct net_bridge *br, u16 vid)
{
	struct net_port_vlans *pv;
	bool found = false;

	rcu_read_lock();
	pv = rcu_dereference(br->vlan_info);

	if (!pv)
		goto out;

	if (test_bit(vid, pv->vlan_bitmap))
		found = true;

out:
	rcu_read_unlock();
	return found;
}

int br_vlan_filter_toggle(struct net_bridge *br, unsigned long val)
{
	if (!rtnl_trylock())
		return restart_syscall();

	if (br->vlan_enabled == val)
		goto unlock;

	br->vlan_enabled = val;

unlock:
	rtnl_unlock();
	return 0;
}

/* Must be protected by RTNL.
 * Must be called with vid in range from 1 to 4094 inclusive.
 */
int nbp_vlan_add(struct net_bridge_port *port, u16 vid, u16 flags)
{
	struct net_port_vlans *pv = NULL;
	int err;

	ASSERT_RTNL();

	pv = rtnl_dereference(port->vlan_info);
	if (pv)
		return __vlan_add(pv, vid, flags);

	/* Create port vlan infomration
	 */
	pv = kzalloc(sizeof(*pv), GFP_KERNEL);
	if (!pv) {
		err = -ENOMEM;
		goto clean_up;
	}

	pv->port_idx = port->port_no;
	pv->parent.port = port;
	err = __vlan_add(pv, vid, flags);
	if (err)
		goto clean_up;

	rcu_assign_pointer(port->vlan_info, pv);
	return 0;

clean_up:
	kfree(pv);
	return err;
}

/* Must be protected by RTNL.
 * Must be called with vid in range from 1 to 4094 inclusive.
 */
int nbp_vlan_delete(struct net_bridge_port *port, u16 vid)
{
	struct net_port_vlans *pv;

	ASSERT_RTNL();

	pv = rtnl_dereference(port->vlan_info);
	if (!pv)
		return -EINVAL;

	br_fdb_find_delete_local(port->br, port, port->dev->dev_addr, vid);

	return __vlan_del(pv, vid);
}

void nbp_vlan_flush(struct net_bridge_port *port)
{
	struct net_port_vlans *pv;
	u16 vid;

	ASSERT_RTNL();

	pv = rtnl_dereference(port->vlan_info);
	if (!pv)
		return;

	for_each_set_bit(vid, pv->vlan_bitmap, VLAN_N_VID)
		vlan_vid_del(port->dev, htons(ETH_P_8021Q), vid);

	__vlan_flush(pv);
}

bool nbp_vlan_find(struct net_bridge_port *port, u16 vid)
{
	struct net_port_vlans *pv;
	bool found = false;

	rcu_read_lock();
	pv = rcu_dereference(port->vlan_info);

	if (!pv)
		goto out;

	if (test_bit(vid, pv->vlan_bitmap))
		found = true;

out:
	rcu_read_unlock();
	return found;
}
