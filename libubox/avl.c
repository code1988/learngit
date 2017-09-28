/*
 * PacketBB handler library (see RFC 5444)
 * Copyright (c) 2010 Henning Rogge <hrogge@googlemail.com>
 * Original OLSRd implementation by Hannes Gredler <hannes@gredler.at>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org/git for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

#include "avl.h"
#include "list.h"

/**
 * internal type save inline function to calculate the maximum of
 * to integers without macro implementation.
 *
 * @param x first parameter of maximum function
 * @param y second parameter of maximum function
 * @return largest integer of both parameters
 */
static inline int avl_max(int x, int y) {
  return x > y ? x : y;
}

/**
 * internal type save inline function to calculate the minimum of
 * to integers without macro implementation.
 *
 * @param x first parameter of minimum function
 * @param y second parameter of minimum function
 * @return smallest integer of both parameters
 */
static inline int avl_min(int x, int y) {
  return x < y ? x : y;
}

static struct avl_node *
avl_find_rec(struct avl_node *node, const void *key, avl_tree_comp comp, void *ptr, int *cmp_result);
static void avl_insert_before(struct avl_tree *tree, struct avl_node *pos_node, struct avl_node *node);
static void avl_insert_after(struct avl_tree *tree, struct avl_node *pos_node, struct avl_node *node);
static void post_insert(struct avl_tree *tree, struct avl_node *node);
static void avl_delete_worker(struct avl_tree *tree, struct avl_node *node);
static void avl_remove(struct avl_tree *tree, struct avl_node *node);

/**
 * Initialize a new avl_tree struct
 * 初始化一棵新的avl树
 * @param tree pointer to avl-tree
 * @param comp pointer to comparator for the tree
 * @param allow_dups true if the tree allows multiple
 *   elements with the same
 * @param ptr custom parameter for comparator
 */
void avl_init(struct avl_tree *tree, avl_tree_comp comp, bool allow_dups, void *ptr)
{
  INIT_LIST_HEAD(&tree->list_head);
  tree->root = NULL;
  tree->count = 0;
  tree->comp = comp;
  tree->allow_dups = allow_dups;
  tree->cmp_ptr = ptr;
}

// 基于链表返回指定avl树节点的下一个节点
static inline struct avl_node *avl_next(struct avl_node *node)
{
    return list_entry(node->list.next, struct avl_node, list);
}

/**
 * Finds a node in an avl-tree with a certain key
 * 根据指定key值从avl树根节点开始查找对应的节点
 * @param tree pointer to avl-tree      指向要查找的avl树
 * @param key pointer to key            指向要被查找的key
 * @return pointer to avl-node with key, NULL if no node with
 *    this key exists.
 */
struct avl_node *
avl_find(const struct avl_tree *tree, const void *key)
{
  struct avl_node *node;
  int diff;

  // 如果是一颗空avl树直接返回
  if (tree->root == NULL)
    return NULL;

  // 从根节点开始查找
  node = avl_find_rec(tree->root, key, tree->comp, tree->cmp_ptr, &diff);

  // 找到则返回对应的节点，找不到则返回NULL
  return diff == 0 ? node : NULL;
}

/**
 * Finds the last node in an avl-tree with a key less or equal
 * than the specified key
 * @param tree pointer to avl-tree
 * @param key pointer to specified key
 * @return pointer to avl-node, NULL if no node with
 *    key less or equal specified key exists.
 */
struct avl_node *
avl_find_lessequal(const struct avl_tree *tree, const void *key) {
  struct avl_node *node, *next;
  int diff;

  // 如果是一颗空avl树直接返回
  if (tree->root == NULL)
    return NULL;

  // 从根节点开始查找
  node = avl_find_rec(tree->root, key, tree->comp, tree->cmp_ptr, &diff);

  /* go left as long as key<node.key */
  while (diff < 0) {
    if (list_is_first(&node->list, &tree->list_head)) {
      return NULL;
    }

    node = (struct avl_node *)node->list.prev;
    diff = (*tree->comp) (key, node->key, tree->cmp_ptr);
  }

  /* go right as long as key>=next_node.key */
  next = node;
  while (diff >= 0) {
    node = next;
    if (list_is_last(&node->list, &tree->list_head)) {
      break;
    }

    next = (struct avl_node *)node->list.next;
    diff = (*tree->comp) (key, next->key, tree->cmp_ptr);
  }
  return node;
}

