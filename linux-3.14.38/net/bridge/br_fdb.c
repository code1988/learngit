/*
 *	Forwarding database
 *	Linux ethernet bridge
 *	以太网桥使用的转发数据库
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/rculist.h>
#include <linux/spinlock.h>
#include <linux/times.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/jhash.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/atomic.h>
#include <asm/unaligned.h>
#include <linux/if_vlan.h>
#include "br_private.h"


static struct kmem_cache *br_fdb_cache __read_mostly;   // 定义了一个二层转发表表项的缓存池
static struct net_bridge_fdb_entry *fdb_find(struct hlist_head *head,
					     const unsigned char *addr,
					     __u16 vid);
static int fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		      const unsigned char *addr, u16 vid);
static void fdb_notify(struct net_bridge *br,
		       const struct net_bridge_fdb_entry *, int);

static u32 fdb_salt __read_mostly;      // 这个值随机生成，用于jhash算法中

/* 初始化供网桥使用的二层转发表项的缓存池
 * 实际就是为其分配一块slab缓冲区
 */
int __init br_fdb_init(void)
{
    // 创建一片新的slab缓存池，单元长度net_bridge_fdb_entry
	br_fdb_cache = kmem_cache_create("bridge_fdb_cache",
					 sizeof(struct net_bridge_fdb_entry),
					 0,
					 SLAB_HWCACHE_ALIGN, NULL);
	if (!br_fdb_cache)
		return -ENOMEM;

    // 生成一个随机数，将用于转发表的hash算法计算
	get_random_bytes(&fdb_salt, sizeof(fdb_salt));
	return 0;
}

/* 销毁以太网桥使用的转发数据库
 * 实际就是销毁分配的slab缓冲区对象
 */
void br_fdb_fini(void)
{
	kmem_cache_destroy(br_fdb_cache);
}


/* if topology_changing then use forward_delay (default 15 sec)
 * otherwise keep longer (default 5 minutes)
 * 返回一个持有时间，当拓扑变化时该值来自转发延迟时间，否则该值来自老化时间
 */
static inline unsigned long hold_time(const struct net_bridge *br)
{
	return br->topology_change ? br->forward_delay : br->ageing_time;
}

static inline int has_expired(const struct net_bridge *br,
				  const struct net_bridge_fdb_entry *fdb)
{
	return !fdb->is_static &&
		time_before_eq(fdb->updated + hold_time(br), jiffies);
}

/* 计算 mac+vid 的散列地址并返回
 * 
 * 备注：这里使用了经典的jhash算法，2个键值分别为mac地址后4字节、vid
 */
static inline int br_mac_hash(const unsigned char *mac, __u16 vid)
{
	/* use 1 byte of OUI and 3 bytes of NIC */
	u32 key = get_unaligned((u32 *)(mac + 2));
	return jhash_2words(key, vid, fdb_salt) & (BR_HASH_SIZE - 1);
}

static void fdb_rcu_free(struct rcu_head *head)
{
	struct net_bridge_fdb_entry *ent
		= container_of(head, struct net_bridge_fdb_entry, rcu);
	kmem_cache_free(br_fdb_cache, ent);
}

static void fdb_delete(struct net_bridge *br, struct net_bridge_fdb_entry *f)
{
	hlist_del_rcu(&f->hlist);
	fdb_notify(br, f, RTM_DELNEIGH);
	call_rcu(&f->rcu, fdb_rcu_free);
}

/* Delete a local entry if no other port had the same address. */
static void fdb_delete_local(struct net_bridge *br,
			     const struct net_bridge_port *p,
			     struct net_bridge_fdb_entry *f)
{
	const unsigned char *addr = f->addr.addr;
	u16 vid = f->vlan_id;
	struct net_bridge_port *op;

	/* Maybe another port has same hw addr? */
	list_for_each_entry(op, &br->port_list, list) {
		if (op != p && ether_addr_equal(op->dev->dev_addr, addr) &&
		    (!vid || nbp_vlan_find(op, vid))) {
			f->dst = op;
			f->added_by_user = 0;
			return;
		}
	}

	/* Maybe bridge device has same hw addr? */
	if (p && ether_addr_equal(br->dev->dev_addr, addr) &&
	    (!vid || br_vlan_find(br, vid))) {
		f->dst = NULL;
		f->added_by_user = 0;
		return;
	}

	fdb_delete(br, f);
}

