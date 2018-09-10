#include "utils.h"
#include "avl.h"

#define HEIGHT(node) ((node == NULL)?0:((avltree_node_t *)node)->height)

int avltree_height(avltree_node_t *tree)
{
    return HEIGHT(tree);
}

/* Return maximum node
 */
avltree_node_t *avltree_maximum(avltree_node_t *node)
{
    if (!node)
        return NULL;

    while(node->right)
        node = node->right;

    return node;
}

/* Return minimum node
 */
avltree_node_t *avltree_minimum(avltree_node_t *node)
{
    if (!node)
        return NULL;

    while(node->left)
        node = node->left;

    return node;
}

/* Create avl tree node
 */
avltree_node_t *avltree_create_node(int key,avltree_node_t *left,avltree_node_t *right)
{
    avltree_node_t *node;

    node = calloc(1,sizeof(avltree_node_t));    
    if (!node)
        return NULL;

    node->key = key;
    node->height = 1;
    node->left = left;
    node->right = right;

    return node;
}

/* LL rotation
 */
avltree_node_t *avltree_ll_rotation(avltree_node_t *old_root)
{
    avltree_node_t *new_root;

    new_root = old_root->left;
    old_root->left = new_root->right;
    new_root->right = old_root;

    old_root->height = MAX(HEIGHT(old_root->left),HEIGHT(old_root->right)) + 1;
    new_root->height = MAX(HEIGHT(new_root->left),old_root->height) + 1;

    return new_root;
}

/* RR rotation
 */
avltree_node_t *avltree_rr_rotation(avltree_node_t *old_root)
{
    avltree_node_t *new_root;

    new_root = old_root->right;
    old_root->right = new_root->left;
    new_root->left = old_root;

    old_root->height = MAX(HEIGHT(old_root->left),HEIGHT(old_root->right)) + 1;
    new_root->height = MAX(HEIGHT(new_root->right),old_root->height) + 1;

    return new_root;
}

/* LR rotation
 */
avltree_node_t *avltree_lr_rotation(avltree_node_t *root)
{
    root->left = avltree_rr_rotation(root->left);
    return avltree_ll_rotation(root);
}

/* RL rotation
 */
avltree_node_t *avltree_rl_rotation(avltree_node_t *root)
{
    root->right = avltree_ll_rotation(root->right);
    return avltree_rr_rotation(root);
}

/* Insert key
 * note: make sure the new key has not already existed in avl tree
 */
avltree_node_t *avltree_insert(avltree_node_t *root,int key)
{
    if (!root) {
        root = avltree_create_node(key,NULL,NULL);
        if (!root)
            return NULL;
    } else if (root->key > key) {
        root->left = avltree_insert(root->left,key);
        if (HEIGHT(root->left) - HEIGHT(root->right) == 2) {
            if (root->left->key > key)
                root = avltree_ll_rotation(root);
            else
                root = avltree_lr_rotation(root);
        }
    } else if (root->key < key) {
        root->right = avltree_insert(root->right,key);
        if (HEIGHT(root->right) - HEIGHT(root->left) == 2) {
            if (root->right->key < key)
                root = avltree_rr_rotation(root);
            else
                root = avltree_rl_rotation(root);
        }
    } else
        return NULL;

    root->height = MAX(HEIGHT(root->left),HEIGHT(root->right)) + 1;

    return root;
}

/* Delete node which has a specified key
 */
avltree_node_t *avltree_delete(avltree_node_t *root,int key)
{
    avltree_node_t *delete;

    delete = avltree_find(root,key);
    if (delete)
        root = avltree_delete_node(root,delete);

    return root;
}

/* Delete node
 */
avltree_node_t *avltree_delete_node(avltree_node_t *root,avltree_node_t *delete)
{
    if (!root || !delete)
        return NULL;

    if (root->key > delete->key) {
        root->left = avltree_delete_node(root->left,delete);
        if (HEIGHT(root->right) - HEIGHT(root->left) == 2) {
            if (HEIGHT(root->right->right) > HEIGHT(root->right->left))
                root = avltree_rr_rotation(root);
            else
                root = avltree_rl_rotation(root);
        }
    } else if (root->key < delete->key) {
        root->right = avltree_delete_node(root->right,delete);
        if (HEIGHT(root->left) - HEIGHT(root->right) == 2) {
            if (HEIGHT(root->left->left) > HEIGHT(root->left->right))
                root = avltree_ll_rotation(root);
            else
                root = avltree_lr_rotation(root);
        }
    } else {
        if (root->left && root->right) {
            if (HEIGHT(root->left) > HEIGHT(root->right)) {
                avltree_node_t *max = avltree_maximum(root->left);
                root->key = max->key;
                root->left = avltree_delete_node(root->left,max);
            } else {
                avltree_node_t *min = avltree_minimum(root->right);
                root->key = min->key;
                root->right = avltree_delete_node(root->right,min);
            }
        } else {
            avltree_node_t *tmp = root;
            root = root->left?root->left:root->right;
            free(tmp);
        }
    }

    if (root)
        root->height = MAX(HEIGHT(root->left),HEIGHT(root->right)) + 1;

    return root;
}

/* Destroy a avl tree
 */
void avltree_destroy(avltree_node_t *root)
{
    if (!root)
        return; 

    if (root->left)
        avltree_destroy(root->left);
    if (root->right)
        avltree_destroy(root->right);

    free(root);
}

/* Find the node which has a specified key
 */
avltree_node_t *avltree_find(avltree_node_t *tree,int key)
{
    if (!tree || tree->key == key)
        return tree;

    if (tree->key > key)
        return avltree_find(tree->left,key);
        
    return avltree_find(tree->right,key);
}

void avltree_preorder(avltree_node_t *tree)
{
    if (tree) {
        printf("%d ",tree->key);
        avltree_preorder(tree->left); 
        avltree_preorder(tree->right); 
    }
}

void avltree_inorder(avltree_node_t *tree)
{
    if (tree) {
        avltree_inorder(tree->left); 
        printf("%d ",tree->key);
        avltree_inorder(tree->right); 
    }
}

void avltree_postorder(avltree_node_t *tree)
{
    if (tree) {
        avltree_postorder(tree->left); 
        avltree_postorder(tree->right); 
        printf("%d ",tree->key);
    }
}