/**
 * Finds the first node in an avl-tree with a key greater or equal
 * than the specified key
 * @param tree pointer to avl-tree
 * @param key pointer to specified key
 * @return pointer to avl-node, NULL if no node with
 *    key greater or equal specified key exists.
 */
struct avl_node *
avl_find_greaterequal(const struct avl_tree *tree, const void *key) {
  struct avl_node *node, *next;
  int diff;

  if (tree->root == NULL)
    return NULL;

  node = avl_find_rec(tree->root, key, tree->comp, tree->cmp_ptr, &diff);

  /* go right as long as key>node.key */
  while (diff > 0) {
    if (list_is_last(&node->list, &tree->list_head)) {
      return NULL;
    }

    node = (struct avl_node *)node->list.next;
    diff = (*tree->comp) (key, node->key, tree->cmp_ptr);
  }

  /* go left as long as key<=next_node.key */
  next = node;
  while (diff <= 0) {
    node = next;
    if (list_is_first(&node->list, &tree->list_head)) {
      break;
    }

    next = (struct avl_node *)node->list.prev;
    diff = (*tree->comp) (key, next->key, tree->cmp_ptr);
  }
  return node;
}

/**
 * Inserts an avl_node into a tree
 * 将指定的avl树节点插入avl树中
 * @param tree pointer to tree  指向要插入的avl树
 * @param new pointer to node   指向要被插入的avl树节点
 * @return 0 if node was inserted successfully, -1 if it was not inserted
 *   because of a key collision
 */
int
avl_insert(struct avl_tree *tree, struct avl_node *new)
{
  struct avl_node *node, *next, *last;
  int diff;

  new->parent = NULL;

  new->left = NULL;
  new->right = NULL;

  new->balance = 0;     // 每个节点在插入前因为不存在子节点所以都是平衡的
  new->leader = true;   // 每个节点在插入前都认为自己是领袖节点

  // 如果还是一颗空avl树，意味着要插入的是第一个节点
  if (tree->root == NULL) {
    list_add(&new->list, &tree->list_head);
    tree->root = new;   // 第一个节点也就作为根节点
    tree->count = 1;
    return 0;
  }

  // 程序运行到这里意味着已经不是一颗空avl树
  node = avl_find_rec(tree->root, new->key, tree->comp, tree->cmp_ptr, &diff);

  last = node;

  /* 从avl_find_rec返回的节点往后遍历链表，直到到达链表尾节点或者下一个节点是领袖节点时跳出遍历
   * last最终指向跳出遍历时的最后一个节点
   */
  while (!list_is_last(&last->list, &tree->list_head)) {
    next = avl_next(last);
    if (next->leader) {
      break;
    }
    last = next;
  }

  // 再次比较avl_find_rec返回的节点和要插入的节点(这步似乎是多余的)
  diff = (*tree->comp) (new->key, node->key, tree->cmp_ptr);

  // 如果相等意味着要插入的节点的key已经存在，也就是发生了冲突
  if (diff == 0) {
    // 如果该avl树不允许存在超过1个相同的key，则返回-1,意味着插入失败
    if (!tree->allow_dups)
      return -1;

    // 如果允许，则新插入的节点必然不是领袖节点
    new->leader = 0;

    // 该相同key的节点实际不会插入avl树中，而是只会插入链表，位置就是last节点的后一个节点
    avl_insert_after(tree, last, new);
    return 0;
  }

  // 程序运行到这里意味着该avl树中不存在跟要插入节点key值相同的节点
  // 那么以下4个插入操作的分支流程中，avl_find_rec返回节点都将作为新节点的父节点
  
  /* 以下是父节点右平衡状态下的左子节点插入流程
   *                父
   *               /  \
   *                  右子
   * 当avl树中不存在指定key值的节点时，父节点处于右平衡意味着：
   *                返回的diff值必定同时为-1
   *                指定key值必定 < 父节点的key值 
   */
  if (node->balance == 1) {
    // 将该节点插入链表，位置就是父节点的前一个节点
    avl_insert_before(tree, node, new);

    // 将新节点插入到父节点的左子节点，就能直接恢复整棵avl树平衡
    node->balance = 0;
    new->parent = node;
    node->left = new;
    return 0;
  }

  /* 以下是父节点左平衡状态下的右子节点插入流程
   *                父
   *               /  \
   *            左子
   * 当avl树中不存在指定key值的节点时，父节点处于左平衡意味着：
   *                返回的diff值必定同时为1
   *                指定key值必定 > 父节点的key值 
   */
  if (node->balance == -1) {
    // 将该节点插入链表，位置就是last节点的后一个节点
    avl_insert_after(tree, last, new);

    // 将新节点插入到父节点的右子节点，就能直接恢复整棵avl树平衡
    node->balance = 0;
    new->parent = node;
    node->right = new;
    return 0;
  }

  // 程序运行到这里意味着父节点是一个没有子节点的末梢节点，显然它是全平衡的
  // 往末梢节点插入新节点后都需要往上进行自平衡
  
  /* 以下是末梢节点的左子节点插入流程
   *                父
   *               /  
   */
  if (diff < 0) {
    // 将该节点插入链表，位置就是末梢节点的前一个节点
    avl_insert_before(tree, node, new);

    // 将新节点插入到末梢节点的左子节点，意味着末梢节点进入左平衡状态，需要往上继续进行自平衡
    node->balance = -1;
    new->parent = node;
    node->left = new;
    post_insert(tree, node);
    return 0;
  }

  /* 以下是末梢节点的右子节点插入流程
   *                父
   *                  \  
   */
  // 将该节点插入链表，位置就是last节点的后一个节点
  avl_insert_after(tree, last, new);

  // 将新节点插入到末梢节点的右子节点，意味着末梢节点进入右平衡状态，需要往上继续进行自平衡
  node->balance = 1;
  new->parent = node;
  node->right = new;
  post_insert(tree, node);
  return 0;
}

