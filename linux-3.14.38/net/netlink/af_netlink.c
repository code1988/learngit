/*
 * NETLINK      Kernel-user communication protocol.
 * netlink协议核心文件
 *
 * 		Authors:	Alan Cox <alan@lxorguk.ukuu.org.uk>
 * 				Alexey Kuznetsov <kuznet@ms2.inr.ac.ru>
 * 				Patrick McHardy <kaber@trash.net>
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Tue Jun 26 14:36:48 MEST 2001 Herbert "herp" Rosmanith
 *                               added netlink_proto_exit
 * Tue Jan 22 18:32:44 BRST 2002 Arnaldo C. de Melo <acme@conectiva.com.br>
 * 				 use nlk_sk, as sk->protinfo is on a diet 8)
 * Fri Jul 22 19:51:12 MEST 2005 Harald Welte <laforge@gnumonks.org>
 * 				 - inc module use count of module that owns
 * 				   the kernel socket in case userspace opens
 * 				   socket of same protocol
 * 				 - remove all module support, since netlink is
 * 				   mandatory if CONFIG_NET=y these days
 */

#include <linux/module.h>

#include <linux/capability.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/socket.h>
#include <linux/un.h>
#include <linux/fcntl.h>
#include <linux/termios.h>
#include <linux/sockios.h>
#include <linux/net.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/notifier.h>
#include <linux/security.h>
#include <linux/jhash.h>
#include <linux/jiffies.h>
#include <linux/random.h>
#include <linux/bitops.h>
#include <linux/mm.h>
#include <linux/types.h>
#include <linux/audit.h>
#include <linux/mutex.h>
#include <linux/vmalloc.h>
#include <linux/if_arp.h>
#include <asm/cacheflush.h>

#include <net/net_namespace.h>
#include <net/sock.h>
#include <net/scm.h>
#include <net/netlink.h>

#include "af_netlink.h"

struct listeners {
	struct rcu_head		rcu;
	unsigned long		masks[0];
};

/* state bits 
 * 用来设置netlink_sock->state
 * */
#define NETLINK_CONGESTED	0x0     // 拥挤标志

/* flags 用来标识netlink套接字的属性
 * 用于netlink_sock->flags中
 * */
#define NETLINK_KERNEL_SOCKET	0x1     // 用来标识该netlink套接字是内核套接字
#define NETLINK_RECV_PKTINFO	0x2
#define NETLINK_BROADCAST_SEND_ERROR	0x4
#define NETLINK_RECV_NO_ENOBUFS	0x8

// 判断该netlink套接字是否是内核套接字
static inline int netlink_is_kernel(struct sock *sk)
{
	return nlk_sk(sk)->flags & NETLINK_KERNEL_SOCKET;
}

// 定义一张netlink接口总表，包含MAX_LINKS个元素，每个元素对应一种netlink协议,如NETLINK_ROUTE
struct netlink_table *nl_table;
EXPORT_SYMBOL_GPL(nl_table);

static DECLARE_WAIT_QUEUE_HEAD(nl_table_wait);

static int netlink_dump(struct sock *sk);
static void netlink_skb_destructor(struct sk_buff *skb);

DEFINE_RWLOCK(nl_table_lock);
EXPORT_SYMBOL_GPL(nl_table_lock);
static atomic_t nl_table_users = ATOMIC_INIT(0);

#define nl_deref_protected(X) rcu_dereference_protected(X, lockdep_is_held(&nl_table_lock));

static ATOMIC_NOTIFIER_HEAD(netlink_chain);

static DEFINE_SPINLOCK(netlink_tap_lock);
static struct list_head netlink_tap_all __read_mostly;

static inline u32 netlink_group_mask(u32 group)
{
	return group ? 1 << (group - 1) : 0;
}

// 根据portid计算出要插入的hash桶头节点
static inline struct hlist_head *nl_portid_hashfn(struct nl_portid_hash *hash, u32 portid)
{
	return &hash->table[jhash_1word(portid, hash->rnd) & hash->mask];
}

int netlink_add_tap(struct netlink_tap *nt)
{
	if (unlikely(nt->dev->type != ARPHRD_NETLINK))
		return -EINVAL;

	spin_lock(&netlink_tap_lock);
	list_add_rcu(&nt->list, &netlink_tap_all);
	spin_unlock(&netlink_tap_lock);

	if (nt->module)
		__module_get(nt->module);

	return 0;
}
EXPORT_SYMBOL_GPL(netlink_add_tap);

static int __netlink_remove_tap(struct netlink_tap *nt)
{
	bool found = false;
	struct netlink_tap *tmp;

	spin_lock(&netlink_tap_lock);

	list_for_each_entry(tmp, &netlink_tap_all, list) {
		if (nt == tmp) {
			list_del_rcu(&nt->list);
			found = true;
			goto out;
		}
	}

	pr_warn("__netlink_remove_tap: %p not found\n", nt);
out:
	spin_unlock(&netlink_tap_lock);

	if (found && nt->module)
		module_put(nt->module);

	return found ? 0 : -ENODEV;
}

int netlink_remove_tap(struct netlink_tap *nt)
{
	int ret;

	ret = __netlink_remove_tap(nt);
	synchronize_net();

	return ret;
}
EXPORT_SYMBOL_GPL(netlink_remove_tap);

static bool netlink_filter_tap(const struct sk_buff *skb)
{
	struct sock *sk = skb->sk;
	bool pass = false;

	/* We take the more conservative approach and
	 * whitelist socket protocols that may pass.
	 */
	switch (sk->sk_protocol) {
	case NETLINK_ROUTE:
	case NETLINK_USERSOCK:
	case NETLINK_SOCK_DIAG:
	case NETLINK_NFLOG:
	case NETLINK_XFRM:
	case NETLINK_FIB_LOOKUP:
	case NETLINK_NETFILTER:
	case NETLINK_GENERIC:
		pass = true;
		break;
	}

	return pass;
}

static int __netlink_deliver_tap_skb(struct sk_buff *skb,
				     struct net_device *dev)
{
	struct sk_buff *nskb;
	struct sock *sk = skb->sk;
	int ret = -ENOMEM;

	dev_hold(dev);
	nskb = skb_clone(skb, GFP_ATOMIC);
	if (nskb) {
		nskb->dev = dev;
		nskb->protocol = htons((u16) sk->sk_protocol);
		nskb->pkt_type = netlink_is_kernel(sk) ?
				 PACKET_KERNEL : PACKET_USER;
		skb_reset_network_header(nskb);
		ret = dev_queue_xmit(nskb);
		if (unlikely(ret > 0))
			ret = net_xmit_errno(ret);
	}

	dev_put(dev);
	return ret;
}

static void __netlink_deliver_tap(struct sk_buff *skb)
{
	int ret;
	struct netlink_tap *tmp;

	if (!netlink_filter_tap(skb))
		return;

	list_for_each_entry_rcu(tmp, &netlink_tap_all, list) {
		ret = __netlink_deliver_tap_skb(skb, tmp->dev);
		if (unlikely(ret))
			break;
	}
}

static void netlink_deliver_tap(struct sk_buff *skb)
{
	rcu_read_lock();

	if (unlikely(!list_empty(&netlink_tap_all)))
		__netlink_deliver_tap(skb);

	rcu_read_unlock();
}

static void netlink_deliver_tap_kernel(struct sock *dst, struct sock *src,
				       struct sk_buff *skb)
{
	if (!(netlink_is_kernel(dst) && netlink_is_kernel(src)))
		netlink_deliver_tap(skb);
}

// 对指定netlink套接字设置缓冲区溢出状态
static void netlink_overrun(struct sock *sk)
{
	struct netlink_sock *nlk = nlk_sk(sk);

	if (!(nlk->flags & NETLINK_RECV_NO_ENOBUFS)) {
		if (!test_and_set_bit(NETLINK_CONGESTED, &nlk_sk(sk)->state)) {
			sk->sk_err = ENOBUFS;
			sk->sk_error_report(sk);
		}
	}
    // 递增该netlink套接字的sk_drops计数值
	atomic_inc(&sk->sk_drops);
}

static void netlink_rcv_wake(struct sock *sk)
{
	struct netlink_sock *nlk = nlk_sk(sk);

	if (skb_queue_empty(&sk->sk_receive_queue))
		clear_bit(NETLINK_CONGESTED, &nlk->state);
	if (!test_bit(NETLINK_CONGESTED, &nlk->state))
		wake_up_interruptible(&nlk->wait);
}

// 根据内核是否配置了CONFIG_NETLINK_MMAP选项，来决定以下这些函数是否生效
#ifdef CONFIG_NETLINK_MMAP
static bool netlink_skb_is_mmaped(const struct sk_buff *skb)
{
	return NETLINK_CB(skb).flags & NETLINK_SKB_MMAPED;
}

static bool netlink_rx_is_mmaped(struct sock *sk)
{
	return nlk_sk(sk)->rx_ring.pg_vec != NULL;
}

static bool netlink_tx_is_mmaped(struct sock *sk)
{
	return nlk_sk(sk)->tx_ring.pg_vec != NULL;
}

static __pure struct page *pgvec_to_page(const void *addr)
{
	if (is_vmalloc_addr(addr))
		return vmalloc_to_page(addr);
	else
		return virt_to_page(addr);
}

static void free_pg_vec(void **pg_vec, unsigned int order, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++) {
		if (pg_vec[i] != NULL) {
			if (is_vmalloc_addr(pg_vec[i]))
				vfree(pg_vec[i]);
			else
				free_pages((unsigned long)pg_vec[i], order);
		}
	}
	kfree(pg_vec);
}

static void *alloc_one_pg_vec_page(unsigned long order)
{
	void *buffer;
	gfp_t gfp_flags = GFP_KERNEL | __GFP_COMP | __GFP_ZERO |
			  __GFP_NOWARN | __GFP_NORETRY;

	buffer = (void *)__get_free_pages(gfp_flags, order);
	if (buffer != NULL)
		return buffer;

	buffer = vzalloc((1 << order) * PAGE_SIZE);
	if (buffer != NULL)
		return buffer;

	gfp_flags &= ~__GFP_NORETRY;
	return (void *)__get_free_pages(gfp_flags, order);
}

static void **alloc_pg_vec(struct netlink_sock *nlk,
			   struct nl_mmap_req *req, unsigned int order)
{
	unsigned int block_nr = req->nm_block_nr;
	unsigned int i;
	void **pg_vec;

	pg_vec = kcalloc(block_nr, sizeof(void *), GFP_KERNEL);
	if (pg_vec == NULL)
		return NULL;

	for (i = 0; i < block_nr; i++) {
		pg_vec[i] = alloc_one_pg_vec_page(order);
		if (pg_vec[i] == NULL)
			goto err1;
	}

	return pg_vec;
err1:
	free_pg_vec(pg_vec, order, block_nr);
	return NULL;
}

static int netlink_set_ring(struct sock *sk, struct nl_mmap_req *req,
			    bool closing, bool tx_ring)
{
	struct netlink_sock *nlk = nlk_sk(sk);
	struct netlink_ring *ring;
	struct sk_buff_head *queue;
	void **pg_vec = NULL;
	unsigned int order = 0;
	int err;

	ring  = tx_ring ? &nlk->tx_ring : &nlk->rx_ring;
	queue = tx_ring ? &sk->sk_write_queue : &sk->sk_receive_queue;

	if (!closing) {
		if (atomic_read(&nlk->mapped))
			return -EBUSY;
		if (atomic_read(&ring->pending))
			return -EBUSY;
	}

	if (req->nm_block_nr) {
		if (ring->pg_vec != NULL)
			return -EBUSY;

		if ((int)req->nm_block_size <= 0)
			return -EINVAL;
		if (!IS_ALIGNED(req->nm_block_size, PAGE_SIZE))
			return -EINVAL;
		if (req->nm_frame_size < NL_MMAP_HDRLEN)
			return -EINVAL;
		if (!IS_ALIGNED(req->nm_frame_size, NL_MMAP_MSG_ALIGNMENT))
			return -EINVAL;

		ring->frames_per_block = req->nm_block_size /
					 req->nm_frame_size;
		if (ring->frames_per_block == 0)
			return -EINVAL;
		if (ring->frames_per_block * req->nm_block_nr !=
		    req->nm_frame_nr)
			return -EINVAL;

		order = get_order(req->nm_block_size);
		pg_vec = alloc_pg_vec(nlk, req, order);
		if (pg_vec == NULL)
			return -ENOMEM;
	} else {
		if (req->nm_frame_nr)
			return -EINVAL;
	}

	err = -EBUSY;
	mutex_lock(&nlk->pg_vec_lock);
	if (closing || atomic_read(&nlk->mapped) == 0) {
		err = 0;
		spin_lock_bh(&queue->lock);

		ring->frame_max		= req->nm_frame_nr - 1;
		ring->head		= 0;
		ring->frame_size	= req->nm_frame_size;
		ring->pg_vec_pages	= req->nm_block_size / PAGE_SIZE;

		swap(ring->pg_vec_len, req->nm_block_nr);
		swap(ring->pg_vec_order, order);
		swap(ring->pg_vec, pg_vec);

		__skb_queue_purge(queue);
		spin_unlock_bh(&queue->lock);

		WARN_ON(atomic_read(&nlk->mapped));
	}
	mutex_unlock(&nlk->pg_vec_lock);

	if (pg_vec)
		free_pg_vec(pg_vec, order, req->nm_block_nr);
	return err;
}

static void netlink_mm_open(struct vm_area_struct *vma)
{
	struct file *file = vma->vm_file;
	struct socket *sock = file->private_data;
	struct sock *sk = sock->sk;

	if (sk)
		atomic_inc(&nlk_sk(sk)->mapped);
}

static void netlink_mm_close(struct vm_area_struct *vma)
{
	struct file *file = vma->vm_file;
	struct socket *sock = file->private_data;
	struct sock *sk = sock->sk;

	if (sk)
		atomic_dec(&nlk_sk(sk)->mapped);
}

static const struct vm_operations_struct netlink_mmap_ops = {
	.open	= netlink_mm_open,
	.close	= netlink_mm_close,
};

static int netlink_mmap(struct file *file, struct socket *sock,
			struct vm_area_struct *vma)
{
	struct sock *sk = sock->sk;
	struct netlink_sock *nlk = nlk_sk(sk);
	struct netlink_ring *ring;
	unsigned long start, size, expected;
	unsigned int i;
	int err = -EINVAL;

	if (vma->vm_pgoff)
		return -EINVAL;

	mutex_lock(&nlk->pg_vec_lock);

	expected = 0;
	for (ring = &nlk->rx_ring; ring <= &nlk->tx_ring; ring++) {
		if (ring->pg_vec == NULL)
			continue;
		expected += ring->pg_vec_len * ring->pg_vec_pages * PAGE_SIZE;
	}

	if (expected == 0)
		goto out;

	size = vma->vm_end - vma->vm_start;
	if (size != expected)
		goto out;