void br_fdb_find_delete_local(struct net_bridge *br,
			      const struct net_bridge_port *p,
			      const unsigned char *addr, u16 vid)
{
	struct hlist_head *head = &br->hash[br_mac_hash(addr, vid)];
	struct net_bridge_fdb_entry *f;

	spin_lock_bh(&br->hash_lock);
	f = fdb_find(head, addr, vid);
	if (f && f->is_local && !f->added_by_user && f->dst == p)
		fdb_delete_local(br, p, f);
	spin_unlock_bh(&br->hash_lock);
}

void br_fdb_changeaddr(struct net_bridge_port *p, const unsigned char *newaddr)
{
	struct net_bridge *br = p->br;
	struct net_port_vlans *pv = nbp_get_vlan_info(p);
	bool no_vlan = !pv;
	int i;
	u16 vid;

	spin_lock_bh(&br->hash_lock);

	/* Search all chains since old address/hash is unknown */
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct hlist_node *h;
		hlist_for_each(h, &br->hash[i]) {
			struct net_bridge_fdb_entry *f;

			f = hlist_entry(h, struct net_bridge_fdb_entry, hlist);
			if (f->dst == p && f->is_local && !f->added_by_user) {
				/* delete old one */
				fdb_delete_local(br, p, f);

				/* if this port has no vlan information
				 * configured, we can safely be done at
				 * this point.
				 */
				if (no_vlan)
					goto insert;
			}
		}
	}

insert:
	/* insert new address,  may fail if invalid address or dup. */
	fdb_insert(br, p, newaddr, 0);

	if (no_vlan)
		goto done;

	/* Now add entries for every VLAN configured on the port.
	 * This function runs under RTNL so the bitmap will not change
	 * from under us.
	 */
	for_each_set_bit(vid, pv->vlan_bitmap, VLAN_N_VID)
		fdb_insert(br, p, newaddr, vid);

done:
	spin_unlock_bh(&br->hash_lock);
}

void br_fdb_change_mac_address(struct net_bridge *br, const u8 *newaddr)
{
	struct net_bridge_fdb_entry *f;
	struct net_port_vlans *pv;
	u16 vid = 0;

	spin_lock_bh(&br->hash_lock);

	/* If old entry was unassociated with any port, then delete it. */
	f = __br_fdb_get(br, br->dev->dev_addr, 0);
	if (f && f->is_local && !f->dst)
		fdb_delete_local(br, NULL, f);

	fdb_insert(br, NULL, newaddr, 0);

	/* Now remove and add entries for every VLAN configured on the
	 * bridge.  This function runs under RTNL so the bitmap will not
	 * change from under us.
	 */
	pv = br_get_vlan_info(br);
	if (!pv)
		goto out;

	for_each_set_bit_from(vid, pv->vlan_bitmap, VLAN_N_VID) {
		f = __br_fdb_get(br, br->dev->dev_addr, vid);
		if (f && f->is_local && !f->dst)
			fdb_delete_local(br, NULL, f);
		fdb_insert(br, NULL, newaddr, vid);
	}
out:
	spin_unlock_bh(&br->hash_lock);
}

void br_fdb_cleanup(unsigned long _data)
{
	struct net_bridge *br = (struct net_bridge *)_data;
	unsigned long delay = hold_time(br);
	unsigned long next_timer = jiffies + br->ageing_time;
	int i;

	spin_lock(&br->hash_lock);
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct net_bridge_fdb_entry *f;
		struct hlist_node *n;

		hlist_for_each_entry_safe(f, n, &br->hash[i], hlist) {
			unsigned long this_timer;
			if (f->is_static)
				continue;
			this_timer = f->updated + delay;
			if (time_before_eq(this_timer, jiffies))
				fdb_delete(br, f);
			else if (time_before(this_timer, next_timer))
				next_timer = this_timer;
		}
	}
	spin_unlock(&br->hash_lock);

	mod_timer(&br->gc_timer, round_jiffies_up(next_timer));
}

/* Completely flush all dynamic entries in forwarding database.*/
void br_fdb_flush(struct net_bridge *br)
{
	int i;

	spin_lock_bh(&br->hash_lock);
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct net_bridge_fdb_entry *f;
		struct hlist_node *n;
		hlist_for_each_entry_safe(f, n, &br->hash[i], hlist) {
			if (!f->is_static)
				fdb_delete(br, f);
		}
	}
	spin_unlock_bh(&br->hash_lock);
}