/**
 * Remove a node from an avl tree
 * 从avl树中删除指定的avl树节点
 * @param tree pointer to tree
 * @param node pointer to node
 */
void
avl_delete(struct avl_tree *tree, struct avl_node *node)
{
  struct avl_node *next;
  struct avl_node *parent;
  struct avl_node *left;
  struct avl_node *right;

  // 只有删除一个领袖节点时才会真正去操作avl树结构
  if (node->leader) {
      /* 如果同时满足以下3个条件：
       *        avl树allow_dups功能开启
       *        该领袖节点不是链表尾节点
       *        该领袖节点的下一个节点不是领袖节点
       *
       * 这意味着该领袖节点存在至少1个key相同的副节点，那么删除该领袖节点不会引起avl树的变化
       */
    if (tree->allow_dups
        && !list_is_last(&node->list, &tree->list_head)
        && !(next = avl_next(node))->leader) {
        /* 原领袖节点删除后，下一个副节点将作为新的领袖节点，
         * 同时继承原领袖节点的平衡状态、父节点、左子节点、右子节点
         */
      next->leader = true;
      next->balance = node->balance;

      parent = node->parent;
      left = node->left;
      right = node->right;

      next->parent = parent;
      next->left = left;
      next->right = right;

      // 如果原领袖节点是根节点，还会继承其根节点属性
      if (parent == NULL)
        tree->root = next;

      /* 以这种替换方式删除原领袖节点后，与原领袖节点相连的节点都需要重新连到新的领袖节点
       */
      else {
        if (node == parent->left)
          parent->left = next;

        else
          parent->right = next;
      }

      if (left != NULL)
        left->parent = next;

      if (right != NULL)
        right->parent = next;
    }

    else
        /* 其他情况下的该领袖节点由于不存在key相同的副节点，所以其删除操作会引起avl树的变化
         */
      avl_delete_worker(tree, node);
  }

  // 最后将该节点从链表中删除
  avl_remove(tree, node);
}

/* 从avl树的指定节点开始往下查找指定key对应的节点
 * @node    - 指向当前被查找的节点
 * @key     - 指向要查找的key
 * @comp    - 指向使用的比较器
 * @cmp_ptr - 指向传递给比较器的自定义参数
 * @cmp_result  - 用于存放比较结果:
 *                                  0   - 找到匹配节点
 *                                  -1  - 未找到匹配节点，要查找的key < 返回节点的key
 *                                  1   - 未找到匹配节点，要查找的key > 返回节点的key
 *
 * 备注：这里使用了尾递归机制
 *       当找到匹配节点时，返回该匹配节点，同时cmp_result = 0
 *       当查找到avl树末梢节点都未找到匹配节点时，返回该匹配节点，同时cmp_result存放了跟末梢节点的比较结果
 */