	start = vma->vm_start;
	for (ring = &nlk->rx_ring; ring <= &nlk->tx_ring; ring++) {
		if (ring->pg_vec == NULL)
			continue;

		for (i = 0; i < ring->pg_vec_len; i++) {
			struct page *page;
			void *kaddr = ring->pg_vec[i];
			unsigned int pg_num;

			for (pg_num = 0; pg_num < ring->pg_vec_pages; pg_num++) {
				page = pgvec_to_page(kaddr);
				err = vm_insert_page(vma, start, page);
				if (err < 0)
					goto out;
				start += PAGE_SIZE;
				kaddr += PAGE_SIZE;
			}
		}
	}

	atomic_inc(&nlk->mapped);
	vma->vm_ops = &netlink_mmap_ops;
	err = 0;
out:
	mutex_unlock(&nlk->pg_vec_lock);
	return err;
}

static void netlink_frame_flush_dcache(const struct nl_mmap_hdr *hdr, unsigned int nm_len)
{
#if ARCH_IMPLEMENTS_FLUSH_DCACHE_PAGE == 1
	struct page *p_start, *p_end;

	/* First page is flushed through netlink_{get,set}_status */
	p_start = pgvec_to_page(hdr + PAGE_SIZE);
	p_end   = pgvec_to_page((void *)hdr + NL_MMAP_HDRLEN + nm_len - 1);
	while (p_start <= p_end) {
		flush_dcache_page(p_start);
		p_start++;
	}
#endif
}

static enum nl_mmap_status netlink_get_status(const struct nl_mmap_hdr *hdr)
{
	smp_rmb();
	flush_dcache_page(pgvec_to_page(hdr));
	return hdr->nm_status;
}

static void netlink_set_status(struct nl_mmap_hdr *hdr,
			       enum nl_mmap_status status)
{
	smp_mb();
	hdr->nm_status = status;
	flush_dcache_page(pgvec_to_page(hdr));
}

static struct nl_mmap_hdr *
__netlink_lookup_frame(const struct netlink_ring *ring, unsigned int pos)
{
	unsigned int pg_vec_pos, frame_off;

	pg_vec_pos = pos / ring->frames_per_block;
	frame_off  = pos % ring->frames_per_block;

	return ring->pg_vec[pg_vec_pos] + (frame_off * ring->frame_size);
}

static struct nl_mmap_hdr *
netlink_lookup_frame(const struct netlink_ring *ring, unsigned int pos,
		     enum nl_mmap_status status)
{
	struct nl_mmap_hdr *hdr;

	hdr = __netlink_lookup_frame(ring, pos);
	if (netlink_get_status(hdr) != status)
		return NULL;

	return hdr;
}

static struct nl_mmap_hdr *
netlink_current_frame(const struct netlink_ring *ring,
		      enum nl_mmap_status status)
{
	return netlink_lookup_frame(ring, ring->head, status);
}

static struct nl_mmap_hdr *
netlink_previous_frame(const struct netlink_ring *ring,
		       enum nl_mmap_status status)
{
	unsigned int prev;

	prev = ring->head ? ring->head - 1 : ring->frame_max;
	return netlink_lookup_frame(ring, prev, status);
}

static void netlink_increment_head(struct netlink_ring *ring)
{
	ring->head = ring->head != ring->frame_max ? ring->head + 1 : 0;
}

static void netlink_forward_ring(struct netlink_ring *ring)
{
	unsigned int head = ring->head, pos = head;
	const struct nl_mmap_hdr *hdr;

	do {
		hdr = __netlink_lookup_frame(ring, pos);
		if (hdr->nm_status == NL_MMAP_STATUS_UNUSED)
			break;
		if (hdr->nm_status != NL_MMAP_STATUS_SKIP)
			break;
		netlink_increment_head(ring);
	} while (ring->head != head);
}

static bool netlink_dump_space(struct netlink_sock *nlk)
{
	struct netlink_ring *ring = &nlk->rx_ring;
	struct nl_mmap_hdr *hdr;
	unsigned int n;

	hdr = netlink_current_frame(ring, NL_MMAP_STATUS_UNUSED);
	if (hdr == NULL)
		return false;

	n = ring->head + ring->frame_max / 2;
	if (n > ring->frame_max)
		n -= ring->frame_max;

	hdr = __netlink_lookup_frame(ring, n);

	return hdr->nm_status == NL_MMAP_STATUS_UNUSED;
}

static unsigned int netlink_poll(struct file *file, struct socket *sock,
				 poll_table *wait)
{
	struct sock *sk = sock->sk;
	struct netlink_sock *nlk = nlk_sk(sk);
	unsigned int mask;
	int err;

	if (nlk->rx_ring.pg_vec != NULL) {
		/* Memory mapped sockets don't call recvmsg(), so flow control
		 * for dumps is performed here. A dump is allowed to continue
		 * if at least half the ring is unused.
		 */
		while (nlk->cb_running && netlink_dump_space(nlk)) {
			err = netlink_dump(sk);
			if (err < 0) {
				sk->sk_err = -err;
				sk->sk_error_report(sk);
				break;
			}
		}
		netlink_rcv_wake(sk);
	}

	mask = datagram_poll(file, sock, wait);

	spin_lock_bh(&sk->sk_receive_queue.lock);
	if (nlk->rx_ring.pg_vec) {
		netlink_forward_ring(&nlk->rx_ring);
		if (!netlink_previous_frame(&nlk->rx_ring, NL_MMAP_STATUS_UNUSED))
			mask |= POLLIN | POLLRDNORM;
	}
	spin_unlock_bh(&sk->sk_receive_queue.lock);

	spin_lock_bh(&sk->sk_write_queue.lock);
	if (nlk->tx_ring.pg_vec) {
		if (netlink_current_frame(&nlk->tx_ring, NL_MMAP_STATUS_UNUSED))
			mask |= POLLOUT | POLLWRNORM;
	}
	spin_unlock_bh(&sk->sk_write_queue.lock);

	return mask;
}

static struct nl_mmap_hdr *netlink_mmap_hdr(struct sk_buff *skb)
{
	return (struct nl_mmap_hdr *)(skb->head - NL_MMAP_HDRLEN);
}

static void netlink_ring_setup_skb(struct sk_buff *skb, struct sock *sk,
				   struct netlink_ring *ring,
				   struct nl_mmap_hdr *hdr)
{
	unsigned int size;
	void *data;

	size = ring->frame_size - NL_MMAP_HDRLEN;
	data = (void *)hdr + NL_MMAP_HDRLEN;

	skb->head	= data;
	skb->data	= data;
	skb_reset_tail_pointer(skb);
	skb->end	= skb->tail + size;
	skb->len	= 0;

	skb->destructor	= netlink_skb_destructor;
	NETLINK_CB(skb).flags |= NETLINK_SKB_MMAPED;
	NETLINK_CB(skb).sk = sk;
}

static int netlink_mmap_sendmsg(struct sock *sk, struct msghdr *msg,
				u32 dst_portid, u32 dst_group,
				struct sock_iocb *siocb)
{
	struct netlink_sock *nlk = nlk_sk(sk);
	struct netlink_ring *ring;
	struct nl_mmap_hdr *hdr;
	struct sk_buff *skb;
	unsigned int maxlen;
	int err = 0, len = 0;

	mutex_lock(&nlk->pg_vec_lock);

	ring   = &nlk->tx_ring;
	maxlen = ring->frame_size - NL_MMAP_HDRLEN;

	do {
		unsigned int nm_len;

		hdr = netlink_current_frame(ring, NL_MMAP_STATUS_VALID);
		if (hdr == NULL) {
			if (!(msg->msg_flags & MSG_DONTWAIT) &&
			    atomic_read(&nlk->tx_ring.pending))
				schedule();
			continue;
		}

		nm_len = ACCESS_ONCE(hdr->nm_len);
		if (nm_len > maxlen) {
			err = -EINVAL;
			goto out;
		}

		netlink_frame_flush_dcache(hdr, nm_len);

		skb = alloc_skb(nm_len, GFP_KERNEL);
		if (skb == NULL) {
			err = -ENOBUFS;
			goto out;
		}
		__skb_put(skb, nm_len);
		memcpy(skb->data, (void *)hdr + NL_MMAP_HDRLEN, nm_len);
		netlink_set_status(hdr, NL_MMAP_STATUS_UNUSED);

		netlink_increment_head(ring);

		NETLINK_CB(skb).portid	  = nlk->portid;
		NETLINK_CB(skb).dst_group = dst_group;
		NETLINK_CB(skb).creds	  = siocb->scm->creds;

		err = security_netlink_send(sk, skb);
		if (err) {
			kfree_skb(skb);
			goto out;
		}

		if (unlikely(dst_group)) {
			atomic_inc(&skb->users);
			netlink_broadcast(sk, skb, dst_portid, dst_group,
					  GFP_KERNEL);
		}
		err = netlink_unicast(sk, skb, dst_portid,
				      msg->msg_flags & MSG_DONTWAIT);
		if (err < 0)
			goto out;
		len += err;

	} while (hdr != NULL ||
		 (!(msg->msg_flags & MSG_DONTWAIT) &&
		  atomic_read(&nlk->tx_ring.pending)));

	if (len > 0)
		err = len;
out:
	mutex_unlock(&nlk->pg_vec_lock);
	return err;
}

static void netlink_queue_mmaped_skb(struct sock *sk, struct sk_buff *skb)
{
	struct nl_mmap_hdr *hdr;

	hdr = netlink_mmap_hdr(skb);
	hdr->nm_len	= skb->len;
	hdr->nm_group	= NETLINK_CB(skb).dst_group;
	hdr->nm_pid	= NETLINK_CB(skb).creds.pid;
	hdr->nm_uid	= from_kuid(sk_user_ns(sk), NETLINK_CB(skb).creds.uid);
	hdr->nm_gid	= from_kgid(sk_user_ns(sk), NETLINK_CB(skb).creds.gid);
	netlink_frame_flush_dcache(hdr, hdr->nm_len);
	netlink_set_status(hdr, NL_MMAP_STATUS_VALID);

	NETLINK_CB(skb).flags |= NETLINK_SKB_DELIVERED;
	kfree_skb(skb);
}

static void netlink_ring_set_copied(struct sock *sk, struct sk_buff *skb)
{
	struct netlink_sock *nlk = nlk_sk(sk);
	struct netlink_ring *ring = &nlk->rx_ring;
	struct nl_mmap_hdr *hdr;

	spin_lock_bh(&sk->sk_receive_queue.lock);
	hdr = netlink_current_frame(ring, NL_MMAP_STATUS_UNUSED);
	if (hdr == NULL) {
		spin_unlock_bh(&sk->sk_receive_queue.lock);
		kfree_skb(skb);
		netlink_overrun(sk);
		return;
	}
	netlink_increment_head(ring);
	__skb_queue_tail(&sk->sk_receive_queue, skb);
	spin_unlock_bh(&sk->sk_receive_queue.lock);

	hdr->nm_len	= skb->len;
	hdr->nm_group	= NETLINK_CB(skb).dst_group;
	hdr->nm_pid	= NETLINK_CB(skb).creds.pid;
	hdr->nm_uid	= from_kuid(sk_user_ns(sk), NETLINK_CB(skb).creds.uid);
	hdr->nm_gid	= from_kgid(sk_user_ns(sk), NETLINK_CB(skb).creds.gid);
	netlink_set_status(hdr, NL_MMAP_STATUS_COPY);
}

#else /* CONFIG_NETLINK_MMAP */
#define netlink_skb_is_mmaped(skb)	false
#define netlink_rx_is_mmaped(sk)	false
#define netlink_tx_is_mmaped(sk)	false
#define netlink_mmap			sock_no_mmap
#define netlink_poll			datagram_poll
#define netlink_mmap_sendmsg(sk, msg, dst_portid, dst_group, siocb)	0
#endif /* CONFIG_NETLINK_MMAP */

static void netlink_skb_destructor(struct sk_buff *skb)
{
#ifdef CONFIG_NETLINK_MMAP
	struct nl_mmap_hdr *hdr;
	struct netlink_ring *ring;
	struct sock *sk;

	/* If a packet from the kernel to userspace was freed because of an
	 * error without being delivered to userspace, the kernel must reset
	 * the status. In the direction userspace to kernel, the status is
	 * always reset here after the packet was processed and freed.
	 */
	if (netlink_skb_is_mmaped(skb)) {
		hdr = netlink_mmap_hdr(skb);
		sk = NETLINK_CB(skb).sk;

		if (NETLINK_CB(skb).flags & NETLINK_SKB_TX) {
			netlink_set_status(hdr, NL_MMAP_STATUS_UNUSED);
			ring = &nlk_sk(sk)->tx_ring;
		} else {
			if (!(NETLINK_CB(skb).flags & NETLINK_SKB_DELIVERED)) {
				hdr->nm_len = 0;
				netlink_set_status(hdr, NL_MMAP_STATUS_VALID);
			}
			ring = &nlk_sk(sk)->rx_ring;
		}

		WARN_ON(atomic_read(&ring->pending) == 0);
		atomic_dec(&ring->pending);
		sock_put(sk);

		skb->head = NULL;
	}
#endif
	if (is_vmalloc_addr(skb->head)) {
		if (!skb->cloned ||
		    !atomic_dec_return(&(skb_shinfo(skb)->dataref)))
			vfree(skb->head);

		skb->head = NULL;
	}
	if (skb->sk != NULL)
		sock_rfree(skb);
}

/* 设置该携带了netlink消息的skb为哪个netlink套接字所拥有
 * @skb - 指向一个skb结构
 * @sk  - 指向将会拥有该skb的sock结构，也就是netlink套接字
 */
static void netlink_skb_set_owner_r(struct sk_buff *skb, struct sock *sk)
{
	WARN_ON(skb->sk != NULL);
	skb->sk = sk;                                   // 设置该skb属于指定的netlink套接字
	skb->destructor = netlink_skb_destructor;       // 为该skb注册特定的析构函数
	atomic_add(skb->truesize, &sk->sk_rmem_alloc);  // 该netlink套接字绑定了一个skb后，势必要增加其接收队列中的已使用空间大小
	sk_mem_charge(sk, skb->truesize);               // 以及减少发送队列剩余可用空间大小
}

// netlink sock结构的析构函数
static void netlink_sock_destruct(struct sock *sk)
{
	struct netlink_sock *nlk = nlk_sk(sk);

	if (nlk->cb_running) {
		if (nlk->cb.done)
			nlk->cb.done(&nlk->cb);

		module_put(nlk->cb.module);
		kfree_skb(nlk->cb.skb);
	}

	skb_queue_purge(&sk->sk_receive_queue);
#ifdef CONFIG_NETLINK_MMAP
	if (1) {
		struct nl_mmap_req req;

		memset(&req, 0, sizeof(req));
		if (nlk->rx_ring.pg_vec)
			netlink_set_ring(sk, &req, true, false);
		memset(&req, 0, sizeof(req));
		if (nlk->tx_ring.pg_vec)
			netlink_set_ring(sk, &req, true, true);
	}
#endif /* CONFIG_NETLINK_MMAP */

	if (!sock_flag(sk, SOCK_DEAD)) {
		printk(KERN_ERR "Freeing alive netlink socket %p\n", sk);
		return;
	}

	WARN_ON(atomic_read(&sk->sk_rmem_alloc));
	WARN_ON(atomic_read(&sk->sk_wmem_alloc));
	WARN_ON(nlk_sk(sk)->groups);
}