/* Flush all entries referring to a specific port.
 * if do_all is set also flush static entries
 */
void br_fdb_delete_by_port(struct net_bridge *br,
			   const struct net_bridge_port *p,
			   int do_all)
{
	int i;

	spin_lock_bh(&br->hash_lock);
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct hlist_node *h, *g;

		hlist_for_each_safe(h, g, &br->hash[i]) {
			struct net_bridge_fdb_entry *f
				= hlist_entry(h, struct net_bridge_fdb_entry, hlist);
			if (f->dst != p)
				continue;

			if (f->is_static && !do_all)
				continue;

			if (f->is_local)
				fdb_delete_local(br, p, f);
			else
				fdb_delete(br, f);
		}
	}
	spin_unlock_bh(&br->hash_lock);
}

/* No locking or refcounting, assumes caller has rcu_read_lock 
 * 根据 mac地址+vid 查找对应的转发表表项
 * */
struct net_bridge_fdb_entry *__br_fdb_get(struct net_bridge *br,
					  const unsigned char *addr,
					  __u16 vid)
{
	struct net_bridge_fdb_entry *fdb;

	hlist_for_each_entry_rcu(fdb,
				&br->hash[br_mac_hash(addr, vid)], hlist) {
		if (ether_addr_equal(fdb->addr.addr, addr) &&
		    fdb->vlan_id == vid) {
			if (unlikely(has_expired(br, fdb)))
				break;
			return fdb;
		}
	}

	return NULL;
}

#if IS_ENABLED(CONFIG_ATM_LANE)
/* Interface used by ATM LANE hook to test
 * if an addr is on some other bridge port */
int br_fdb_test_addr(struct net_device *dev, unsigned char *addr)
{
	struct net_bridge_fdb_entry *fdb;
	struct net_bridge_port *port;
	int ret;

	rcu_read_lock();
	port = br_port_get_rcu(dev);
	if (!port)
		ret = 0;
	else {
		fdb = __br_fdb_get(port->br, addr, 0);
		ret = fdb && fdb->dst && fdb->dst->dev != dev &&
			fdb->dst->state == BR_STATE_FORWARDING;
	}
	rcu_read_unlock();

	return ret;
}
#endif /* CONFIG_ATM_LANE */

/*
 * Fill buffer with forwarding table records in
 * the API format.
 */
int br_fdb_fillbuf(struct net_bridge *br, void *buf,
		   unsigned long maxnum, unsigned long skip)
{
	struct __fdb_entry *fe = buf;
	int i, num = 0;
	struct net_bridge_fdb_entry *f;

	memset(buf, 0, maxnum*sizeof(struct __fdb_entry));

	rcu_read_lock();
	for (i = 0; i < BR_HASH_SIZE; i++) {
		hlist_for_each_entry_rcu(f, &br->hash[i], hlist) {
			if (num >= maxnum)
				goto out;

			if (has_expired(br, f))
				continue;

			/* ignore pseudo entry for local MAC address */
			if (!f->dst)
				continue;

			if (skip) {
				--skip;
				continue;
			}

			/* convert from internal format to API */
			memcpy(fe->mac_addr, f->addr.addr, ETH_ALEN);

			/* due to ABI compat need to split into hi/lo */
			fe->port_no = f->dst->port_no;
			fe->port_hi = f->dst->port_no >> 8;

			fe->is_local = f->is_local;
			if (!f->is_static)
				fe->ageing_timer_value = jiffies_delta_to_clock_t(jiffies - f->updated);
			++fe;
			++num;
		}
	}

 out:
	rcu_read_unlock();

	return num;
}

// 根据mac+vid查找对应的转发表项
static struct net_bridge_fdb_entry *fdb_find(struct hlist_head *head,
					     const unsigned char *addr,
					     __u16 vid)
{
	struct net_bridge_fdb_entry *fdb;

	hlist_for_each_entry(fdb, head, hlist) {
		if (ether_addr_equal(fdb->addr.addr, addr) &&
		    fdb->vlan_id == vid)
			return fdb;
	}
	return NULL;
}

/* 根据mac+vid从指定hash桶中查找对应的转发表项
 * @head    - 指向一个hash桶头结构
 * @addr    - 指向要查找的mac
 * @vid     - 要查找的mac所在vlan
 * @返回值  - 成功则返回对应的转发表项，否则返回NULL
 */