static struct avl_node *
avl_find_rec(struct avl_node *node, const void *key, avl_tree_comp comp, void *cmp_ptr, int *cmp_result)
{
  int diff;

  // 执行比较器
  diff = (*comp) (key, node->key, cmp_ptr);
  *cmp_result = diff;

  // 以下是要查找的key小于当前节点key时的流程
  if (diff < 0) {
    // 如果左子节点存在，则进入左子节点继续递归查找，否则意味着当前节点已经是avl树末梢节点，直接返回该末梢节点
    if (node->left != NULL)
      return avl_find_rec(node->left, key, comp, cmp_ptr, cmp_result);

    return node;
  }

  // 以下是要查找的key大于当前节点key时的流程
  if (diff > 0) {
    // 如果右子节点存在，则进入右子节点继续递归查找，否则意味着当前节点已经是avl树末梢节点，直接返回该末梢节点
    if (node->right != NULL)
      return avl_find_rec(node->right, key, comp, cmp_ptr, cmp_result);

    return node;
  }

  // 程序运行到这里意味着两个key值相等，也就是成功匹配，返回匹配到的节点
  return node;
}

/* 指定节点node(对应图中的节点"5")右旋，右旋后，node节点的左子节点将位于node节点原本的位置:
 *      图1:        5           3       | 图2：     2             2
 *                 /           / \      |            \             \
 *                3     -->   2   5     |             5    -->      3 
 *               /                      |            /               \
 *              2                       |           3                 5
 *
 *  衍生图：        5                  3
 *                 / \                / \
 *                3   6              2   5
 *               / \       -->      /   / \
 *              2   4              1   4   6
 *             /
 *            1
 */
static void
avl_rotate_right(struct avl_tree *tree, struct avl_node *node)
{
  struct avl_node *left, *parent;

  left = node->left;
  parent = node->parent;

  left->parent = parent;
  node->parent = left;

  /* 如果node节点的父节点不存在，意味着node节点是avl树的根节点，右旋后根节点变成left节点
   * 如果node节点的父节点存在，那么右旋后，父节点的相应的子节点变成left节点
   */
  if (parent == NULL)
    tree->root = left;
  else {
    if (parent->left == node)
      parent->left = left;

    else
      parent->right = left;
  }

  /* node节点右旋后，左节点的右子节点将连接node节点
   * 那么如果左节点原本存在右子节点，该右子节点将作为node节点的左子节点
   */
  node->left = left->right;
  left->right = node;

  if (node->left != NULL)
    node->left->parent = node;

  /* node节点右旋后，node节点和left节点应该必然都是右重增加(至少加1)，其中
   * node节点应该会是必然恢复到全平衡;
   * left节点则会是全平衡或者右平衡
   */
  node->balance += 1 - avl_min(left->balance, 0);   // "+1"是因为node节点的左子节点至少去掉了left节点所在的那层
  left->balance += 1 + avl_max(node->balance, 0);   // "+1"是因为left节点的右子节点至少增加了node节点所在的那层
}

/* 指定节点node(对应图中的节点"2")左旋:
 *      图1：       2                   3       | 图2：         5             5
 *                   \                 / \      |              /             /
 *                    3     -->       2   5     |             2    -->      3 
 *                     \                        |              \           /
 *                      5                       |               3         2       
 *
 *  衍生图：        2                   4
 *                 / \                 / \
 *                1   4               2   5
 *                   / \    -->      / \   \
 *                  3   5           1   3   6
 *                       \ 
 *                        6
 */
static void
avl_rotate_left(struct avl_tree *tree, struct avl_node *node)
{
  struct avl_node *right, *parent;

  right = node->right;
  parent = node->parent;

  right->parent = parent;
  node->parent = right;

  if (parent == NULL)
    tree->root = right;

  else {
    if (parent->left == node)
      parent->left = right;

    else
      parent->right = right;
  }

  node->right = right->left;
  right->left = node;

  if (node->right != NULL)
    node->right->parent = node;

  node->balance -= 1 + avl_max(right->balance, 0);
  right->balance -= 1 - avl_min(node->balance, 0);
}

