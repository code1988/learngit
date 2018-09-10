#ifndef _AVL_H
#define _AVL_H


typedef struct avltree_node {
    int key;
    int height;
    struct avltree_node *left;
    struct avltree_node *right;
}avltree_node_t;

int avltree_height(avltree_node_t *node);
avltree_node_t *avltree_maximum(avltree_node_t *node);
avltree_node_t *avltree_minimum(avltree_node_t *node);
avltree_node_t *avltree_create_node(int key,avltree_node_t *left,avltree_node_t *right);
avltree_node_t *avltree_ll_rotation(avltree_node_t *old_root);
avltree_node_t *avltree_rr_rotation(avltree_node_t *old_root);
avltree_node_t *avltree_lr_rotation(avltree_node_t *root);
avltree_node_t *avltree_rl_rotation(avltree_node_t *root);
avltree_node_t *avltree_insert(avltree_node_t *root,int key);
avltree_node_t *avltree_delete(avltree_node_t *root,int key);
avltree_node_t *avltree_delete_node(avltree_node_t *root,avltree_node_t *delete);
void avltree_destroy(avltree_node_t *root);
avltree_node_t *avltree_find(avltree_node_t *tree,int key);
void avltree_preorder(avltree_node_t *tree);
void avltree_inorder(avltree_node_t *tree);
void avltree_postorder(avltree_node_t *tree);

#endif