static struct net_bridge_fdb_entry *fdb_find_rcu(struct hlist_head *head,
						 const unsigned char *addr,
						 __u16 vid)
{
	struct net_bridge_fdb_entry *fdb;

	hlist_for_each_entry_rcu(fdb, head, hlist) {
		if (ether_addr_equal(fdb->addr.addr, addr) &&
		    fdb->vlan_id == vid)
			return fdb;
	}
	return NULL;
}

/* 创建表项
 * @head    - 指向新创建的表项要插入的hash桶头结构
 * @addr    - 指向要添加的mac
 * @source  - 指向收到该mac的桥端口
 * @vid     - 该mac所属的vlan
 *
 * 备注：创建的转发表项缺省属性有
 *          非本地
 *          动态
 *          非用户手动添加
 */
static struct net_bridge_fdb_entry *fdb_create(struct hlist_head *head,
					       struct net_bridge_port *source,
					       const unsigned char *addr,
					       __u16 vid)
{
	struct net_bridge_fdb_entry *fdb;

	fdb = kmem_cache_alloc(br_fdb_cache, GFP_ATOMIC);
	if (fdb) {
		memcpy(fdb->addr.addr, addr, ETH_ALEN);
		fdb->dst = source;
		fdb->vlan_id = vid;
		fdb->is_local = 0;
		fdb->is_static = 0;
		fdb->added_by_user = 0;
		fdb->updated = fdb->used = jiffies;
		hlist_add_head_rcu(&fdb->hlist, head);
	}
	return fdb;
}

static int fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		  const unsigned char *addr, u16 vid)
{
	struct hlist_head *head = &br->hash[br_mac_hash(addr, vid)];
	struct net_bridge_fdb_entry *fdb;

	if (!is_valid_ether_addr(addr))
		return -EINVAL;

	fdb = fdb_find(head, addr, vid);
	if (fdb) {
		/* it is okay to have multiple ports with same
		 * address, just use the first one.
		 */
		if (fdb->is_local)
			return 0;
		br_warn(br, "adding interface %s with same address "
		       "as a received packet\n",
		       source ? source->dev->name : br->dev->name);
		fdb_delete(br, fdb);
	}

	fdb = fdb_create(head, source, addr, vid);
	if (!fdb)
		return -ENOMEM;

	fdb->is_local = fdb->is_static = 1;
	fdb_notify(br, fdb, RTM_NEWNEIGH);
	return 0;
}

/* Add entry for local address of interface 
 * 将桥端口设备的mac地址作为本地地址加入到转发表中
 * */
int br_fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		  const unsigned char *addr, u16 vid)
{
	int ret;

	spin_lock_bh(&br->hash_lock);
	ret = fdb_insert(br, source, addr, vid);
	spin_unlock_bh(&br->hash_lock);
	return ret;
}

/* 使用指定 mac地址+端口+vlan 3元组更新网桥的转发表(显然，如果转发表中不存在该表项则添加)
 * @br      - 指向要操作的网桥
 * @source  - 指向收到报文的桥端口
 * @addr    - 指向要添加的mac
 * @vid     - 该mac所在的vlan
 * @added_by_user   - 1：表示该3元组是由用户手动添加的； 0：表示该3元组从报文中自动学习得到
 */