// avl树执行插入自平衡，从平衡发生变化的节点依次往上递归，直到整棵树恢复平衡
static void
post_insert(struct avl_tree *tree, struct avl_node *node)
{
  struct avl_node *parent = node->parent;

  // 当到达根节点时，意味着整棵avl树已经完成自平衡，直接返回
  if (parent == NULL)
    return;

  /* 以下是当前节点为父节点的左子节点时的平衡流程
   * 
   */
  if (node == parent->left) {
    // 插入的节点位于父节点的左子节点下必然导致父节点左重递增
    parent->balance--;           

    // 左重递增后的父节点如果达到全平衡，则自平衡结束
    if (parent->balance == 0)
      return;

    // 左重递增后的父节点如果进入左平衡，则需要从父节点往上继续进行递归平衡
    if (parent->balance == -1) {
      post_insert(tree, parent);
      return;
    }

    // 程序运行到这里意味着左重递增后的父节点左重2，则需要进行旋转操作

    // 如果当前节点左平衡，就是LL失衡，则需要对父节点右旋恢复
    if (node->balance == -1) {
      avl_rotate_right(tree, parent);
      return;
    }

    /* 程序运行到这里意味着父节点左重2,当前节点右平衡，就是LR失衡
     * 则需要先对当前节点左旋，左旋之后的avl树变成LL失衡，然后再对父节点右旋最终恢复平衡
     */
    avl_rotate_left(tree, node);
    avl_rotate_right(tree, node->parent->parent);
    return;
  }

  /* 以下是当前节点为父节点的右子节点时的平衡流程
   * 跟上面的正好相反，略
   */
  parent->balance++;

  if (parent->balance == 0)
    return;

  if (parent->balance == 1) {
    post_insert(tree, parent);
    return;
  }

  if (node->balance == 1) {
    avl_rotate_left(tree, parent);
    return;
  }

  avl_rotate_right(tree, node);
  avl_rotate_left(tree, node->parent->parent);
}

// 将node节点插入到pos_node节点的前一个节点位置，这是一个链表上的操作，但是会更新avl树的节点数量
static void
avl_insert_before(struct avl_tree *tree, struct avl_node *pos_node, struct avl_node *node)
{
  list_add_tail(&node->list, &pos_node->list);
  tree->count++;
}

// 将node节点插入到pos_node节点的后一个节点位置，这是一个链表上的操作，但是会更新avl树的节点数量
static void
avl_insert_after(struct avl_tree *tree, struct avl_node *pos_node, struct avl_node *node)
{
  list_add(&node->list, &pos_node->list);
  tree->count++;
}

// 将node节点从链表中删除，这是一个链表上的操作，但是会更新avl树的节点数量
static void
avl_remove(struct avl_tree *tree, struct avl_node *node)
{
  list_del(&node->list);
  tree->count--;
}

static void
avl_post_delete(struct avl_tree *tree, struct avl_node *node)
{
  struct avl_node *parent;

  if ((parent = node->parent) == NULL)
    return;

  if (node == parent->left) {
    parent->balance++;

    if (parent->balance == 0) {
      avl_post_delete(tree, parent);
      return;
    }

    if (parent->balance == 1)
      return;

    if (parent->right->balance == 0) {
      avl_rotate_left(tree, parent);
      return;
    }

    if (parent->right->balance == 1) {
      avl_rotate_left(tree, parent);
      avl_post_delete(tree, parent->parent);
      return;
    }

    avl_rotate_right(tree, parent->right);
    avl_rotate_left(tree, parent);
    avl_post_delete(tree, parent->parent);
    return;
  }

  parent->balance--;

  if (parent->balance == 0) {
    avl_post_delete(tree, parent);
    return;
  }

  if (parent->balance == -1)
    return;

  if (parent->left->balance == 0) {
    avl_rotate_right(tree, parent);
    return;
  }

  if (parent->left->balance == -1) {
    avl_rotate_right(tree, parent);
    avl_post_delete(tree, parent->parent);
    return;
  }

  avl_rotate_left(tree, parent->left);
  avl_rotate_right(tree, parent);
  avl_post_delete(tree, parent->parent);
}

// 从node节点开始往下遍历，返回一个最小的node
static struct avl_node *
avl_local_min(struct avl_node *node)
{
  while (node->left != NULL)
    node = node->left;

  return node;
}

#if 0
static struct avl_node *
avl_local_max(struct avl_node *node)
{
  while (node->right != NULL)
    node = node->right;

  return node;
}
#endif