/* This lock without WQ_FLAG_EXCLUSIVE is good on UP and it is _very_ bad on
 * SMP. Look, when several writers sleep and reader wakes them up, all but one
 * immediately hit write lock and grab all the cpus. Exclusive sleep solves
 * this, _but_ remember, it adds useless work on UP machines.
 */

void netlink_table_grab(void)
	__acquires(nl_table_lock)
{
	might_sleep();

	write_lock_irq(&nl_table_lock);

	if (atomic_read(&nl_table_users)) {
		DECLARE_WAITQUEUE(wait, current);

		add_wait_queue_exclusive(&nl_table_wait, &wait);
		for (;;) {
			set_current_state(TASK_UNINTERRUPTIBLE);
			if (atomic_read(&nl_table_users) == 0)
				break;
			write_unlock_irq(&nl_table_lock);
			schedule();
			write_lock_irq(&nl_table_lock);
		}

		__set_current_state(TASK_RUNNING);
		remove_wait_queue(&nl_table_wait, &wait);
	}
}

void netlink_table_ungrab(void)
	__releases(nl_table_lock)
{
	write_unlock_irq(&nl_table_lock);
	wake_up(&nl_table_wait);
}

static inline void
netlink_lock_table(void)
{
	/* read_lock() synchronizes us to netlink_table_grab */

	read_lock(&nl_table_lock);
	atomic_inc(&nl_table_users);
	read_unlock(&nl_table_lock);
}

static inline void
netlink_unlock_table(void)
{
	if (atomic_dec_and_test(&nl_table_users))
		wake_up(&nl_table_wait);
}

// 缺省的net命名空间比较函数，比较net是否相同，从而判断是否属于同一个网络命名空间
static bool netlink_compare(struct net *net, struct sock *sk)
{
	return net_eq(sock_net(sk), net);
}

// 根据protocol和portid查找对应的netlink套接字
static struct sock *netlink_lookup(struct net *net, int protocol, u32 portid)
{
	struct netlink_table *table = &nl_table[protocol];
	struct nl_portid_hash *hash = &table->hash;
	struct hlist_head *head;
	struct sock *sk;

	read_lock(&nl_table_lock);
    // 根据portid计算的到对应的hash表项
	head = nl_portid_hashfn(hash, portid);
    // 根据注册的比较函数遍历该表项，查找匹配节点
	sk_for_each(sk, head) {
		if (table->compare(net, sk) &&
		    (nlk_sk(sk)->portid == portid)) {
			sock_hold(sk);
			goto found;
		}
	}
	sk = NULL;
found:
	read_unlock(&nl_table_lock);
	return sk;
}

static struct hlist_head *nl_portid_hash_zalloc(size_t size)
{
	if (size <= PAGE_SIZE)
		return kzalloc(size, GFP_ATOMIC);
	else
		return (struct hlist_head *)
			__get_free_pages(GFP_ATOMIC | __GFP_ZERO,
					 get_order(size));
}

static void nl_portid_hash_free(struct hlist_head *table, size_t size)
{
	if (size <= PAGE_SIZE)
		kfree(table);
	else
		free_pages((unsigned long)table, get_order(size));
}

static int nl_portid_hash_rehash(struct nl_portid_hash *hash, int grow)
{
	unsigned int omask, mask, shift;
	size_t osize, size;
	struct hlist_head *otable, *table;
	int i;

	omask = mask = hash->mask;
	osize = size = (mask + 1) * sizeof(*table);
	shift = hash->shift;

	if (grow) {
		if (++shift > hash->max_shift)
			return 0;
		mask = mask * 2 + 1;
		size *= 2;
	}

	table = nl_portid_hash_zalloc(size);
	if (!table)
		return 0;

	otable = hash->table;
	hash->table = table;
	hash->mask = mask;
	hash->shift = shift;
	get_random_bytes(&hash->rnd, sizeof(hash->rnd));

	for (i = 0; i <= omask; i++) {
		struct sock *sk;
		struct hlist_node *tmp;

		sk_for_each_safe(sk, tmp, &otable[i])
			__sk_add_node(sk, nl_portid_hashfn(hash, nlk_sk(sk)->portid));
	}

	nl_portid_hash_free(otable, osize);
	hash->rehash_time = jiffies + 10 * 60 * HZ;
	return 1;
}

static inline int nl_portid_hash_dilute(struct nl_portid_hash *hash, int len)
{
	int avg = hash->entries >> hash->shift;

	if (unlikely(avg > 1) && nl_portid_hash_rehash(hash, 1))
		return 1;

	if (unlikely(len > avg) && time_after(jiffies, hash->rehash_time)) {
		nl_portid_hash_rehash(hash, 0);
		return 1;
	}

	return 0;
}

static const struct proto_ops netlink_ops;

// 更新套接字所属协议类型的监听集合
static void netlink_update_listeners(struct sock *sk)
{
	struct netlink_table *tbl = &nl_table[sk->sk_protocol];
	unsigned long mask;
	unsigned int i;
	struct listeners *listeners;

	listeners = nl_deref_protected(tbl->listeners);
	if (!listeners)
		return;

    // 以下的for实际应该只循环了1次
	for (i = 0; i < NLGRPLONGS(tbl->groups); i++) {
		mask = 0;
        // 遍历该协议类型下的组播hash链表，收集所有关心的组播消息
		sk_for_each_bound(sk, &tbl->mc_list) {
			if (i < NLGRPLONGS(nlk_sk(sk)->ngroups))
				mask |= nlk_sk(sk)->groups[i];
		}
		listeners->masks[i] = mask;
	}
	/* this function is only called with the netlink table "grabbed", which
	 * makes sure updates are visible before bind or setsockopt return. */
}

/* 将netlink套接字添加到nl_table中的相应的hash表中
 */
static int netlink_insert(struct sock *sk, struct net *net, u32 portid)
{
	struct netlink_table *table = &nl_table[sk->sk_protocol];
	struct nl_portid_hash *hash = &table->hash;
	struct hlist_head *head;
	int err = -EADDRINUSE;
	struct sock *osk;
	int len;

	netlink_table_grab();
    // 首先根据portid计算出将要插入的hash桶头节点
	head = nl_portid_hashfn(hash, portid);
	len = 0;
    // 遍历链表，通过注册的compare函数判断对应net命名空间下是否存在相同portid的netlink套接字
	sk_for_each(osk, head) {
		if (table->compare(net, osk) &&
		    (nlk_sk(osk)->portid == portid))
			break;
		len++;
	}
    // osk有效意味着已经有相同portid的netlink套接字注册在相同的net命名空间，所以不通过
	if (osk)
		goto err;

	err = -EBUSY;
	if (nlk_sk(sk)->portid)
		goto err;

	err = -ENOMEM;
	if (BITS_PER_LONG > 32 && unlikely(hash->entries >= UINT_MAX))
		goto err;

	if (len && nl_portid_hash_dilute(hash, len))
		head = nl_portid_hashfn(hash, portid);
	hash->entries++;
    // 绑定成功后就在这里更新该netlink套接字单播地址
	nlk_sk(sk)->portid = portid;
    // 最后将符合条件的netlink套接字加入hash表
	sk_add_node(sk, head);
	err = 0;

err:
	netlink_table_ungrab();
	return err;
}

static void netlink_remove(struct sock *sk)
{
	netlink_table_grab();
	if (sk_del_node_init(sk))
		nl_table[sk->sk_protocol].hash.entries--;
	if (nlk_sk(sk)->subscriptions)
		__sk_del_bind_node(sk);
	netlink_table_ungrab();
}

// 定义了一个所有netlink协议共有的proto结构
static struct proto netlink_proto = {
	.name	  = "NETLINK",
	.owner	  = THIS_MODULE,
	.obj_size = sizeof(struct netlink_sock),
};

/* 创建并初始化一个netlink_sock结构(其中包含了sock结构)
 * 本函数是创建netlink套接字的核心，在2种情况下会被调用到：
 *          1. 内核主动创建完全属于内核的netlink套接字时
 *          2. 由用户进程调用socket()系统调用，从而在内核中创建属于用户进程的netlink套接字时
 */
static int __netlink_create(struct net *net, struct socket *sock,
			    struct mutex *cb_mutex, int protocol)
{
	struct sock *sk;
	struct netlink_sock *nlk;

    // netlink套接字和socket层netlink协议族的通用操作集合关联
	sock->ops = &netlink_ops;

    // 分配基于netlink协议的sock结构
	sk = sk_alloc(net, PF_NETLINK, GFP_KERNEL, &netlink_proto);
	if (!sk)
		return -ENOMEM;

    // 初始化sock的发送接收队列、数据缓存、等待队列和互斥锁等
	sock_init_data(sock, sk);

	nlk = nlk_sk(sk);
	if (cb_mutex) {
		nlk->cb_mutex = cb_mutex;
	} else {
		nlk->cb_mutex = &nlk->cb_def_mutex;
		mutex_init(nlk->cb_mutex);
	}
	init_waitqueue_head(&nlk->wait);
#ifdef CONFIG_NETLINK_MMAP
	mutex_init(&nlk->pg_vec_lock);
#endif

	sk->sk_destruct = netlink_sock_destruct;    // 设置netlink sock结构的析构函数
	sk->sk_protocol = protocol;
	return 0;
}

/* 应用层创建PF_NETLINK类型的套接字时就会在__sock_create中调用到这里
 * 
 * 备注：其实本函数只是创建了半个netlink套接字(sock结构)，另一半在__sock_create中创建(socket结构)
 */
static int netlink_create(struct net *net, struct socket *sock, int protocol,
			  int kern)
{
	struct module *module = NULL;
	struct mutex *cb_mutex;
	struct netlink_sock *nlk;
	void (*bind)(int group);
	int err = 0;

    // 首先将套接字状态标记为未连接
	sock->state = SS_UNCONNECTED;

    // netlink接口只支持SOCK_RAW和SOCK_DGRAM这两种套接字类型
	if (sock->type != SOCK_RAW && sock->type != SOCK_DGRAM)
		return -ESOCKTNOSUPPORT;

    // 检查具体的协议类型是否有效
	if (protocol < 0 || protocol >= MAX_LINKS)
		return -EPROTONOSUPPORT;

    // 以下这个锁区域主要是完成了几个赋值操作
	netlink_lock_table();
#ifdef CONFIG_MODULES
	if (!nl_table[protocol].registered) {
		netlink_unlock_table();
		request_module("net-pf-%d-proto-%d", PF_NETLINK, protocol);
		netlink_lock_table();
	}
#endif
    // 检查该协议类型的内核netlink套接字是否已经注册了，只有nl_table中已经注册的协议类型才能继续创建下去
	if (nl_table[protocol].registered &&
	    try_module_get(nl_table[protocol].module))
		module = nl_table[protocol].module;
	else
		err = -EPROTONOSUPPORT;

    // 取出内核netlink创建时注册的私有锁和bind函数，接下来就要用到了
	cb_mutex = nl_table[protocol].cb_mutex;
	bind = nl_table[protocol].bind;
	netlink_unlock_table();

	if (err < 0)
		goto out;

    /* 创建并初始化一个协议类型相关的sock结构
     *
     * 备注：跟内核主动创建netlink套接字时不同的是，创建属于用户进程的netlink套接字似乎不再关心传入的net命名空间
     */
	err = __netlink_create(net, sock, cb_mutex, protocol);
	if (err < 0)
		goto out_module;

	local_bh_disable();
	sock_prot_inuse_add(net, &netlink_proto, 1);
	local_bh_enable();

    // 最后对netlink套接字相关参数进行赋值
	nlk = nlk_sk(sock->sk);
	nlk->module = module;
	nlk->netlink_bind = bind;
out:
	return err;

out_module:
	module_put(module);
	goto out;
}

static int netlink_release(struct socket *sock)
{
	struct sock *sk = sock->sk;
	struct netlink_sock *nlk;

	if (!sk)
		return 0;

	netlink_remove(sk);
	sock_orphan(sk);
	nlk = nlk_sk(sk);

	/*
	 * OK. Socket is unlinked, any packets that arrive now
	 * will be purged.
	 */

	sock->sk = NULL;
	wake_up_interruptible_all(&nlk->wait);

	skb_queue_purge(&sk->sk_write_queue);

	if (nlk->portid) {
		struct netlink_notify n = {
						.net = sock_net(sk),
						.protocol = sk->sk_protocol,
						.portid = nlk->portid,
					  };
		atomic_notifier_call_chain(&netlink_chain,
				NETLINK_URELEASE, &n);
	}

	module_put(nlk->module);

	netlink_table_grab();
	if (netlink_is_kernel(sk)) {
		BUG_ON(nl_table[sk->sk_protocol].registered == 0);
		if (--nl_table[sk->sk_protocol].registered == 0) {
			struct listeners *old;

			old = nl_deref_protected(nl_table[sk->sk_protocol].listeners);
			RCU_INIT_POINTER(nl_table[sk->sk_protocol].listeners, NULL);
			kfree_rcu(old, rcu);
			nl_table[sk->sk_protocol].module = NULL;
			nl_table[sk->sk_protocol].bind = NULL;
			nl_table[sk->sk_protocol].flags = 0;
			nl_table[sk->sk_protocol].registered = 0;
		}
	} else if (nlk->subscriptions) {
		netlink_update_listeners(sk);
	}
	netlink_table_ungrab();

	kfree(nlk->groups);
	nlk->groups = NULL;

	local_bh_disable();
	sock_prot_inuse_add(sock_net(sk), &netlink_proto, -1);
	local_bh_enable();
	sock_put(sk);
	return 0;
}

// 内核将一个netlink套接字自动绑定到某个单播地址上
static int netlink_autobind(struct socket *sock)
{
	struct sock *sk = sock->sk;
	struct net *net = sock_net(sk);
	struct netlink_table *table = &nl_table[sk->sk_protocol];
	struct nl_portid_hash *hash = &table->hash;
	struct hlist_head *head;
	struct sock *osk;
	s32 portid = task_tgid_vnr(current);    // 获取当前进程ID
	int err;
	static s32 rover = -4097;

    /* 首先尝试用当前进程ID作为绑定的单播地址；
     * 如果当前进程ID已经绑过其他的相同协议类型的netlink套接字，则选用rover为基准进行查找，直到找到可用的值
     * 最后用找到的可用值作为该netlink的单播地址，执行绑定操作
     */
retry:
	cond_resched();
	netlink_table_grab();
	head = nl_portid_hashfn(hash, portid);
	sk_for_each(osk, head) {
		if (!table->compare(net, osk))
			continue;
		if (nlk_sk(osk)->portid == portid) {
			/* Bind collision, search negative portid values. */
			portid = rover--;
			if (rover > -4097)
				rover = -4097;
			netlink_table_ungrab();
			goto retry;
		}
	}
	netlink_table_ungrab();

    // 根据找到的portid完成绑定
	err = netlink_insert(sk, net, portid);
	if (err == -EADDRINUSE)
		goto retry;

	/* If 2 threads race to autobind, that is fine.  */
	if (err == -EBUSY)
		err = 0;

	return err;
}