void br_fdb_update(struct net_bridge *br, struct net_bridge_port *source,
		   const unsigned char *addr, u16 vid, bool added_by_user)
{
    // 计算得到该mac地址对应的hash桶头结构
	struct hlist_head *head = &br->hash[br_mac_hash(addr, vid)];
	struct net_bridge_fdb_entry *fdb;

	/* some users want to always flood. 
     * 如果该网桥持有时间为0意味着所属的桥端口都不学习任何mac，也就是总是泛洪
     * */
	if (hold_time(br) == 0)
		return;

	/* ignore packets unless we are using this port 
     * 如果该桥端口既不处于learning状态，又不处于forwarding状态，则意味着该桥端口不学习mac
     * */
	if (!(source->state == BR_STATE_LEARNING ||
	      source->state == BR_STATE_FORWARDING))
		return;

    // 根据mac+vid查找对应的转发表项
	fdb = fdb_find_rcu(head, addr, vid);
	if (likely(fdb)) {
        // 如果该表项已经存在，则刷新该表项
		/* attempt to update an entry for a local interface 
         * 如果该mac属于网桥自身mac，意味着收到了自己发出的报文，通常也就意味着链路成环，有风暴的可能
         * */
		if (unlikely(fdb->is_local)) {
			if (net_ratelimit())
				br_warn(br, "received packet on %s with "
					"own address as source address\n",
					source->dev->name);
		} else {
			/* fastpath: update of existing entry 
             * 排除了以上的特殊情况后，这里才是真正刷新该表项的地方
             * */
			fdb->dst = source;
			fdb->updated = jiffies;
			if (unlikely(added_by_user))
				fdb->added_by_user = 1;
		}
	} else {
        // 如果该表项不存在，则添加该表项
		spin_lock(&br->hash_lock);
        // 这里进行才次查找确认的目的是为了防止fdb_find_rcu到fdb_find期间cpu被其他用户抢占并抢先插入了该表项
		if (likely(!fdb_find(head, addr, vid))) {
			fdb = fdb_create(head, source, addr, vid);
			if (fdb) {
				if (unlikely(added_by_user))
					fdb->added_by_user = 1;
                // 添加完该表项后需要发送RTM_NEWNEIGH通知
				fdb_notify(br, fdb, RTM_NEWNEIGH);
			}
		}
		/* else  we lose race and someone else inserts
		 * it first, don't bother updating
		 */
		spin_unlock(&br->hash_lock);
	}
}

static int fdb_to_nud(const struct net_bridge_fdb_entry *fdb)
{
	if (fdb->is_local)
		return NUD_PERMANENT;
	else if (fdb->is_static)
		return NUD_NOARP;
	else if (has_expired(fdb->dst->br, fdb))
		return NUD_STALE;
	else
		return NUD_REACHABLE;
}

static int fdb_fill_info(struct sk_buff *skb, const struct net_bridge *br,
			 const struct net_bridge_fdb_entry *fdb,
			 u32 portid, u32 seq, int type, unsigned int flags)
{
	unsigned long now = jiffies;
	struct nda_cacheinfo ci;
	struct nlmsghdr *nlh;
	struct ndmsg *ndm;

	nlh = nlmsg_put(skb, portid, seq, type, sizeof(*ndm), flags);
	if (nlh == NULL)
		return -EMSGSIZE;

	ndm = nlmsg_data(nlh);
	ndm->ndm_family	 = AF_BRIDGE;
	ndm->ndm_pad1    = 0;
	ndm->ndm_pad2    = 0;
	ndm->ndm_flags	 = 0;
	ndm->ndm_type	 = 0;
	ndm->ndm_ifindex = fdb->dst ? fdb->dst->dev->ifindex : br->dev->ifindex;
	ndm->ndm_state   = fdb_to_nud(fdb);

	if (nla_put(skb, NDA_LLADDR, ETH_ALEN, &fdb->addr))
		goto nla_put_failure;
	ci.ndm_used	 = jiffies_to_clock_t(now - fdb->used);
	ci.ndm_confirmed = 0;
	ci.ndm_updated	 = jiffies_to_clock_t(now - fdb->updated);
	ci.ndm_refcnt	 = 0;
	if (nla_put(skb, NDA_CACHEINFO, sizeof(ci), &ci))
		goto nla_put_failure;

	if (nla_put(skb, NDA_VLAN, sizeof(u16), &fdb->vlan_id))
		goto nla_put_failure;

	return nlmsg_end(skb, nlh);

nla_put_failure:
	nlmsg_cancel(skb, nlh);
	return -EMSGSIZE;
}

// 计算转发表的netlink消息payload长度
static inline size_t fdb_nlmsg_size(void)
{
	return NLMSG_ALIGN(sizeof(struct ndmsg))
		+ nla_total_size(ETH_ALEN) /* NDA_LLADDR */
		+ nla_total_size(sizeof(u16)) /* NDA_VLAN */
		+ nla_total_size(sizeof(struct nda_cacheinfo));
}

/* 转发表的rtnetlink通知接口
 * @br      - 指向所属的网桥
 * @fdb     - 指向发起该通知的转发表表项
 * @type    - rtnetlink通知类型
 */
