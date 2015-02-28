#include <math.h>

#include "vec.h"
#include "avl_tree.h"

typedef enum {
    BALANCED    =  0,
    LEFT        = -1,
    RIGHT       =  1,
    OVERLEFT    = -2,
    OVERRIGHT   =  2
} Balance;

AVLNode* new_node(void* e);

void rotate_left(AVLNode** root);
void rotate_right(AVLNode** root);
void rebalance_node(AVLNode** n);
void rebalance_loaded_path(Vec* path);
void rebalance_unloaded_path(Vec* path);

AVLNode** find_entry(AVLTree* avl, AVLNode** np, void* e, Vec* path);
AVLNode** find_largest_entry(AVLNode** np);

void may_drop_node(AVLNode* n, void (*drop_elem)(void*));
void may_plain_drop_node(AVLNode* n);

AVLTree AVLTree_new(int8_t (*compare)(const void*, const void*)) {
    return (AVLTree) {
        .root = NULL,
        .cmp = compare
    };
}

bool AVLTree_is_empty(const AVLTree* avl) {
    return !avl->root;
}

AVLNode* new_node(void* e) {
    AVLNode* n = malloc(sizeof(AVLNode));
    assert_alloc(n);

    *n = (AVLNode) { .elem = e, .balance = BALANCED, .left = NULL, .right = NULL };
    return n;
}

void rotate_left(AVLNode** root) {
    AVLNode* r = *root;
    AVLNode* t = r->right;

    r->right = t->left;
    t->left = r;

    *root = t;

    r->balance = r->balance - 1 + int_min(0, -t->balance);
    t->balance = t->balance - 1 + int_min(r->balance, 0);
}

void rotate_right(AVLNode** root) {
    AVLNode* r = *root;
    AVLNode* t = r->left;

    r->left = t->right;
    t->right = r;

    *root = t;

    r->balance = r->balance + 1 + int_max(-t->balance, 0);
    t->balance = t->balance + 1 + int_max(0, t->balance);
}

void rebalance_node(AVLNode** n) {
    if ((*n)->balance == OVERLEFT) {
        AVLNode** l = &(*n)->left;
        if ((*l)->balance == RIGHT) {
            rotate_left(l);
        }
        rotate_right(n);
    } else if ((*n)->balance == OVERRIGHT) {
        AVLNode** r = &(*n)->right;
        if ((*r)->balance == LEFT) {
            rotate_right(r);
        }
        rotate_left(n);
    }
}

AVLNode** find_entry(AVLTree* avl, AVLNode** np, void* e, Vec* path) {
    AVLNode* n = *np;
    if (!n) return np;

    int8_t cmp = (*avl->cmp)(n->elem, e);
    if (cmp == 0) return np;

    Vec_push(path, &np);
    if (cmp < 0) {
        return find_entry(avl, &n->left, e, path);
    } else {
        return find_entry(avl, &n->right, e, path);
    }
}

bool AVLTree_insert(AVLTree* avl, void* e) {
    Vec path = Vec_new(sizeof(AVLNode**));
    AVLNode** entry = find_entry(avl, &avl->root, e, &path);

    bool insert = !*entry;
    if (insert) {
        *entry = new_node(e);
        rebalance_loaded_path(&path);
    }

    Vec_plain_drop(&path);
    return insert;
}

void rebalance_loaded_path(Vec* path) {
    if (Vec_len(path) < 2) return;

    AVLNode** n;
    Vec_pop(path, &n);
    AVLNode** p;
    while (Vec_pop(path, &p)) {
        (*p)->balance += (*n == (*p)->left) ? (LEFT) : (RIGHT);
        rebalance_node(p);

        n = p;
    }
}

AVLNode** find_largest_entry(AVLNode** np) {
    AVLNode* n = *np;
    if (n->right) {
        return find_largest_entry(&n->right);
    }
    return np;
}

bool AVLTree_remove(AVLTree* avl, void* e) {
    Vec path = Vec_new(sizeof(AVLNode**));
    AVLNode** entry = find_entry(avl, &avl->root, e, &path);

    bool remove = *entry;
    if (remove) {
        AVLNode** p = entry;
        if ((*entry)->left && (*entry)->right) {
            AVLNode** x = find_largest_entry(&(*entry)->left);
            mem_swap(&(*entry)->elem, &(*x)->elem, sizeof(void*));
            p = x;
        }

        AVLNode* target = *p;
        if (target->left) {
            *p = target->left;
        } else if (target->right) {
            *p = target->right;
        } else {
            *p = NULL;
        }
        free(target);

        rebalance_unloaded_path(&path);
    }

    Vec_plain_drop(&path);
    return remove;
}

void rebalance_unloaded_path(Vec* path) {
    if (Vec_len(path) < 2) return;

    AVLNode** n;
    Vec_pop(path, &n);
    AVLNode** p;
    while (Vec_pop(path, &p)) {
        (*p)->balance -= (*n == (*p)->left) ? (LEFT) : (RIGHT);
        rebalance_node(p);

        n = p;
    }
}

void AVLTree_clear(AVLTree* avl, void (*drop_elem)(void*)) {
    may_drop_node(avl->root, drop_elem);
    avl->root = NULL;
}

void may_drop_node(AVLNode* n, void (*drop_elem)(void*)) {
    if (n) {
        drop_elem(n->elem);
        may_drop_node(n->left, drop_elem);
        may_drop_node(n->right, drop_elem);
    }
}

void AVLTree_plain_clear(AVLTree* avl) {
    may_plain_drop_node(avl->root);
    avl->root = NULL;
}

void may_plain_drop_node(AVLNode* n) {
    if (n) {
        may_plain_drop_node(n->left);
        may_plain_drop_node(n->right);
    }
}