/**
 * __netlink_ns_capable - General netlink message capability test
 * @nsp: NETLINK_CB of the socket buffer holding a netlink command from userspace.
 * @user_ns: The user namespace of the capability to use
 * @cap: The capability to use
 *
 * Test to see if the opener of the socket we received the message
 * from had when the netlink socket was created and the sender of the
 * message has has the capability @cap in the user namespace @user_ns.
 */
bool __netlink_ns_capable(const struct netlink_skb_parms *nsp,
			struct user_namespace *user_ns, int cap)
{
	return ((nsp->flags & NETLINK_SKB_DST) ||
		file_ns_capable(nsp->sk->sk_socket->file, user_ns, cap)) &&
		ns_capable(user_ns, cap);
}
EXPORT_SYMBOL(__netlink_ns_capable);

/**
 * netlink_ns_capable - General netlink message capability test
 * @skb: socket buffer holding a netlink command from userspace
 * @user_ns: The user namespace of the capability to use
 * @cap: The capability to use
 *
 * Test to see if the opener of the socket we received the message
 * from had when the netlink socket was created and the sender of the
 * message has has the capability @cap in the user namespace @user_ns.
 */
bool netlink_ns_capable(const struct sk_buff *skb,
			struct user_namespace *user_ns, int cap)
{
	return __netlink_ns_capable(&NETLINK_CB(skb), user_ns, cap);
}
EXPORT_SYMBOL(netlink_ns_capable);

/**
 * netlink_capable - Netlink global message capability test
 * @skb: socket buffer holding a netlink command from userspace
 * @cap: The capability to use
 *
 * Test to see if the opener of the socket we received the message
 * from had when the netlink socket was created and the sender of the
 * message has has the capability @cap in all user namespaces.
 */
bool netlink_capable(const struct sk_buff *skb, int cap)
{
	return netlink_ns_capable(skb, &init_user_ns, cap);
}
EXPORT_SYMBOL(netlink_capable);

/**
 * netlink_net_capable - Netlink network namespace message capability test
 * @skb: socket buffer holding a netlink command from userspace
 * @cap: The capability to use
 *
 * Test to see if the opener of the socket we received the message
 * from had when the netlink socket was created and the sender of the
 * message has has the capability @cap over the network namespace of
 * the socket we received the message from.
 */
bool netlink_net_capable(const struct sk_buff *skb, int cap)
{
	return netlink_ns_capable(skb, sock_net(skb->sk)->user_ns, cap);
}
EXPORT_SYMBOL(netlink_net_capable);

/* 判断该netlink套接字所属的具体协议类型是否具有flag标识的某项权限
 * 
 * 备注：对于超级用户，flag标识的权限都开放；对于非超级用户，则只有已经被设置为允许的操作才开放
 */
static inline int netlink_allowed(const struct socket *sock, unsigned int flag)
{
	return (nl_table[sock->sk->sk_protocol].flags & flag) ||
		ns_capable(sock_net(sock->sk)->user_ns, CAP_NET_ADMIN);
}

// 将指定netlink套接字加入所属netlink协议类型的多播hash链表
static void netlink_update_subscriptions(struct sock *sk, unsigned int subscriptions)
{
	struct netlink_sock *nlk = nlk_sk(sk);

	if (nlk->subscriptions && !subscriptions)
        /* 如果该netlink套接字有阅订过组播消息，并且当前传入组播数量为0,
         * 意味不再阅订组播，所以需要将该netlink套接字从组播hash桶中删除
         */
		__sk_del_bind_node(sk);
	else if (!nlk->subscriptions && subscriptions)
        /* 如果该netlink套接字第一次阅订组播消息，
         * 那么将该netlink套接字作为一个普通节点插入头节点list对应的hash桶中
         */
		sk_add_bind_node(sk, &nl_table[sk->sk_protocol].mc_list);
    
    // 更新该netlink套接字阅订的组播数量
	nlk->subscriptions = subscriptions;
}

// 为指定套接字分配组播空间
static int netlink_realloc_groups(struct sock *sk)
{
	struct netlink_sock *nlk = nlk_sk(sk);
	unsigned int groups;
	unsigned long *new_groups;
	int err = 0;

	netlink_table_grab();

	groups = nl_table[sk->sk_protocol].groups;
	if (!nl_table[sk->sk_protocol].registered) {
		err = -ENOENT;
		goto out_unlock;
	}

	if (nlk->ngroups >= groups)
		goto out_unlock;

	new_groups = krealloc(nlk->groups, NLGRPSZ(groups), GFP_ATOMIC);
	if (new_groups == NULL) {
		err = -ENOMEM;
		goto out_unlock;
	}
	memset((char *)new_groups + NLGRPSZ(nlk->ngroups), 0,
	       NLGRPSZ(groups) - NLGRPSZ(nlk->ngroups));

    // 记录下分配的组播空间信息
	nlk->groups = new_groups;
	nlk->ngroups = groups;
 out_unlock:
	netlink_table_ungrab();
	return err;
}

/* 用户态对netlink接口的套接字执行系统调用bind()时就会进入到这里
 * @sock    - 要绑定的socket结构，也就是套接字
 * @addr    - 要绑定的套接字地址
 * @addr_len- 套接字地址长度
 */
static int netlink_bind(struct socket *sock, struct sockaddr *addr,
			int addr_len)
{
	struct sock *sk = sock->sk;
	struct net *net = sock_net(sk);
	struct netlink_sock *nlk = nlk_sk(sk);
	struct sockaddr_nl *nladdr = (struct sockaddr_nl *)addr;
	int err;

    // 首先是入参合法性检测
	if (addr_len < sizeof(struct sockaddr_nl))
		return -EINVAL;

	if (nladdr->nl_family != AF_NETLINK)
		return -EINVAL;

	/* Only superuser is allowed to listen multicasts 
     *
     * 如果传入的套接字地址中指定了要监听的组播，需要判断该套接字是否具有监听组播的权限
     * 备注：上面的英文描述有问题，实际不只是root用户
     * */
	if (nladdr->nl_groups) {
		if (!netlink_allowed(sock, NL_CFG_F_NONROOT_RECV))
			return -EPERM;
        // 为该套接字分配组播空间
		err = netlink_realloc_groups(sk);
		if (err)
			return err;
	}

     /* 需要注意的一点就是，如果netlink套接字已经绑定在一个地址上，就不能再绑到一个新的地址.
     *      对于尚未绑定的netlink套接字，这时候如果传入的套接字地址中指定了要绑定的单播地址就用该地址绑定;
     *      如果没有指定就自动绑定一个
     */
	if (nlk->portid) {
		if (nladdr->nl_pid != nlk->portid)
			return -EINVAL;
	} else {
        // 绑定netlink套接字的过程，其实就是将其加入nl_table的过程
		err = nladdr->nl_pid ?
			netlink_insert(sk, net, nladdr->nl_pid) :
			netlink_autobind(sock);
		if (err)
			return err;
	}

    // 如果用户态没有指定组播地址，并且没有分配组播的内存，那么bind工作就到此结束了
	if (!nladdr->nl_groups && (nlk->groups == NULL || !(u32)nlk->groups[0]))
		return 0;

    // 程序运行到这里意味着用户态指定了需要绑定的组播地址
	netlink_table_grab();
    // 将指定套接字加入所属netlink协议类型的多播hash链表
	netlink_update_subscriptions(sk, nlk->subscriptions +
					 hweight32(nladdr->nl_groups) -
					 hweight32(nlk->groups[0]));
    // 保存组播地址mask值到开头申请的组播空间
	nlk->groups[0] = (nlk->groups[0] & ~0xffffffffUL) | nladdr->nl_groups;
    // 更新套接字所属协议类型的监听集合
	netlink_update_listeners(sk);
	netlink_table_ungrab();

    // 如果具体的netlink协议类型注册了私有的bind函数，并且用户态指定了要绑定的组播地址，则在这里调用该私有的bind函数
	if (nlk->netlink_bind && nlk->groups[0]) {
		int i;

		for (i=0; i<nlk->ngroups; i++) {
			if (test_bit(i, nlk->groups))
				nlk->netlink_bind(i);
		}
	}

	return 0;
}

static int netlink_connect(struct socket *sock, struct sockaddr *addr,
			   int alen, int flags)
{
	int err = 0;
	struct sock *sk = sock->sk;
	struct netlink_sock *nlk = nlk_sk(sk);
	struct sockaddr_nl *nladdr = (struct sockaddr_nl *)addr;

	if (alen < sizeof(addr->sa_family))
		return -EINVAL;

	if (addr->sa_family == AF_UNSPEC) {
		sk->sk_state	= NETLINK_UNCONNECTED;
		nlk->dst_portid	= 0;
		nlk->dst_group  = 0;
		return 0;
	}
	if (addr->sa_family != AF_NETLINK)
		return -EINVAL;

	if ((nladdr->nl_groups || nladdr->nl_pid) &&
	    !netlink_allowed(sock, NL_CFG_F_NONROOT_SEND))
		return -EPERM;

	if (!nlk->portid)
		err = netlink_autobind(sock);

	if (err == 0) {
		sk->sk_state	= NETLINK_CONNECTED;
		nlk->dst_portid = nladdr->nl_pid;
		nlk->dst_group  = ffs(nladdr->nl_groups);
	}

	return err;
}

static int netlink_getname(struct socket *sock, struct sockaddr *addr,
			   int *addr_len, int peer)
{
	struct sock *sk = sock->sk;
	struct netlink_sock *nlk = nlk_sk(sk);
	DECLARE_SOCKADDR(struct sockaddr_nl *, nladdr, addr);

	nladdr->nl_family = AF_NETLINK;
	nladdr->nl_pad = 0;
	*addr_len = sizeof(*nladdr);

	if (peer) {
		nladdr->nl_pid = nlk->dst_portid;
		nladdr->nl_groups = netlink_group_mask(nlk->dst_group);
	} else {
		nladdr->nl_pid = nlk->portid;
		nladdr->nl_groups = nlk->groups ? nlk->groups[0] : 0;
	}
	return 0;
}

/* 根据源sock结构和目的单播地址，得到目的sock结构
 *
 * @ssk         - 源sock结构
 * @portid      - 目的单播地址
 */
static struct sock *netlink_getsockbyportid(struct sock *ssk, u32 portid)
{
	struct sock *sock;
	struct netlink_sock *nlk;

    // 根据protocol和portid查找对应的netlink套接字，也就是目的netlink套接字
	sock = netlink_lookup(sock_net(ssk), ssk->sk_protocol, portid);
	if (!sock)
		return ERR_PTR(-ECONNREFUSED);

	/* Don't bother queuing skb if kernel socket has no input function 
     *
     * 对于已经处于connect状态的目的netlink套接字，还需要确认两端是否匹配
     * */
	nlk = nlk_sk(sock);
	if (sock->sk_state == NETLINK_CONNECTED &&
	    nlk->dst_portid != nlk_sk(ssk)->portid) {
		sock_put(sock);
		return ERR_PTR(-ECONNREFUSED);
	}
	return sock;
}

struct sock *netlink_getsockbyfilp(struct file *filp)
{
	struct inode *inode = file_inode(filp);
	struct sock *sock;

	if (!S_ISSOCK(inode->i_mode))
		return ERR_PTR(-ENOTSOCK);

	sock = SOCKET_I(inode)->sk;
	if (sock->sk_family != AF_NETLINK)
		return ERR_PTR(-EINVAL);

	sock_hold(sock);
	return sock;
}

/* 分配一个用于承载netlink消息的skb
 * @size    - 要分配的skb数据缓冲区大小
 * @broadcast   - 目的组播地址
 *
 * 备注：跟netlink_alloc_skb的区别在于本函数用于分配大尺寸skb数据缓冲区
 */
static struct sk_buff *netlink_alloc_large_skb(unsigned int size,
					       int broadcast)
{
	struct sk_buff *skb;
	void *data;

	if (size <= NLMSG_GOODSIZE || broadcast)
		return alloc_skb(size, GFP_KERNEL);

	size = SKB_DATA_ALIGN(size) +
	       SKB_DATA_ALIGN(sizeof(struct skb_shared_info));

	data = vmalloc(size);
	if (data == NULL)
		return NULL;

	skb = build_skb(data, size);
	if (skb == NULL)
		vfree(data);
	else {
		skb->head_frag = 0;
		skb->destructor = netlink_skb_destructor;
	}

	return skb;
}

/*
 * Attach a skb to a netlink socket.
 * 将一个指定skb绑定到一个指定的属于用户进程的netlink套接字上
 * @sk      - 指向目的sock结构
 * @skb     - 属于发送方的承载了netlink消息的skb
 * @timeo   - 指向一个超时时间
 * @ssk     - 指向源sock结构
 *
 * The caller must hold a reference to the destination socket. On error, the
 * reference is dropped. The skb is not send to the destination, just all
 * all error checks are performed and memory in the queue is reserved.
 * Return values:
 *
 * < 0: error. skb freed, reference to sock dropped.
 * 0: continue
 * 1: repeat lookup - reference dropped while waiting for socket memory.
 */
int netlink_attachskb(struct sock *sk, struct sk_buff *skb,
		      long *timeo, struct sock *ssk)
{
	struct netlink_sock *nlk;

    // 获取目的netlink套接字，也就是目的用户进程netlink套接字
	nlk = nlk_sk(sk);

    /* 在没有内核使能CONFIG_NETLINK_MMAP配置选项时，
     * 如果目的netlink套接字上已经接收尚未处理的数据大小超过了接收缓冲区大小，或者目的netlink套接字被设置了拥挤标志，
     * 意味着该sbk不能立即被目的netlink套接字接收，需要加入等待队列
     */
	if ((atomic_read(&sk->sk_rmem_alloc) > sk->sk_rcvbuf ||
	     test_bit(NETLINK_CONGESTED, &nlk->state)) &&
	    !netlink_skb_is_mmaped(skb)) {
        // 申请一个等待队列
		DECLARE_WAITQUEUE(wait, current);
        // 如果传入的超时时间为0,意味着非阻塞调用，则丢弃这条netlink消息，并返回EAGAIN
		if (!*timeo) {
            /* 如果该netlink消息对应的源sock结构不存在，或者该netlink消息来自kernel，
             * 则对目的netlink套接字设置缓冲区溢出状态
             */
			if (!ssk || netlink_is_kernel(ssk))
				netlink_overrun(sk);
			sock_put(sk);
			kfree_skb(skb);
			return -EAGAIN;
		}

        // 程序运行到这里意味着是阻塞调用
        // 改变当前进程状态为可中断
		__set_current_state(TASK_INTERRUPTIBLE);
        // 将目的netlink套接字加入等待队列（同时意味着进入睡眠，猜测）
		add_wait_queue(&nlk->wait, &wait);

        // 程序到这里意味着被唤醒了(猜测)
        // 如果接收条件还是不满足，则要计算剩余的超时时间
		if ((atomic_read(&sk->sk_rmem_alloc) > sk->sk_rcvbuf ||
		     test_bit(NETLINK_CONGESTED, &nlk->state)) &&
		    !sock_flag(sk, SOCK_DEAD))
			*timeo = schedule_timeout(*timeo);

        // 改变当前进程状态为运行
		__set_current_state(TASK_RUNNING);
        // 将目的netlink套接字从等待队列中删除
		remove_wait_queue(&nlk->wait, &wait);
		sock_put(sk);

		if (signal_pending(current)) {
			kfree_skb(skb);
			return sock_intr_errno(*timeo);
		}
        // 返回1,意味着还将要再次调用本函数
		return 1;
	}
    
    // 设置该skb的所有者是目的用户进程netlink套接字，这里才是真正的绑定操作，上面都是前奏
	netlink_skb_set_owner_r(skb, sk);
	return 0;
}