// 本函数才是通常意义上的avl树节点删除接口(也就是必然会引发avl树结构的变化)
static void
avl_delete_worker(struct avl_tree *tree, struct avl_node *node)
{
  struct avl_node *parent, *min;

  parent = node->parent;

  // 以下是删除一个没有子节点的末梢节点的流程
  if (node->left == NULL && node->right == NULL) {
    // 如果该末梢节点同时也就是根节点，那么删除后就变成一颗空avl树，直接返回
    if (parent == NULL) {
      tree->root = NULL;
      return;
    }

    /* 以下是当该末梢节点是父节点的左子节点时的删除流程
     * 删除该末梢节点意味着父节右重递增
     */
    if (parent->left == node) {
      parent->left = NULL;
      parent->balance++;

      // 如果右重递增后的父节点处于右平衡，意味着右重递增前父节点处于全平衡，删除操作不会影响avl树的平衡，所以直接返回
      if (parent->balance == 1)
        return;

      // 如果右重递增后的父节点处于全平衡，意味着右重递增前父节点处于左平衡，删除操作可能打破avl树的平衡，需要往上进行avl树自平衡
      if (parent->balance == 0) {
        avl_post_delete(tree, parent);
        return;
      }

      // 程序运行到这里意味着右重递增后的父节点右重2，需要进行旋转操作

      /* 如果父节点的右子节点全平衡，则需要对父节点左旋恢复，旋转后由于层数不变(比如下图中旋转前后都是3层)，直接返回
       *        父
       *       /  \
       *           右子
       *           /  \
       *        左子  右子
       */
      if (parent->right->balance == 0) {
        avl_rotate_left(tree, parent);
        return;
      }

      /* 如果父节点的右子节点右平衡，则需要对父节点左旋，旋转后由于层数变化(比如下图中旋转前3层，旋转后2层)，需要往上进行avl树自平衡
       *        父
       *       /  \
       *           右子
       *              \
       *              右子
       */
      if (parent->right->balance == 1) {
        avl_rotate_left(tree, parent);
        avl_post_delete(tree, parent->parent);
        return;
      }

      /* 如果父节点的右子节点左平衡，则需要对右子节点右旋，在对父节点左旋，旋转后由于层数变化(比如下图中旋转前3层，旋转后2层)，需要往上进行avl树自平衡
       *        父
       *       /  \
       *           右子
       *           /
       *        左子
       */
      avl_rotate_right(tree, parent->right);
      avl_rotate_left(tree, parent);
      avl_post_delete(tree, parent->parent);
      return;
    }

    // 以下是当该末梢节点是父节点的右子节点时的删除流程，基本同上，略
    if (parent->right == node) {
      parent->right = NULL;
      parent->balance--;

      if (parent->balance == -1)
        return;

      if (parent->balance == 0) {
        avl_post_delete(tree, parent);
        return;
      }

      if (parent->left->balance == 0) {
        avl_rotate_right(tree, parent);
        return;
      }

      if (parent->left->balance == -1) {
        avl_rotate_right(tree, parent);
        avl_post_delete(tree, parent->parent);
        return;
      }

      avl_rotate_left(tree, parent->left);
      avl_rotate_right(tree, parent);
      avl_post_delete(tree, parent->parent);
      return;
    }
  }

  // 程序运行到这里意味着要删除一个存在子节点的节点

  // 以下是删除一个只存在右子节点的节点的流程
  if (node->left == NULL) {
    // 当删除一个只存在右子节点的根节点时，右子节点将变成根节点，完后直接返回
    if (parent == NULL) {
      tree->root = node->right;
      node->right->parent = NULL;
      return;
    }

    // 其他情况下，右子节点将会继承删除节点的父节点，同时右子节点将会继承删除节点在父节点中的位置
    node->right->parent = parent;

    if (parent->left == node)
      parent->left = node->right;

    else
      parent->right = node->right;

    // 由于删除节点后层数发生变化，所以需要进行avl树自平衡 
    avl_post_delete(tree, node->right);
    return;
  }

  // 以下是删除一个只存在左子节点的节点的流程，基本同上，略
  if (node->right == NULL) {
    if (parent == NULL) {
      tree->root = node->left;
      node->left->parent = NULL;
      return;
    }

    node->left->parent = parent;

    if (parent->left == node)
      parent->left = node->left;

    else
      parent->right = node->left;

    avl_post_delete(tree, node->left);
    return;
  }

  // 程序运行到这里意味着删除一个左、右子节点都存在的节点

  /* 获取要删除节点的右子节点下(包含右子节点)的最小节点，
   * 然后将该最小节点从avl树中移除，
   * 最后用该最小节点替换掉要删除的节点
   */
  min = avl_local_min(node->right);
  avl_delete_worker(tree, min);
  parent = node->parent;

  min->balance = node->balance;
  min->parent = parent;
  min->left = node->left;
  min->right = node->right;

  if (min->left != NULL)
    min->left->parent = min;

  if (min->right != NULL)
    min->right->parent = min;

  if (parent == NULL) {
    tree->root = min;
    return;
  }

  if (parent->left == node) {
    parent->left = min;
    return;
  }

  parent->right = min;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