static void fdb_notify(struct net_bridge *br,
		       const struct net_bridge_fdb_entry *fdb, int type)
{
	struct net *net = dev_net(br->dev);
	struct sk_buff *skb;
	int err = -ENOBUFS;

    // 首先是申请一个skb，用于承载转发表的rtnetlink消息
	skb = nlmsg_new(fdb_nlmsg_size(), GFP_ATOMIC);
	if (skb == NULL)
		goto errout;

	err = fdb_fill_info(skb, br, fdb, 0, 0, type, 0);
	if (err < 0) {
		/* -EMSGSIZE implies BUG in fdb_nlmsg_size() */
		WARN_ON(err == -EMSGSIZE);
		kfree_skb(skb);
		goto errout;
	}
	rtnl_notify(skb, net, 0, RTNLGRP_NEIGH, NULL, GFP_ATOMIC);
	return;
errout:
	rtnl_set_sk_err(net, RTNLGRP_NEIGH, err);
}

/* Dump information about entries, in response to GETNEIGH */
int br_fdb_dump(struct sk_buff *skb,
		struct netlink_callback *cb,
		struct net_device *dev,
		int idx)
{
	struct net_bridge *br = netdev_priv(dev);
	int i;

	if (!(dev->priv_flags & IFF_EBRIDGE))
		goto out;

	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct net_bridge_fdb_entry *f;

		hlist_for_each_entry_rcu(f, &br->hash[i], hlist) {
			if (idx < cb->args[0])
				goto skip;

			if (fdb_fill_info(skb, br, f,
					  NETLINK_CB(cb->skb).portid,
					  cb->nlh->nlmsg_seq,
					  RTM_NEWNEIGH,
					  NLM_F_MULTI) < 0)
				break;
skip:
			++idx;
		}
	}

out:
	return idx;
}

/* Update (create or replace) forwarding database entry */
static int fdb_add_entry(struct net_bridge_port *source, const __u8 *addr,
			 __u16 state, __u16 flags, __u16 vid)
{
	struct net_bridge *br = source->br;
	struct hlist_head *head = &br->hash[br_mac_hash(addr, vid)];
	struct net_bridge_fdb_entry *fdb;
	bool modified = false;

	fdb = fdb_find(head, addr, vid);
	if (fdb == NULL) {
		if (!(flags & NLM_F_CREATE))
			return -ENOENT;

		fdb = fdb_create(head, source, addr, vid);
		if (!fdb)
			return -ENOMEM;

		modified = true;
	} else {
		if (flags & NLM_F_EXCL)
			return -EEXIST;

		if (fdb->dst != source) {
			fdb->dst = source;
			modified = true;
		}
	}

	if (fdb_to_nud(fdb) != state) {
		if (state & NUD_PERMANENT)
			fdb->is_local = fdb->is_static = 1;
		else if (state & NUD_NOARP) {
			fdb->is_local = 0;
			fdb->is_static = 1;
		} else
			fdb->is_local = fdb->is_static = 0;

		modified = true;
	}
	fdb->added_by_user = 1;

	fdb->used = jiffies;
	if (modified) {
		fdb->updated = jiffies;
		fdb_notify(br, fdb, RTM_NEWNEIGH);
	}

	return 0;
}

static int __br_fdb_add(struct ndmsg *ndm, struct net_bridge_port *p,
	       const unsigned char *addr, u16 nlh_flags, u16 vid)
{
	int err = 0;

	if (ndm->ndm_flags & NTF_USE) {
		rcu_read_lock();
		br_fdb_update(p->br, p, addr, vid, true);
		rcu_read_unlock();
	} else {
		spin_lock_bh(&p->br->hash_lock);
		err = fdb_add_entry(p, addr, ndm->ndm_state,
				    nlh_flags, vid);
		spin_unlock_bh(&p->br->hash_lock);
	}

	return err;
}