/* 将指定skb发送到指定用户进程netlink套接字，换句话说，实际也就是将该skb放入了该套接字的接收队列中
 * @sk  - 指向一个用户进程netlink套接字
 * @skb - 指向一个承载了netlink消息的skb
 *
 * 备注：该skb调用本函数前已经绑定到该用户进程netlink套接字
 *       之所以说组播消息不支持发往内核的根本原因就在本函数中：
 *              本函数最后通过调用sk_data_ready钩子函数来通知所在套接字接收组播消息，
 *              而内核netlink套接字实际屏蔽了这个钩子函数，也就意味着永远无法接收到组播消息
 */
static int __netlink_sendskb(struct sock *sk, struct sk_buff *skb)
{
    // 这里可以知道，成功时的返回值就是skb中承载的netlink消息长度 
	int len = skb->len;

	netlink_deliver_tap(skb);

#ifdef CONFIG_NETLINK_MMAP
	if (netlink_skb_is_mmaped(skb))
		netlink_queue_mmaped_skb(sk, skb);
	else if (netlink_rx_is_mmaped(sk))
		netlink_ring_set_copied(sk, skb);
	else
#endif /* CONFIG_NETLINK_MMAP */
        // 将skb放入该netlink套接字接收队列末尾
		skb_queue_tail(&sk->sk_receive_queue, skb);
    // 执行该netlink套接字的sk_data_ready回调，用于通知其有数据接收到
	sk->sk_data_ready(sk, len);
	return len;
}

/* 将指定skb发送到目的用户进程netlink套接字
 * @sk  - 目的netlink套接字
 * @skb - 指向一个承载了netlink消息的skb
 *
 * 备注：该skb调用本函数前已经绑定到该用户进程netlink套接字
 */
int netlink_sendskb(struct sock *sk, struct sk_buff *skb)
{
	int len = __netlink_sendskb(sk, skb);

	sock_put(sk);
	return len;
}

void netlink_detachskb(struct sock *sk, struct sk_buff *skb)
{
	kfree_skb(skb);
	sock_put(sk);
}

// 重新调整skb数据区大小
static struct sk_buff *netlink_trim(struct sk_buff *skb, gfp_t allocation)
{
	int delta;

	WARN_ON(skb->sk != NULL);
	if (netlink_skb_is_mmaped(skb))
		return skb;

	delta = skb->end - skb->tail;
    // 如果原本skb的数据区是在vmalloc中，或者数据区的剩余部分很小，就不做处理直接返回了
	if (is_vmalloc_addr(skb->head) || delta * 2 < skb->truesize)
		return skb;

    // 如果该skb的数据包缓冲区被其他skb共用，这里就要clone出一个新的skb结构
	if (skb_shared(skb)) {
		struct sk_buff *nskb = skb_clone(skb, allocation);
		if (!nskb)
			return skb;
		consume_skb(skb);
		skb = nskb;
	}

    // 重新调整该skb的数据包缓冲区的tailroom大小
	if (!pskb_expand_head(skb, 0, -delta, allocation))
		skb->truesize -= delta;

	return skb;
}

/* 将单播netlink消息发往内核
 *
 * @sk  - 目的sock结构
 * @skb - 属于发送方的承载了netlink消息的skb
 * @ssk - 源sock结构
 *
 * 备注：本函数只会在一种情况下被调用到：
 *                  用户进程   --单播--> kernel 
 *       skb的所有者在本函数中发生了变化
 */
static int netlink_unicast_kernel(struct sock *sk, struct sk_buff *skb,
				  struct sock *ssk)
{
	int ret;
    // 获取目的netlink套接字，也就是内核netlink套接字
	struct netlink_sock *nlk = nlk_sk(sk);

	ret = -ECONNREFUSED;
    // 检查内核netlink套接字是否注册了netlink_rcv回调(就是各个协议在创建内核netlink套接字时通常会传入的input函数)
	if (nlk->netlink_rcv != NULL) {
		ret = skb->len;
        // 设置该skb的所有者是内核的netlink套接字
		netlink_skb_set_owner_r(skb, sk);
        // 保存该netlink消息的源sock结构
		NETLINK_CB(skb).sk = ssk;
        // netlink tap机制暂略
		netlink_deliver_tap_kernel(sk, ssk, skb);
        // 调用内核netlink套接字的协议类型相关的netlink_rcv钩子来处理收到的消息
        // 到这里，意味着发往内核的netlink消息已经被成功接收
		nlk->netlink_rcv(skb);
		consume_skb(skb);
	} else {
        // 如果指定的内核netlink套接字没有注册netlink_rcv回调，就直接丢弃所有收到的netlink消息
		kfree_skb(skb);
	}
	sock_put(sk);
	return ret;
}

/* 发送netlink单播消息
 *
 * @ssk         - 源sock结构
 * @skb         - 属于发送方的承载了netlink消息的skb
 * @portid      - 目的单播地址
 * @nonblock    - 1：非阻塞调用，2：阻塞调用
 *
 * 备注: 以下3种情况都会调用到本函数：
 *          [1]. kernel     --单播--> 用户进程
 *          [2]. 用户进程   --单播--> kernel
 *          [3]. 用户进程a  --单播--> 用户进程b
 */
int netlink_unicast(struct sock *ssk, struct sk_buff *skb,
		    u32 portid, int nonblock)
{
	struct sock *sk;
	int err;
	long timeo;

    // 重新调整skb数据区大小
	skb = netlink_trim(skb, gfp_any());

    // 计算发送超时时间
	timeo = sock_sndtimeo(ssk, nonblock);
retry:
    // 根据源sock结构和目的单播地址，得到目的sock结构
	sk = netlink_getsockbyportid(ssk, portid);
	if (IS_ERR(sk)) {
		kfree_skb(skb);
		return PTR_ERR(sk);
	}

    // 如果该目的netlink套接字属于内核，则进入第 [2] 种情况下的分支
	if (netlink_is_kernel(sk))
		return netlink_unicast_kernel(sk, skb, ssk);

    // 程序运行到这里，意味着以下属于第 [1]/[3] 种情况下的分支
    // 对于发往用户进程的单播消息都要调用BPF过滤
	if (sk_filter(sk, skb)) {
		err = skb->len;
		kfree_skb(skb);
		sock_put(sk);
		return err;
	}

    // 将该skb绑定到用户进程netlink套接字上，如果成功，skb的所有者也从这里开始变更为目的用户进程netlink套接字
	err = netlink_attachskb(sk, skb, &timeo, ssk);
    // 如果返回值是1，意味着要重新尝试绑定操作
	if (err == 1)
		goto retry;
	if (err)
		return err;

    // 将该skb发送到目的用户进程netlink套接字
	return netlink_sendskb(sk, skb);
}
EXPORT_SYMBOL(netlink_unicast);

/* 分配一个用于承载netlink消息的skb结构
 *
 * @ssk     - netlink消息的源sock结构
 * @size    - 将要创建的skb数据缓冲区大小
 * @dst_portid  - netlink消息的目的单播地址
 *
 * 备注：跟netlink_alloc_large_skb的区别在于本函数用于小尺寸skb数据缓冲区(猜测)
 */
struct sk_buff *netlink_alloc_skb(struct sock *ssk, unsigned int size,
				  u32 dst_portid, gfp_t gfp_mask)
{
#ifdef CONFIG_NETLINK_MMAP
	struct sock *sk = NULL;
	struct sk_buff *skb;
	struct netlink_ring *ring;
	struct nl_mmap_hdr *hdr;
	unsigned int maxlen;

    // 根据源sock结构和目的单播地址，得到目的sock结构
	sk = netlink_getsockbyportid(ssk, dst_portid);
	if (IS_ERR(sk))
		goto out;

	ring = &nlk_sk(sk)->rx_ring;
	/* fast-path without atomic ops for common case: non-mmaped receiver */
	if (ring->pg_vec == NULL)
		goto out_put;

	if (ring->frame_size - NL_MMAP_HDRLEN < size)
		goto out_put;

	skb = alloc_skb_head(gfp_mask);
	if (skb == NULL)
		goto err1;

	spin_lock_bh(&sk->sk_receive_queue.lock);
	/* check again under lock */
	if (ring->pg_vec == NULL)
		goto out_free;

	/* check again under lock */
	maxlen = ring->frame_size - NL_MMAP_HDRLEN;
	if (maxlen < size)
		goto out_free;

	netlink_forward_ring(ring);
	hdr = netlink_current_frame(ring, NL_MMAP_STATUS_UNUSED);
	if (hdr == NULL)
		goto err2;
	netlink_ring_setup_skb(skb, sk, ring, hdr);
	netlink_set_status(hdr, NL_MMAP_STATUS_RESERVED);
	atomic_inc(&ring->pending);
	netlink_increment_head(ring);

	spin_unlock_bh(&sk->sk_receive_queue.lock);
	return skb;

err2:
	kfree_skb(skb);
	spin_unlock_bh(&sk->sk_receive_queue.lock);
	netlink_overrun(sk);
err1:
	sock_put(sk);
	return NULL;

out_free:
	kfree_skb(skb);
	spin_unlock_bh(&sk->sk_receive_queue.lock);
out_put:
	sock_put(sk);
out:
#endif
	return alloc_skb(size, gfp_mask);
}
EXPORT_SYMBOL_GPL(netlink_alloc_skb);

int netlink_has_listeners(struct sock *sk, unsigned int group)
{
	int res = 0;
	struct listeners *listeners;

	BUG_ON(!netlink_is_kernel(sk));

	rcu_read_lock();
	listeners = rcu_dereference(nl_table[sk->sk_protocol].listeners);

	if (listeners && group - 1 < nl_table[sk->sk_protocol].groups)
		res = test_bit(group - 1, listeners->masks);

	rcu_read_unlock();

	return res;
}
EXPORT_SYMBOL_GPL(netlink_has_listeners);

/* 将携带了netlink组播消息的skb发送到指定目的用户进程netlink套接字
 * @返回值      -1  :套接字接收条件不满足
 *              0   :netlink组播消息发送成功，套接字已经接收但尚未处理数据长度小于等于其接收缓冲的1/2 
 *              1   :netlink组播消息发送成功，套接字已经接收但尚未处理数据长度大于其接收缓冲的1/2(这种情况似乎意味着套接字处于拥挤状态)
 *
 * 备注：到本函数这里，已经确定了传入的netlink套接字跟组播消息匹配正确；
 *       netlink组播消息不支持阻塞
 */
static int netlink_broadcast_deliver(struct sock *sk, struct sk_buff *skb)
{
	struct netlink_sock *nlk = nlk_sk(sk);

    // 判断该netlink套接字是否满足接收条件
	if (atomic_read(&sk->sk_rmem_alloc) <= sk->sk_rcvbuf &&
	    !test_bit(NETLINK_CONGESTED, &nlk->state)) {
        // 如果满足接收条件，则设置skb的所有者是该netlink套接字
		netlink_skb_set_owner_r(skb, sk);
        // 将skb发送到该netlink套接字,实际也就是将该skb放入了该套接字的接收队列中
		__netlink_sendskb(sk, skb);
        
        // 这里最后判断该netlink套接字的已经接收尚未处理数据长度是否大于其接收缓冲的1/2
		return atomic_read(&sk->sk_rmem_alloc) > (sk->sk_rcvbuf >> 1);
	}

    // 程序运行到这里意味着接收条件不满足
	return -1;
}

// 定义了一条netlink组播消息的管理块
struct netlink_broadcast_data {
	struct sock *exclude_sk;    // 指向消息的源sock
	struct net *net;            // 指向消息所在的net命名空间
	u32 portid;                 // 记录了消息的目的单播地址
	u32 group;                  // 记录了消息的目的组播地址
	int failure;                // 传送失败置1
	int delivery_failure;       // 传送失败置1
	int congested;              // 标示了目的套接字是否处于拥挤状态
	int delivered;              // 传送成功置1
	gfp_t allocation;
	struct sk_buff *skb, *skb2; // @skb - 指向一个承载了netlink组播消息的skb
	int (*tx_filter)(struct sock *dsk, struct sk_buff *skb, void *data);
	void *tx_data;
};

/* 尝试向指定用户进程netlink套接字发送组播消息
 * @sk  - 指向一个sock结构，对应一个用户进程netlink套接字
 * @p   - 指向一个netlink组播消息的管理块
 *
 * 备注：传入的这个netlink套接字跟组播消息属于同一种netlink协议类型，并且这个套接字开启了组播阅订，除了这些，其他信息(比如阅订了具体哪些组播)都是不确定的
 */
static int do_one_broadcast(struct sock *sk,
				   struct netlink_broadcast_data *p)
{
	struct netlink_sock *nlk = nlk_sk(sk);
	int val;

    // 如果源sock和目的sock是同一个则直接返回
	if (p->exclude_sk == sk)
		goto out;

    // 如果目的单播地址就是该netlink套接字，或者目的组播地址超出了该netlink套接字的上限，或者该netlink套接字没有阅订这条组播消息，都直接返回
	if (nlk->portid == p->portid || p->group - 1 >= nlk->ngroups ||
	    !test_bit(p->group - 1, nlk->groups))
		goto out;

    // 如果两者不属于同一个net命名空间，则直接返回
	if (!net_eq(sock_net(sk), p->net))
		goto out;

    // 如果netlink组播消息的管理块携带了failure标志, 则对该netlink套接字设置缓冲区溢出状态
	if (p->failure) {
		netlink_overrun(sk);
		goto out;
	}

	sock_hold(sk);
    // 设置skb2，其内容来自skb
	if (p->skb2 == NULL) {
		if (skb_shared(p->skb)) {
			p->skb2 = skb_clone(p->skb, p->allocation);
		} else {
			p->skb2 = skb_get(p->skb);
			/*
			 * skb ownership may have been set when
			 * delivered to a previous socket.
			 */
			skb_orphan(p->skb2);
		}
	}

