#ifndef AVL_TREE_H
#define AVL_TREE_H

#include "util.h"

typedef struct AVLNode AVLNode;
struct AVLNode {
    void* elem;
    int balance;
    AVLNode* left;
    AVLNode* right;
};

typedef struct {
    AVLNode* root;
    int8_t (*cmp)(const void*, const void*);
} AVLTree;

/// Creates an empty tree that will be ordered by `compare`.
/// No allocation is done at this call.
AVLTree AVLTree_new(int8_t (*compare)(const void*, const void*));

/// Is the tree empty ?
bool AVLTree_is_empty(const AVLTree* avl);

/// Inserts an element in the tree.
/// The `e` pointer is stored as-is.
/// The tree might be rebalanced.
bool AVLTree_insert(AVLTree* avl, void* e);

/// Removes an element from the tree.
/// Returns the removed element or `NULL` if nothing was removed.
/// The tree might be rebalanced.
void* AVLTree_remove(AVLTree* avl, void* e);

/// Clears the tree, removing all elements.
/// Calls `drop_elem` on each removed element.
void AVLTree_clear(AVLTree* avl, void (*drop_elem)(void*));

/// Clears the tree, removing all elements.
void AVLTree_plain_clear(AVLTree* avl);

#endif // AVL_TREE_H
