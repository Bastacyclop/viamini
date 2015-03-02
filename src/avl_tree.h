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

size_t AVLNode_height(const AVLNode* n);

typedef struct {
    AVLNode* root;
    const size_t elem_size;
    const void* (*const key_of)(const void*);
    int8_t (*const cmp)(const void*, const void*);
} AVLTree;

/// Creates an empty tree that will be ordered by `compare`.
/// No allocation is done at this call.
AVLTree AVLTree_new(size_t elem_size, const void* (*key_from_elem)(const void*),
                    int8_t (*compare)(const void*, const void*));


/// Returns the height of the tree (0 is the empty tree).
size_t AVLTree_height(const AVLTree* avl);

/// Is the tree empty ?
bool AVLTree_is_empty(const AVLTree* avl);

/// Inserts an element in the tree.
/// The element is copied from `e`.
/// The tree might be rebalanced.
bool AVLTree_insert(AVLTree* avl, void* e);

/// Removes an element from the tree matching to the given key.
/// Returns `true` if an element was popped.
///   and copies the element to `e` if `e` is not `NULL`.
/// Returns `false` otherwise.
/// The tree might be rebalanced.
bool AVLTree_remove(AVLTree* avl, const void* k, void* e);

/// Clears the tree, removing all elements.
/// Calls `drop_elem` on each removed element.
void AVLTree_clear(AVLTree* avl, void (*drop_elem)(void*));

/// Clears the tree, removing all elements.
void AVLTree_plain_clear(AVLTree* avl);


const AVLNode* AVLTree_find_sup_eq(const AVLTree* avl, const void* k);

#endif // AVL_TREE_H