	if (p->skb2 == NULL) {
        // 到这里如果skb2还是NULL，意味着上一步中clone失败
		netlink_overrun(sk);
		/* Clone failed. Notify ALL listeners. */
		p->failure = 1;
		if (nlk->flags & NETLINK_BROADCAST_SEND_ERROR)
			p->delivery_failure = 1;
	} else if (p->tx_filter && p->tx_filter(sk, p->skb2, p->tx_data)) {
        // 如果传入了发送过滤器，但是过滤不通过，释放skb2
		kfree_skb(p->skb2);
		p->skb2 = NULL;
	} else if (sk_filter(sk, p->skb2)) {
        // 如果BPF过滤不通过，释放skb2
		kfree_skb(p->skb2);
		p->skb2 = NULL;
	} else if ((val = netlink_broadcast_deliver(sk, p->skb2)) < 0) {
        // 如果将承载了组播消息的skb发送到该用户进程netlink套接字失败
		netlink_overrun(sk);
		if (nlk->flags & NETLINK_BROADCAST_SEND_ERROR)
			p->delivery_failure = 1;
	} else {
        // 发送成功最终会进入这里
		p->congested |= val;
		p->delivered = 1;
		p->skb2 = NULL;     // 这里没有释放skb2，而是在上层函数进行释放，原因是因为上层遍历时可能要反复进入本函数，所以skb2要被反复用到
	}
	sock_put(sk);

out:
	return 0;
}

/* 发送netlink组播消息
 * @ssk         - 源sock结构
 * @skb         - 属于发送方的承载了netlink消息的skb
 * @portid      - 目的单播地址
 * @group       - 目的组播地址
 * @filter      - 指向一个过滤器
 * @filter_data - 指向一个传递给过滤器的参数
 */
int netlink_broadcast_filtered(struct sock *ssk, struct sk_buff *skb, u32 portid,
	u32 group, gfp_t allocation,
	int (*filter)(struct sock *dsk, struct sk_buff *skb, void *data),
	void *filter_data)
{
    // 获取源sock所在net命名空间
	struct net *net = sock_net(ssk);
	struct netlink_broadcast_data info;
	struct sock *sk;

    // 重新调整skb数据区大小
	skb = netlink_trim(skb, allocation);

	info.exclude_sk = ssk;
	info.net = net;
	info.portid = portid;
	info.group = group;
	info.failure = 0;
	info.delivery_failure = 0;
	info.congested = 0;
	info.delivered = 0;
	info.allocation = allocation;
	info.skb = skb;
	info.skb2 = NULL;
	info.tx_filter = filter;
	info.tx_data = filter_data;

	/* While we sleep in clone, do not allow to change socket list */

	netlink_lock_table();
    
    // 遍历该netlink套接字所在协议类型中所有阅订了组播功能的套接字，然后尝试向其发送该组播消息
	sk_for_each_bound(sk, &nl_table[ssk->sk_protocol].mc_list)
		do_one_broadcast(sk, &info);

    // 至此，netlink组播消息已经发送完毕
    // 释放skb结构
	consume_skb(skb);

	netlink_unlock_table();

    // 发送失败处理
	if (info.delivery_failure) {
		kfree_skb(info.skb2);
		return -ENOBUFS;
	}
    // 释放skb2结构
	consume_skb(info.skb2);

    // 发送成功处理
	if (info.delivered) {
		if (info.congested && (allocation & __GFP_WAIT))
			yield();
		return 0;
	}
	return -ESRCH;
}
EXPORT_SYMBOL(netlink_broadcast_filtered);

/* 发送netlink组播消息(显然这只是个封装)
 * @ssk         - 指向源sock结构
 * @skb         - 属于发送方的承载了netlink消息的skb
 * @portid      - 目的单播地址
 * @group       - 目的组播地址
 *
 * 备注: 以下2种情况都会调用到本函数：
 *          [1]. 用户进程   --组播--> 用户进程
 *          [2]. kernel     --组播--> 用户进程
 */
int netlink_broadcast(struct sock *ssk, struct sk_buff *skb, u32 portid,
		      u32 group, gfp_t allocation)
{
	return netlink_broadcast_filtered(ssk, skb, portid, group, allocation,
		NULL, NULL);
}
EXPORT_SYMBOL(netlink_broadcast);

struct netlink_set_err_data {
	struct sock *exclude_sk;
	u32 portid;
	u32 group;
	int code;
};

static int do_one_set_err(struct sock *sk, struct netlink_set_err_data *p)
{
	struct netlink_sock *nlk = nlk_sk(sk);
	int ret = 0;

	if (sk == p->exclude_sk)
		goto out;

	if (!net_eq(sock_net(sk), sock_net(p->exclude_sk)))
		goto out;

	if (nlk->portid == p->portid || p->group - 1 >= nlk->ngroups ||
	    !test_bit(p->group - 1, nlk->groups))
		goto out;

	if (p->code == ENOBUFS && nlk->flags & NETLINK_RECV_NO_ENOBUFS) {
		ret = 1;
		goto out;
	}

	sk->sk_err = p->code;
	sk->sk_error_report(sk);
out:
	return ret;
}

/**
 * netlink_set_err - report error to broadcast listeners
 * @ssk: the kernel netlink socket, as returned by netlink_kernel_create()
 * @portid: the PORTID of a process that we want to skip (if any)
 * @group: the broadcast group that will notice the error
 * @code: error code, must be negative (as usual in kernelspace)
 *
 * This function returns the number of broadcast listeners that have set the
 * NETLINK_RECV_NO_ENOBUFS socket option.
 */
int netlink_set_err(struct sock *ssk, u32 portid, u32 group, int code)
{
	struct netlink_set_err_data info;
	struct sock *sk;
	int ret = 0;

	info.exclude_sk = ssk;
	info.portid = portid;
	info.group = group;
	/* sk->sk_err wants a positive error value */
	info.code = -code;

	read_lock(&nl_table_lock);

	sk_for_each_bound(sk, &nl_table[ssk->sk_protocol].mc_list)
		ret += do_one_set_err(sk, &info);

	read_unlock(&nl_table_lock);
	return ret;
}
EXPORT_SYMBOL(netlink_set_err);

/* must be called with netlink table grabbed */
static void netlink_update_socket_mc(struct netlink_sock *nlk,
				     unsigned int group,
				     int is_new)
{
	int old, new = !!is_new, subscriptions;

	old = test_bit(group - 1, nlk->groups);
	subscriptions = nlk->subscriptions - old + new;
	if (new)
		__set_bit(group - 1, nlk->groups);
	else
		__clear_bit(group - 1, nlk->groups);
	netlink_update_subscriptions(&nlk->sk, subscriptions);
	netlink_update_listeners(&nlk->sk);
}

static int netlink_setsockopt(struct socket *sock, int level, int optname,
			      char __user *optval, unsigned int optlen)
{
	struct sock *sk = sock->sk;
	struct netlink_sock *nlk = nlk_sk(sk);
	unsigned int val = 0;
	int err;

	if (level != SOL_NETLINK)
		return -ENOPROTOOPT;

	if (optname != NETLINK_RX_RING && optname != NETLINK_TX_RING &&
	    optlen >= sizeof(int) &&
	    get_user(val, (unsigned int __user *)optval))
		return -EFAULT;

	switch (optname) {
	case NETLINK_PKTINFO:
		if (val)
			nlk->flags |= NETLINK_RECV_PKTINFO;
		else
			nlk->flags &= ~NETLINK_RECV_PKTINFO;
		err = 0;
		break;
	case NETLINK_ADD_MEMBERSHIP:
	case NETLINK_DROP_MEMBERSHIP: {
		if (!netlink_allowed(sock, NL_CFG_F_NONROOT_RECV))
			return -EPERM;
		err = netlink_realloc_groups(sk);
		if (err)
			return err;
		if (!val || val - 1 >= nlk->ngroups)
			return -EINVAL;
		netlink_table_grab();
		netlink_update_socket_mc(nlk, val,
					 optname == NETLINK_ADD_MEMBERSHIP);
		netlink_table_ungrab();

		if (nlk->netlink_bind)
			nlk->netlink_bind(val);

		err = 0;
		break;
	}
	case NETLINK_BROADCAST_ERROR:
		if (val)
			nlk->flags |= NETLINK_BROADCAST_SEND_ERROR;
		else
			nlk->flags &= ~NETLINK_BROADCAST_SEND_ERROR;
		err = 0;
		break;
	case NETLINK_NO_ENOBUFS:
		if (val) {
			nlk->flags |= NETLINK_RECV_NO_ENOBUFS;
			clear_bit(NETLINK_CONGESTED, &nlk->state);
			wake_up_interruptible(&nlk->wait);
		} else {
			nlk->flags &= ~NETLINK_RECV_NO_ENOBUFS;
		}
		err = 0;
		break;
#ifdef CONFIG_NETLINK_MMAP
	case NETLINK_RX_RING:
	case NETLINK_TX_RING: {
		struct nl_mmap_req req;

		/* Rings might consume more memory than queue limits, require
		 * CAP_NET_ADMIN.
		 */
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (optlen < sizeof(req))
			return -EINVAL;
		if (copy_from_user(&req, optval, sizeof(req)))
			return -EFAULT;
		err = netlink_set_ring(sk, &req, false,
				       optname == NETLINK_TX_RING);
		break;
	}
#endif /* CONFIG_NETLINK_MMAP */
	default:
		err = -ENOPROTOOPT;
	}
	return err;
}

static int netlink_getsockopt(struct socket *sock, int level, int optname,
			      char __user *optval, int __user *optlen)
{
	struct sock *sk = sock->sk;
	struct netlink_sock *nlk = nlk_sk(sk);
	int len, val, err;

	if (level != SOL_NETLINK)
		return -ENOPROTOOPT;

	if (get_user(len, optlen))
		return -EFAULT;
	if (len < 0)
		return -EINVAL;

	switch (optname) {
	case NETLINK_PKTINFO:
		if (len < sizeof(int))
			return -EINVAL;
		len = sizeof(int);
		val = nlk->flags & NETLINK_RECV_PKTINFO ? 1 : 0;
		if (put_user(len, optlen) ||
		    put_user(val, optval))
			return -EFAULT;
		err = 0;
		break;
	case NETLINK_BROADCAST_ERROR:
		if (len < sizeof(int))
			return -EINVAL;
		len = sizeof(int);
		val = nlk->flags & NETLINK_BROADCAST_SEND_ERROR ? 1 : 0;
		if (put_user(len, optlen) ||
		    put_user(val, optval))
			return -EFAULT;
		err = 0;
		break;
	case NETLINK_NO_ENOBUFS:
		if (len < sizeof(int))
			return -EINVAL;
		len = sizeof(int);
		val = nlk->flags & NETLINK_RECV_NO_ENOBUFS ? 1 : 0;
		if (put_user(len, optlen) ||
		    put_user(val, optval))
			return -EFAULT;
		err = 0;
		break;
	default:
		err = -ENOPROTOOPT;
	}
	return err;
}

static void netlink_cmsg_recv_pktinfo(struct msghdr *msg, struct sk_buff *skb)
{
	struct nl_pktinfo info;

	info.group = NETLINK_CB(skb).dst_group;
	put_cmsg(msg, SOL_NETLINK, NETLINK_PKTINFO, sizeof(info), &info);
}

/* 用户态对netlink接口的套接字执行系统调用sendmsg()时就会进入到这里
 * @sock    - 指向用户进程的netlink套接字，也就是发送方的
 * @msg     - 承载了发送方传递的netlink消息
 * @len     - netlink消息长度
 */
static int netlink_sendmsg(struct kiocb *kiocb, struct socket *sock,
			   struct msghdr *msg, size_t len)
{
	struct sock_iocb *siocb = kiocb_to_siocb(kiocb);
	struct sock *sk = sock->sk;
	struct netlink_sock *nlk = nlk_sk(sk);
	DECLARE_SOCKADDR(struct sockaddr_nl *, addr, msg->msg_name);    // 显然，这里定义的addr指向netlink消息的目的地址
	u32 dst_portid;
	u32 dst_group;
	struct sk_buff *skb;
	int err;
	struct scm_cookie scm;
	u32 netlink_skb_flags = 0;

    // netlink消息不支持传输带外数据
	if (msg->msg_flags&MSG_OOB)
		return -EOPNOTSUPP;

    // 辅助消息处理
	if (NULL == siocb->scm)
		siocb->scm = &scm;

	err = scm_send(sock, msg, siocb->scm, true);
	if (err < 0)
		return err;

	if (msg->msg_namelen) {
		err = -EINVAL;
        // 如果用户进程指定了目的地址，则对其进行合法性检测，然后从中获取单播地址和组播地址
		if (addr->nl_family != AF_NETLINK)
			goto out;
		dst_portid = addr->nl_pid;
		dst_group = ffs(addr->nl_groups);
		err =  -EPERM;
        // 如果有设置目的组播地址，或者目的单播地址不是发往kernel，就需要检查具体的netlink协议是否支持
		if ((dst_group || dst_portid) &&
		    !netlink_allowed(sock, NL_CFG_F_NONROOT_SEND))
			goto out;
		netlink_skb_flags |= NETLINK_SKB_DST;
	} else {
        // 如果用户进程没有指定目的地址，则使用该netlink套接字默认的(缺省都是0,可以通过connect系统调用指定)
		dst_portid = nlk->dst_portid;
		dst_group = nlk->dst_group;
	}

    // 如果该netlink套接字还没有被绑定过，首先执行动态绑定
	if (!nlk->portid) {
		err = netlink_autobind(sock);
		if (err)
			goto out;
	}

    // 以下这部分只有当内核配置了CONFIG_NETLINK_MMAP选项才生效
	if (netlink_tx_is_mmaped(sk) &&
	    msg->msg_iov->iov_base == NULL) {
		err = netlink_mmap_sendmsg(sk, msg, dst_portid, dst_group,
					   siocb);
		goto out;
	}

    // 检查需要发送的数据长度是否合法
	err = -EMSGSIZE;
	if (len > sk->sk_sndbuf - 32)
		goto out;

    // 分配skb结构空间
	err = -ENOBUFS;
	skb = netlink_alloc_large_skb(len, dst_group);
	if (skb == NULL)
		goto out;

    // 初始化这个skb中的cb字段
	NETLINK_CB(skb).portid	= nlk->portid;
	NETLINK_CB(skb).dst_group = dst_group;
	NETLINK_CB(skb).creds	= siocb->scm->creds;
	NETLINK_CB(skb).flags	= netlink_skb_flags;

    // 将数据从msg_iov拷贝到skb中
	err = -EFAULT;
	if (memcpy_fromiovec(skb_put(skb, len), msg->msg_iov, len)) {
		kfree_skb(skb);
		goto out;
	}

    // 执行LSM模块，略过
	err = security_netlink_send(sk, skb);
	if (err) {
		kfree_skb(skb);
		goto out;
	}

    // 至此，netlink消息已经被放入新创建的sbk中，接下来，内核根据单播还是组播消息，执行了不同的发送流程
    // 发送netlink组播消息
	if (dst_group) {
		atomic_inc(&skb->users);
		netlink_broadcast(sk, skb, dst_portid, dst_group, GFP_KERNEL);
	}
    // 发送netlink单播消息
	err = netlink_unicast(sk, skb, dst_portid, msg->msg_flags&MSG_DONTWAIT);

out:
	scm_destroy(siocb->scm);
	return err;
}

