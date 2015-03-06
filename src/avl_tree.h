#ifndef AVL_TREE_H
#define AVL_TREE_H

#include "util.h"

typedef struct AVLNode AVLNode;
struct AVLNode {
    size_t height;
    AVLNode* left;
    AVLNode* right;
    void* elem;
};

/// Returns the height of a node (0 if `n` is `NULL`).
size_t AVLNode_height(const AVLNode* n);

/// An AVL tree using linked nodes.
typedef struct {
    AVLNode* root;
    const size_t elem_size;
    int8_t (*const cmp)(const void*, const void*);
} AVLTree;

/// Creates an empty tree that will be ordered by `compare`.
/// No allocation is done at this call.
AVLTree AVLTree_new(size_t elem_size, int8_t (*compare)(const void*, const void*));

/// Returns the height of the tree (0 if the tree is empty).
size_t AVLTree_height(const AVLTree* avl);

/// Is the tree empty ?
bool AVLTree_is_empty(const AVLTree* avl);

/// Inserts an element in the tree.
/// The element is copied from `e`.
/// The tree might be rebalanced.
bool AVLTree_insert(AVLTree* avl, void* e);

/// Removes an element from the tree.
/// Returns `true` if an element was popped.
///   and copies the element to `removed` if `removed` is not `NULL`.
/// Returns `false` otherwise.
/// The tree might be rebalanced.
bool AVLTree_remove(AVLTree* avl, const void* e, void* removed);

/// Clears the tree, removing all elements.
/// Calls `drop_elem` on each removed element.
void AVLTree_clear(AVLTree* avl, void (*drop_elem)(void*));

/// Clears the tree, removing all elements.
void AVLTree_plain_clear(AVLTree* avl);

#endif // AVL_TREE_H
