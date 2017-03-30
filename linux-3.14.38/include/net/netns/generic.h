/*
 * generic net pointers
 */

#ifndef __NET_GENERIC_H__
#define __NET_GENERIC_H__

#include <linux/bug.h>
#include <linux/rcupdate.h>

/*
 * Generic net pointers are to be used by modules to put some private
 * stuff on the struct net without explicit struct net modification
 * 通用网络指针管理块，用于管理当前命名空间下各个网络功能模块的私有数据块
 *
 * 备注：这里的实现挺优美，该结构体使用了C的空数组概念，各个私有数据块指针紧密排列在该结构体之后，并通过id索引
 * The rules are simple:
 * 1. set pernet_operations->id.  After register_pernet_device you
 *    will have the id of your private pointer.
 * 2. set pernet_operations->size to have the code allocate and free
 *    a private structure pointed to from struct net.
 * 3. do not change this pointer while the net is alive;
 * 4. do not try to have any private reference on the net_generic object.
 *
 * After accomplishing all of the above, the private pointer can be
 * accessed with the net_generic() call.
 */
struct net_generic {
	unsigned int len;   // 当前ptr数组有效长度
	struct rcu_head rcu;

	void *ptr[0];       // 空指针，仅用于占位，后面紧跟各模块私有数据地址
};

// 根据id号在该网络命名空间下索引对应的私有数据块
static inline void *net_generic(const struct net *net, int id)
{
	struct net_generic *ng;
	void *ptr;

	rcu_read_lock();
	ng = rcu_dereference(net->gen);
	BUG_ON(id == 0 || id > ng->len);
	ptr = ng->ptr[id - 1];
	rcu_read_unlock();

	BUG_ON(!ptr);
	return ptr;
}
#endif