/* 用户态对netlink接口的套接字执行系统调用recvmsg()时就会进入到这里
 * @sock    - 指向用户进程的netlink套接字，也就是接收方的
 * @msg     - 用于存放接收到的netlink消息
 * @len     - 用户空间支持的netlink消息接收长度上限
 * @flags   - 跟本次接收操作有关的标志位集合(主要来源于用户空间)
 */
static int netlink_recvmsg(struct kiocb *kiocb, struct socket *sock,
			   struct msghdr *msg, size_t len,
			   int flags)
{
	struct sock_iocb *siocb = kiocb_to_siocb(kiocb);
	struct scm_cookie scm;
	struct sock *sk = sock->sk;
	struct netlink_sock *nlk = nlk_sk(sk);
	int noblock = flags&MSG_DONTWAIT;
	size_t copied;
	struct sk_buff *skb, *data_skb;
	int err, ret;

    // netlink消息不支持接收带外数据
	if (flags&MSG_OOB)
		return -EOPNOTSUPP;

	copied = 0;

    // 从套接字的接收队列中获取一个skb
	skb = skb_recv_datagram(sk, flags, noblock, &err);
	if (skb == NULL)
		goto out;

    // 程序运行到这里意味着已经获取到一个skb
	data_skb = skb;

#ifdef CONFIG_COMPAT_NETLINK_MESSAGES
	if (unlikely(skb_shinfo(skb)->frag_list)) {
		/*
		 * If this skb has a frag_list, then here that means that we
		 * will have to use the frag_list skb's data for compat tasks
		 * and the regular skb's data for normal (non-compat) tasks.
		 *
		 * If we need to send the compat skb, assign it to the
		 * 'data_skb' variable so that it will be used below for data
		 * copying. We keep 'skb' for everything else, including
		 * freeing both later.
		 */
		if (flags & MSG_CMSG_COMPAT)
			data_skb = skb_shinfo(skb)->frag_list;
	}
#endif

    // 如果收到的skb中承载的netlink消息长度大于用户空间接收缓存的最大长度，则设置MSG_TRUNC标志，并将实际接收长度改为接收缓存的长度
	copied = data_skb->len;
	if (len < copied) {
		msg->msg_flags |= MSG_TRUNC;
		copied = len;
	}

	skb_reset_transport_header(data_skb);
	err = skb_copy_datagram_iovec(data_skb, 0, msg->msg_iov, copied);

	if (msg->msg_name) {
		DECLARE_SOCKADDR(struct sockaddr_nl *, addr, msg->msg_name);
		addr->nl_family = AF_NETLINK;
		addr->nl_pad    = 0;
		addr->nl_pid	= NETLINK_CB(skb).portid;
		addr->nl_groups	= netlink_group_mask(NETLINK_CB(skb).dst_group);
		msg->msg_namelen = sizeof(*addr);
	}

	if (nlk->flags & NETLINK_RECV_PKTINFO)
		netlink_cmsg_recv_pktinfo(msg, skb);

	if (NULL == siocb->scm) {
		memset(&scm, 0, sizeof(scm));
		siocb->scm = &scm;
	}
	siocb->scm->creds = *NETLINK_CREDS(skb);
	if (flags & MSG_TRUNC)
		copied = data_skb->len;

	skb_free_datagram(sk, skb);

	if (nlk->cb_running &&
	    atomic_read(&sk->sk_rmem_alloc) <= sk->sk_rcvbuf / 2) {
		ret = netlink_dump(sk);
		if (ret) {
			sk->sk_err = -ret;
			sk->sk_error_report(sk);
		}
	}

	scm_recv(sock, msg, siocb->scm, flags);
out:
	netlink_rcv_wake(sk);
	return err ? : copied;
}

// 从2.6.24版本开始，这个函数似乎是被作废了
static void netlink_data_ready(struct sock *sk, int len)
{
	BUG();
}

/*
 *	We export these functions to other modules. They provide a
 *	complete set of kernel non-blocking support for message
 *	queueing.
 *  内核用于创建一个具体协议(如NETLINK_ROUTE)的netlink套接字,成功返回创建的sock结构
 */
struct sock *
__netlink_kernel_create(struct net *net, int unit, struct module *module,
			struct netlink_kernel_cfg *cfg)
{
	struct socket *sock;
	struct sock *sk;
	struct netlink_sock *nlk;
	struct listeners *listeners = NULL;
	struct mutex *cb_mutex = cfg ? cfg->cb_mutex : NULL;
	unsigned int groups;

	BUG_ON(!nl_table);

	if (unit < 0 || unit >= MAX_LINKS)
		return NULL;

    // 首先是创建并初始化一个socket结构
	if (sock_create_lite(PF_NETLINK, SOCK_DGRAM, unit, &sock))
		return NULL;

	/*
	 * We have to just have a reference on the net from sk, but don't
	 * get_net it. Besides, we cannot get and then put the net here.
	 * So we create one inside init_net and the move it to net.
	 */

    /* 然后是创建并初始化一个sock结构
     *
     * 备注： 这里要注意的是，首先使用了init_net缺省网络命名空间来创建sock结构，然后再转回到当前的网络命名空间
     *        从上面的注释来看，这么做的原因大概是无法对当前的网络命名空间执行get_net()操作(__netlink_create->sk_alloc中有调用到)
     */
	if (__netlink_create(&init_net, sock, cb_mutex, unit) < 0)
		goto out_sock_release_nosk;

	sk = sock->sk;
	sk_change_net(sk, net);

    // 这里可以看出，内核默认支持最少32个组播地址
	if (!cfg || cfg->groups < 32)
		groups = 32;
	else
		groups = cfg->groups;

    // 分配linsters的空间
	listeners = kzalloc(sizeof(*listeners) + NLGRPSZ(groups), GFP_KERNEL);
	if (!listeners)
		goto out_sock_release;

    /* 从2.6.24版本开始，netlink_data_ready这个函数被作废了
     * 意味着内核netlink套接字屏蔽了sk_data_ready回调，
     * 通观整个netlink模块可知，netlink组播消息传递过程中会调用到该回调，
     * 这也就是说，内核netlink套接字不可能会收到组播消息
     */
	sk->sk_data_ready = netlink_data_ready;
    // 如果某个具体的netlink协议配置了消息处理函数，就将其注册到netlink套接字的对应位置
	if (cfg && cfg->input)
		nlk_sk(sk)->netlink_rcv = cfg->input;

    /* 将创建的netlink套接字添加到nl_table对应表项中的hash表中
     * 由于本函数创建的都是内核nelink套接字，所以portid 固定为 0
     */
	if (netlink_insert(sk, net, 0))
		goto out_sock_release;

	nlk = nlk_sk(sk);
	nlk->flags |= NETLINK_KERNEL_SOCKET;    // 标记这是个内核netlink套接字

	netlink_table_grab();

    /* 判断该协议类型是否已经注册过了，如果没有则在这里初始化nl_table对应的表项，如果有就不再初始化该表项了
     *
     * 个人的初步猜测：基于不同的net命名空间，相同协议类型的内核netlink套接字可以对应创建多个
     */
	if (!nl_table[unit].registered) {
		nl_table[unit].groups = groups;
		rcu_assign_pointer(nl_table[unit].listeners, listeners);
		nl_table[unit].cb_mutex = cb_mutex;
		nl_table[unit].module = module;
		if (cfg) {
			nl_table[unit].bind = cfg->bind;
			nl_table[unit].flags = cfg->flags;
			if (cfg->compare)
				nl_table[unit].compare = cfg->compare;
		}
		nl_table[unit].registered = 1;
	} else {
        // 多个相同协议类型的内核netlink套接字为何在这里取消listeners空间?
		kfree(listeners);
		nl_table[unit].registered++;
	}
	netlink_table_ungrab();
	return sk;

out_sock_release:
	kfree(listeners);
	netlink_kernel_release(sk);
	return NULL;

out_sock_release_nosk:
	sock_release(sock);
	return NULL;
}
EXPORT_SYMBOL(__netlink_kernel_create);

// 内核用于注销一个具体协议(如NETLINK_ROUTE)的netlink套接字
void netlink_kernel_release(struct sock *sk)
{
	sk_release_kernel(sk);
}
EXPORT_SYMBOL(netlink_kernel_release);

int __netlink_change_ngroups(struct sock *sk, unsigned int groups)
{
	struct listeners *new, *old;
	struct netlink_table *tbl = &nl_table[sk->sk_protocol];

	if (groups < 32)
		groups = 32;

	if (NLGRPSZ(tbl->groups) < NLGRPSZ(groups)) {
		new = kzalloc(sizeof(*new) + NLGRPSZ(groups), GFP_ATOMIC);
		if (!new)
			return -ENOMEM;
		old = nl_deref_protected(tbl->listeners);
		memcpy(new->masks, old->masks, NLGRPSZ(tbl->groups));
		rcu_assign_pointer(tbl->listeners, new);

		kfree_rcu(old, rcu);
	}
	tbl->groups = groups;

	return 0;
}

/**
 * netlink_change_ngroups - change number of multicast groups
 *
 * This changes the number of multicast groups that are available
 * on a certain netlink family. Note that it is not possible to
 * change the number of groups to below 32. Also note that it does
 * not implicitly call netlink_clear_multicast_users() when the
 * number of groups is reduced.
 *
 * @sk: The kernel netlink socket, as returned by netlink_kernel_create().
 * @groups: The new number of groups.
 */
int netlink_change_ngroups(struct sock *sk, unsigned int groups)
{
	int err;

	netlink_table_grab();
	err = __netlink_change_ngroups(sk, groups);
	netlink_table_ungrab();

	return err;
}

void __netlink_clear_multicast_users(struct sock *ksk, unsigned int group)
{
	struct sock *sk;
	struct netlink_table *tbl = &nl_table[ksk->sk_protocol];

	sk_for_each_bound(sk, &tbl->mc_list)
		netlink_update_socket_mc(nlk_sk(sk), group, 0);
}

struct nlmsghdr *
__nlmsg_put(struct sk_buff *skb, u32 portid, u32 seq, int type, int len, int flags)
{
	struct nlmsghdr *nlh;
	int size = nlmsg_msg_size(len);

	nlh = (struct nlmsghdr*)skb_put(skb, NLMSG_ALIGN(size));
	nlh->nlmsg_type = type;
	nlh->nlmsg_len = size;
	nlh->nlmsg_flags = flags;
	nlh->nlmsg_pid = portid;
	nlh->nlmsg_seq = seq;
	if (!__builtin_constant_p(size) || NLMSG_ALIGN(size) - size != 0)
		memset(nlmsg_data(nlh) + len, 0, NLMSG_ALIGN(size) - size);
	return nlh;
}
EXPORT_SYMBOL(__nlmsg_put);

/*
 * It looks a bit ugly.
 * It would be better to create kernel thread.
 * 实际执行dump操作了，很多操作都重复进行，确实丑陋
 *
 * @sk  - 指向netlink消息的源sock结构
 */

static int netlink_dump(struct sock *sk)
{
	struct netlink_sock *nlk = nlk_sk(sk);
	struct netlink_callback *cb;
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh;
	int len, err = -ENOBUFS;
	int alloc_size;

	mutex_lock(nlk->cb_mutex);
	if (!nlk->cb_running) {
		err = -EINVAL;
		goto errout_skb;
	}

	cb = &nlk->cb;
	alloc_size = max_t(int, cb->min_dump_alloc, NLMSG_GOODSIZE);

	if (!netlink_rx_is_mmaped(sk) &&
	    atomic_read(&sk->sk_rmem_alloc) >= sk->sk_rcvbuf)
		goto errout_skb;

    // 这里创建了一个新的skb，用于存储回复用的netlink消息
	skb = netlink_alloc_skb(sk, alloc_size, nlk->portid, GFP_KERNEL);
	if (!skb)
		goto errout_skb;

    // 设置该携带了netlink消息的skb为最初的源netlink套接字所拥有
	netlink_skb_set_owner_r(skb, sk);

    // 这里终于真正执行了dumpit函数...
	len = cb->dump(skb, cb);

	if (len > 0) {
		mutex_unlock(nlk->cb_mutex);

		if (sk_filter(sk, skb))
			kfree_skb(skb);
		else
			__netlink_sendskb(sk, skb);
		return 0;
	}

	nlh = nlmsg_put_answer(skb, cb, NLMSG_DONE, sizeof(len), NLM_F_MULTI);
	if (!nlh)
		goto errout_skb;

	nl_dump_check_consistent(cb, nlh);

	memcpy(nlmsg_data(nlh), &len, sizeof(len));

	if (sk_filter(sk, skb))
		kfree_skb(skb);
	else
		__netlink_sendskb(sk, skb);

	if (cb->done)
		cb->done(cb);

	nlk->cb_running = false;
	mutex_unlock(nlk->cb_mutex);
	module_put(cb->module);
	consume_skb(cb->skb);
	return 0;

errout_skb:
	mutex_unlock(nlk->cb_mutex);
	kfree_skb(skb);
	return err;
}

/* dump操作前的准备工作
 *
 * @ssk     - 这个sock结构也就是对应了一个netlink套接字
 * @skb     - 存储了netlink消息的skb
 * @nlh     - 一条完整的netlink消息
 * @control - 用于dump操作的控制块，里面主要记录了dump回调函数和dump的数据空间大小
 *
 * 备注：需要注意的是，ssk表示接收到该netlink消息并进行处理的一方
 *       而函数内的sk代表的是发送该netlink消息的一方
 */
int __netlink_dump_start(struct sock *ssk, struct sk_buff *skb,
			 const struct nlmsghdr *nlh,
			 struct netlink_dump_control *control)
{
	struct netlink_callback *cb;
	struct sock *sk;
	struct netlink_sock *nlk;
	int ret;

	/* Memory mapped dump requests need to be copied to avoid looping
	 * on the pending state in netlink_mmap_sendmsg() while the CB hold
	 * a reference to the skb.
	 */
	if (netlink_skb_is_mmaped(skb)) {
		skb = skb_copy(skb, GFP_KERNEL);
		if (skb == NULL)
			return -ENOBUFS;
	} else
		atomic_inc(&skb->users);

    // 根据protocol和portid查找对应的源netlink套接字(其实netlink_skb_parms结构中的sk字段就已经记录了源sock结构，这里没必要多此一举)
	sk = netlink_lookup(sock_net(ssk), ssk->sk_protocol, NETLINK_CB(skb).portid);
	if (sk == NULL) {
		ret = -ECONNREFUSED;
		goto error_free;
	}

    // 注意，以下的所有操作都是基于源netlink套接字！
	nlk = nlk_sk(sk);
	mutex_lock(nlk->cb_mutex);
	/* A dump is in progress... 
     * 每个源netlink套接字同时只能处理一个dump操作
     * */
	if (nlk->cb_running) {
		ret = -EBUSY;
		goto error_unlock;
	}
	/* add reference of module which cb->dump belongs to */
	if (!try_module_get(control->module)) {
		ret = -EPROTONOSUPPORT;
		goto error_unlock;
	}