/* Add new permanent fdb entry with RTM_NEWNEIGH */
int br_fdb_add(struct ndmsg *ndm, struct nlattr *tb[],
	       struct net_device *dev,
	       const unsigned char *addr, u16 nlh_flags)
{
	struct net_bridge_port *p;
	int err = 0;
	struct net_port_vlans *pv;
	unsigned short vid = VLAN_N_VID;

	if (!(ndm->ndm_state & (NUD_PERMANENT|NUD_NOARP|NUD_REACHABLE))) {
		pr_info("bridge: RTM_NEWNEIGH with invalid state %#x\n", ndm->ndm_state);
		return -EINVAL;
	}

	if (tb[NDA_VLAN]) {
		if (nla_len(tb[NDA_VLAN]) != sizeof(unsigned short)) {
			pr_info("bridge: RTM_NEWNEIGH with invalid vlan\n");
			return -EINVAL;
		}

		vid = nla_get_u16(tb[NDA_VLAN]);

		if (!vid || vid >= VLAN_VID_MASK) {
			pr_info("bridge: RTM_NEWNEIGH with invalid vlan id %d\n",
				vid);
			return -EINVAL;
		}
	}

	if (is_zero_ether_addr(addr)) {
		pr_info("bridge: RTM_NEWNEIGH with invalid ether address\n");
		return -EINVAL;
	}

	p = br_port_get_rtnl(dev);
	if (p == NULL) {
		pr_info("bridge: RTM_NEWNEIGH %s not a bridge port\n",
			dev->name);
		return -EINVAL;
	}

	pv = nbp_get_vlan_info(p);
	if (vid != VLAN_N_VID) {
		if (!pv || !test_bit(vid, pv->vlan_bitmap)) {
			pr_info("bridge: RTM_NEWNEIGH with unconfigured "
				"vlan %d on port %s\n", vid, dev->name);
			return -EINVAL;
		}

		/* VID was specified, so use it. */
		err = __br_fdb_add(ndm, p, addr, nlh_flags, vid);
	} else {
		if (!pv || bitmap_empty(pv->vlan_bitmap, VLAN_N_VID)) {
			err = __br_fdb_add(ndm, p, addr, nlh_flags, 0);
			goto out;
		}

		/* We have vlans configured on this port and user didn't
		 * specify a VLAN.  To be nice, add/update entry for every
		 * vlan on this port.
		 */
		for_each_set_bit(vid, pv->vlan_bitmap, VLAN_N_VID) {
			err = __br_fdb_add(ndm, p, addr, nlh_flags, vid);
			if (err)
				goto out;
		}
	}

out:
	return err;
}

static int fdb_delete_by_addr(struct net_bridge *br, const u8 *addr, u16 vlan)
{
	struct hlist_head *head = &br->hash[br_mac_hash(addr, vlan)];
	struct net_bridge_fdb_entry *fdb;

	fdb = fdb_find(head, addr, vlan);
	if (!fdb)
		return -ENOENT;

	fdb_delete(br, fdb);
	return 0;
}

static int __br_fdb_delete(struct net_bridge_port *p,
			   const unsigned char *addr, u16 vid)
{
	int err;

	spin_lock_bh(&p->br->hash_lock);
	err = fdb_delete_by_addr(p->br, addr, vid);
	spin_unlock_bh(&p->br->hash_lock);

	return err;
}

/* Remove neighbor entry with RTM_DELNEIGH */
int br_fdb_delete(struct ndmsg *ndm, struct nlattr *tb[],
		  struct net_device *dev,
		  const unsigned char *addr)
{
	struct net_bridge_port *p;
	int err;
	struct net_port_vlans *pv;
	unsigned short vid = VLAN_N_VID;

	if (tb[NDA_VLAN]) {
		if (nla_len(tb[NDA_VLAN]) != sizeof(unsigned short)) {
			pr_info("bridge: RTM_NEWNEIGH with invalid vlan\n");
			return -EINVAL;
		}

		vid = nla_get_u16(tb[NDA_VLAN]);

		if (!vid || vid >= VLAN_VID_MASK) {
			pr_info("bridge: RTM_NEWNEIGH with invalid vlan id %d\n",
				vid);
			return -EINVAL;
		}
	}
	p = br_port_get_rtnl(dev);
	if (p == NULL) {
		pr_info("bridge: RTM_DELNEIGH %s not a bridge port\n",
			dev->name);
		return -EINVAL;
	}

	pv = nbp_get_vlan_info(p);
	if (vid != VLAN_N_VID) {
		if (!pv || !test_bit(vid, pv->vlan_bitmap)) {
			pr_info("bridge: RTM_DELNEIGH with unconfigured "
				"vlan %d on port %s\n", vid, dev->name);
			return -EINVAL;
		}

		err = __br_fdb_delete(p, addr, vid);
	} else {
		if (!pv || bitmap_empty(pv->vlan_bitmap, VLAN_N_VID)) {
			err = __br_fdb_delete(p, addr, 0);
			goto out;
		}

		/* We have vlans configured on this port and user didn't
		 * specify a VLAN.  To be nice, add/update entry for every
		 * vlan on this port.
		 */
		err = -ENOENT;
		for_each_set_bit(vid, pv->vlan_bitmap, VLAN_N_VID) {
			err &= __br_fdb_delete(p, addr, vid);
		}
	}
out:
	return err;
}
