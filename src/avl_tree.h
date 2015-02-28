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

AVLTree AVLTree_new(int8_t (*compare)(const void*, const void*));

bool AVLTree_is_empty(const AVLTree* avl);

bool AVLTree_insert(AVLTree* avl, void* e);

bool AVLTree_remove(AVLTree* avl, void* e);

void AVLTree_clear(AVLTree* avl, void (*drop_elem)(void*));

void AVLTree_plain_clear(AVLTree* avl);

#endif // AVL_TREE_H