    // 设置该netlink套接字当前有效的操作集合
	cb = &nlk->cb;
	memset(cb, 0, sizeof(*cb));
	cb->dump = control->dump;
	cb->done = control->done;
	cb->nlh = nlh;
	cb->data = control->data;
	cb->module = control->module;
	cb->min_dump_alloc = control->min_dump_alloc;
	cb->skb = skb;

    // 标记源netlink套接字正处于dump操作中
	nlk->cb_running = true;

	mutex_unlock(nlk->cb_mutex);

    // 执行实际的dump操作
	ret = netlink_dump(sk);
	sock_put(sk);

	if (ret)
		return ret;

	/* We successfully started a dump, by returning -EINTR we
	 * signal not to send ACK even if it was requested.
	 */
	return -EINTR;

error_unlock:
	sock_put(sk);
	mutex_unlock(nlk->cb_mutex);
error_free:
	kfree_skb(skb);
	return ret;
}
EXPORT_SYMBOL(__netlink_dump_start);

void netlink_ack(struct sk_buff *in_skb, struct nlmsghdr *nlh, int err)
{
	struct sk_buff *skb;
	struct nlmsghdr *rep;
	struct nlmsgerr *errmsg;
	size_t payload = sizeof(*errmsg);

	/* error messages get the original request appened */
	if (err)
		payload += nlmsg_len(nlh);

	skb = netlink_alloc_skb(in_skb->sk, nlmsg_total_size(payload),
				NETLINK_CB(in_skb).portid, GFP_KERNEL);
	if (!skb) {
		struct sock *sk;

		sk = netlink_lookup(sock_net(in_skb->sk),
				    in_skb->sk->sk_protocol,
				    NETLINK_CB(in_skb).portid);
		if (sk) {
			sk->sk_err = ENOBUFS;
			sk->sk_error_report(sk);
			sock_put(sk);
		}
		return;
	}

	rep = __nlmsg_put(skb, NETLINK_CB(in_skb).portid, nlh->nlmsg_seq,
			  NLMSG_ERROR, payload, 0);
	errmsg = nlmsg_data(rep);
	errmsg->error = err;
	memcpy(&errmsg->msg, nlh, err ? nlh->nlmsg_len : sizeof(*nlh));
	netlink_unicast(in_skb->sk, skb, NETLINK_CB(in_skb).portid, MSG_DONTWAIT);
}
EXPORT_SYMBOL(netlink_ack);

// 处理收到的netlink消息
int netlink_rcv_skb(struct sk_buff *skb, int (*cb)(struct sk_buff *,
						     struct nlmsghdr *))
{
	struct nlmsghdr *nlh;
	int err;

    // 逐条解析netlink消息
	while (skb->len >= nlmsg_total_size(0)) 
    {
		int msglen;

		nlh = nlmsg_hdr(skb);
		err = 0;

        // 从长度方面对消息进行合法性检测，有问题的直接丢弃
		if (nlh->nlmsg_len < NLMSG_HDRLEN || skb->len < nlh->nlmsg_len)
			return 0;

		/* Only requests are handled by the kernel 
         * kernel只处理带NLM_F_REQUEST标记的消息，不然就直接尝试返回一个errno=0的ack
         * */
		if (!(nlh->nlmsg_flags & NLM_F_REQUEST))
			goto ack;

		/* Skip control messages 
         * kernel不处理netlink控制消息，所以直接尝试返回一个errno=0的ack
         * */
		if (nlh->nlmsg_type < NLMSG_MIN_TYPE)
			goto ack;

        // 基于某个指定的netlink协议对该消息做进一步处理
		err = cb(skb, nlh);
		if (err == -EINTR)
			goto skip;

ack:
        // 设置了NLM_F_ACK标志或者处理过程中出现错误，kernel都会回一个ack给用户空间
		if (nlh->nlmsg_flags & NLM_F_ACK || err)
			netlink_ack(skb, nlh, err);

skip:
        // skb的读指针掠过这条处理完毕的netlink消息
		msglen = NLMSG_ALIGN(nlh->nlmsg_len);
		if (msglen > skb->len)
			msglen = skb->len;
		skb_pull(skb, msglen);
	}

	return 0;
}
EXPORT_SYMBOL(netlink_rcv_skb);

/**
 * nlmsg_notify - send a notification netlink message
 * @sk: netlink socket to use
 * @skb: notification message
 * @portid: destination netlink portid for reports or 0
 * @group: destination multicast group or 0
 * @report: 1 to report back, 0 to disable
 * @flags: allocation flags
 */
int nlmsg_notify(struct sock *sk, struct sk_buff *skb, u32 portid,
		 unsigned int group, int report, gfp_t flags)
{
	int err = 0;

	if (group) {
		int exclude_portid = 0;

		if (report) {
			atomic_inc(&skb->users);
			exclude_portid = portid;
		}

		/* errors reported via destination sk->sk_err, but propagate
		 * delivery errors if NETLINK_BROADCAST_ERROR flag is set */
		err = nlmsg_multicast(sk, skb, exclude_portid, group, flags);
	}

	if (report) {
		int err2;

		err2 = nlmsg_unicast(sk, skb, portid);
		if (!err || err == -ESRCH)
			err = err2;
	}

	return err;
}
EXPORT_SYMBOL(nlmsg_notify);

#ifdef CONFIG_PROC_FS
struct nl_seq_iter {
	struct seq_net_private p;
	int link;
	int hash_idx;
};

static struct sock *netlink_seq_socket_idx(struct seq_file *seq, loff_t pos)
{
	struct nl_seq_iter *iter = seq->private;
	int i, j;
	struct sock *s;
	loff_t off = 0;

	for (i = 0; i < MAX_LINKS; i++) {
		struct nl_portid_hash *hash = &nl_table[i].hash;

		for (j = 0; j <= hash->mask; j++) {
			sk_for_each(s, &hash->table[j]) {
				if (sock_net(s) != seq_file_net(seq))
					continue;
				if (off == pos) {
					iter->link = i;
					iter->hash_idx = j;
					return s;
				}
				++off;
			}
		}
	}
	return NULL;
}

static void *netlink_seq_start(struct seq_file *seq, loff_t *pos)
	__acquires(nl_table_lock)
{
	read_lock(&nl_table_lock);
	return *pos ? netlink_seq_socket_idx(seq, *pos - 1) : SEQ_START_TOKEN;
}

static void *netlink_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	struct sock *s;
	struct nl_seq_iter *iter;
	struct net *net;
	int i, j;

	++*pos;

	if (v == SEQ_START_TOKEN)
		return netlink_seq_socket_idx(seq, 0);

	net = seq_file_net(seq);
	iter = seq->private;
	s = v;
	do {
		s = sk_next(s);
	} while (s && !nl_table[s->sk_protocol].compare(net, s));
	if (s)
		return s;

	i = iter->link;
	j = iter->hash_idx + 1;

	do {
		struct nl_portid_hash *hash = &nl_table[i].hash;

		for (; j <= hash->mask; j++) {
			s = sk_head(&hash->table[j]);

			while (s && !nl_table[s->sk_protocol].compare(net, s))
				s = sk_next(s);
			if (s) {
				iter->link = i;
				iter->hash_idx = j;
				return s;
			}
		}

		j = 0;
	} while (++i < MAX_LINKS);

	return NULL;
}

static void netlink_seq_stop(struct seq_file *seq, void *v)
	__releases(nl_table_lock)
{
	read_unlock(&nl_table_lock);
}


static int netlink_seq_show(struct seq_file *seq, void *v)
{
	if (v == SEQ_START_TOKEN) {
		seq_puts(seq,
			 "sk       Eth Pid    Groups   "
			 "Rmem     Wmem     Dump     Locks     Drops     Inode\n");
	} else {
		struct sock *s = v;
		struct netlink_sock *nlk = nlk_sk(s);

		seq_printf(seq, "%pK %-3d %-6u %08x %-8d %-8d %d %-8d %-8d %-8lu\n",
			   s,
			   s->sk_protocol,
			   nlk->portid,
			   nlk->groups ? (u32)nlk->groups[0] : 0,
			   sk_rmem_alloc_get(s),
			   sk_wmem_alloc_get(s),
			   nlk->cb_running,
			   atomic_read(&s->sk_refcnt),
			   atomic_read(&s->sk_drops),
			   sock_i_ino(s)
			);

	}
	return 0;
}

static const struct seq_operations netlink_seq_ops = {
	.start  = netlink_seq_start,
	.next   = netlink_seq_next,
	.stop   = netlink_seq_stop,
	.show   = netlink_seq_show,
};


static int netlink_seq_open(struct inode *inode, struct file *file)
{
	return seq_open_net(inode, file, &netlink_seq_ops,
				sizeof(struct nl_seq_iter));
}

static const struct file_operations netlink_seq_fops = {
	.owner		= THIS_MODULE,
	.open		= netlink_seq_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release_net,
};

#endif

int netlink_register_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&netlink_chain, nb);
}
EXPORT_SYMBOL(netlink_register_notifier);

int netlink_unregister_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(&netlink_chain, nb);
}
EXPORT_SYMBOL(netlink_unregister_notifier);

// 定义了socket层netlink协议族共用的操作集合
static const struct proto_ops netlink_ops = {
	.family =	PF_NETLINK,
	.owner =	THIS_MODULE,
	.release =	netlink_release,
	.bind =		netlink_bind,
	.connect =	netlink_connect,
	.socketpair =	sock_no_socketpair,
	.accept =	sock_no_accept,
	.getname =	netlink_getname,
	.poll =		netlink_poll,
	.ioctl =	sock_no_ioctl,
	.listen =	sock_no_listen,
	.shutdown =	sock_no_shutdown,
	.setsockopt =	netlink_setsockopt,
	.getsockopt =	netlink_getsockopt,
	.sendmsg =	netlink_sendmsg,
	.recvmsg =	netlink_recvmsg,
	.mmap =		netlink_mmap,
	.sendpage =	sock_no_sendpage,
};

// 定义了一个netlink协议族操作集合(主要包含netlink接口创建函数)
static const struct net_proto_family netlink_family_ops = {
	.family = PF_NETLINK,
	.create = netlink_create,
	.owner	= THIS_MODULE,	/* for consistency 8) */
};

// 具体的netlink功能初始化(实际就是创建proc文件系统下的netlink接口)
static int __net_init netlink_net_init(struct net *net)
{
#ifdef CONFIG_PROC_FS
    // proc文件系统下创建/proc/net/netlink文件(文件属性:普通文件 + 只读)
	if (!proc_create("netlink", 0, net->proc_net, &netlink_seq_fops))
		return -ENOMEM;
#endif
	return 0;
}

// netlink功能注销
static void __net_exit netlink_net_exit(struct net *net)
{
#ifdef CONFIG_PROC_FS
	remove_proc_entry("netlink", net->proc_net);
#endif
}

// 创建并注册NETLINK_USERSOCK协议到nl_table中
static void __init netlink_add_usersock_entry(void)
{
	struct listeners *listeners;
	int groups = 32;

	listeners = kzalloc(sizeof(*listeners) + NLGRPSZ(groups), GFP_KERNEL);
	if (!listeners)
		panic("netlink_add_usersock_entry: Cannot allocate listeners\n");

	netlink_table_grab();

	nl_table[NETLINK_USERSOCK].groups = groups;
	rcu_assign_pointer(nl_table[NETLINK_USERSOCK].listeners, listeners);
	nl_table[NETLINK_USERSOCK].module = THIS_MODULE;
	nl_table[NETLINK_USERSOCK].registered = 1;
	nl_table[NETLINK_USERSOCK].flags = NL_CFG_F_NONROOT_SEND;

	netlink_table_ungrab();
}

// 定义了netlink模块在网络命名空间层面的操作集合
static struct pernet_operations __net_initdata netlink_net_ops = {
	.init = netlink_net_init,
	.exit = netlink_net_exit,
};

/* 内核netlink功能模块初始化
 * 备注：内核在这里默认创建了两种协议类型的netlink：NETLINK_USERSOCK和NETLINK_ROUTE
 */
static int __init netlink_proto_init(void)
{
	int i;
	unsigned long limit;
	unsigned int order;
    // 向内核的proto_list链表注册所有netlink协议共有的proto结构，至此所有从socket层下来的netlink消息都可以通过该接口到达相应的传输层
	int err = proto_register(&netlink_proto, 0);

	if (err != 0)
		goto out;

    // 确保netlink参数控制块大小不超过通用的socket buffer预留的控制块大小
	BUILD_BUG_ON(sizeof(struct netlink_skb_parms) > FIELD_SIZEOF(struct sk_buff, cb));

    // 创建了一张MAX_LINKS长度的nl_table表，这张表是整个netlink功能模块的核心，每种协议类型占一个表项
	nl_table = kcalloc(MAX_LINKS, sizeof(*nl_table), GFP_KERNEL);
	if (!nl_table)
		goto panic;

	if (totalram_pages >= (128 * 1024))
		limit = totalram_pages >> (21 - PAGE_SHIFT);
	else
		limit = totalram_pages >> (23 - PAGE_SHIFT);

	order = get_bitmask_order(limit) - 1 + PAGE_SHIFT;
	limit = (1UL << order) / sizeof(struct hlist_head);
	order = get_bitmask_order(min(limit, (unsigned long)UINT_MAX)) - 1;

    // 给nl_table表每个表项申请一张hash表并且注册缺省的net命名空间比较函数
	for (i = 0; i < MAX_LINKS; i++) 
    {
		struct nl_portid_hash *hash = &nl_table[i].hash;

        // 每张hash表默认都会创建1个初始bucket，实际也就是存放一个链表头指针
		hash->table = nl_portid_hash_zalloc(1 * sizeof(*hash->table));
		if (!hash->table) {
			while (i-- > 0)
				nl_portid_hash_free(nl_table[i].hash.table,
						 1 * sizeof(*hash->table));
			kfree(nl_table);
			goto panic;
		}
		hash->max_shift = order;
		hash->shift = 0;
		hash->mask = 0;
		hash->rehash_time = jiffies;

		nl_table[i].compare = netlink_compare;
	}

    // 初始化netlink_tap_all链表头
	INIT_LIST_HEAD(&netlink_tap_all);

    // 创建并注册NETLINK_USERSOCK协议到nl_table中
	netlink_add_usersock_entry();

    /* 注册netlink协议族操作集合到内核中(主要包含netlink接口创建函数)
     * 后续，应用层创建netlink类型的socket时就会调用这里注册的create函数
     */
	sock_register(&netlink_family_ops);
    /* 将netlink模块注册到每一个网络命名空间，并且执行了netlink_net_init
     * 需要注意的是，由于netlink_net_ops.size = 0，意味着netlink模块没有私有空间
     */
	register_pernet_subsys(&netlink_net_ops);
	/* The netlink device handler may be needed early. */
    // 创建并注册NETLINK_ROUTE协议到nl_table中
	rtnetlink_init();

out:
	return err;
panic:
	panic("netlink_init: Cannot allocate nl_table\n");
}

// 将netlink_proto_init函数放入".initcall1.init" section
// init/main.c 中的do_initcalls函数调用
core_initcall(netlink_proto_init);
