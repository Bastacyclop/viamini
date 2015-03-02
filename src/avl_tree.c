#include "avl_tree.h"

typedef enum {
    BALANCED    =  0,
    LEFT        = -1,
    RIGHT       =  1,
    OVERLEFT    = -2,
    OVERRIGHT   =  2
} Balance;

AVLNode* new_node(void* e, size_t elem_size);

static
size_t height(AVLNode* n);
void update_height(AVLNode* n);
Balance balance(AVLNode* n);

AVLNode* rotate_left(AVLNode* n);
AVLNode* rotate_right(AVLNode* n);
AVLNode* rebalance_if_overleft(AVLNode* n);
AVLNode* rebalance_if_overright(AVLNode* n);

AVLNode* avl_insert(AVLTree* avl, AVLNode* n, const void* k, void* e, bool* done);
AVLNode* avl_remove(AVLTree* avl, AVLNode* n, const void* k, void* e, bool* done);
AVLNode* remove_max(AVLTree* avl, AVLNode* n, void* e);

void may_drop_node(AVLNode* n, void (*drop_elem)(void*));
void may_plain_drop_node(AVLNode* n);

AVLTree AVLTree_new(size_t elem_size, const void* (*key_from_elem)(const void*),
                    int8_t (*compare)(const void*, const void*)) {
    return (AVLTree) {
        .root = NULL,
        .elem_size = elem_size,
        .key_of = key_from_elem,
        .cmp = compare
    };
}

size_t AVLTree_height(const AVLTree* avl) {
    return height(avl->root);
}

bool AVLTree_is_empty(const AVLTree* avl) {
    return AVLTree_height(avl) == 0;
}

AVLNode* new_node(void* e, size_t elem_size) {
    AVLNode* n = malloc(sizeof(AVLNode));
    assert_alloc(n);
    void* elem = malloc(elem_size); // we could avoid double allocation.
    assert_alloc(elem);
    memcpy(elem, e, elem_size);

    *n = (AVLNode) { .height = 1, .left = NULL, .right = NULL, .elem = elem };
    return n;
}

size_t height(AVLNode* n) {
    if (!n) return 0;
    return n->height;
}

void update_height(AVLNode* n) {
    n->height = 1 + size_t_max(height(n->left), height(n->right));
}

Balance balance(AVLNode* n) {
    return (int64_t)(-height(n->left)) + (int64_t)(height(n->right));
}

// n has to have a right child.
AVLNode* rotate_left(AVLNode* n) {
    AVLNode* t = n->right;

    n->right = t->left;
    t->left = n;

    update_height(n);
    update_height(t);
    return t;
}

// n has to have a left child.
AVLNode* rotate_right(AVLNode* n) {
    AVLNode* t = n->left;

    n->left = t->right;
    t->right = n;

    update_height(n);
    update_height(t);
    return t;
}

AVLNode* rebalance_if_overleft(AVLNode* n) {
    Balance bal = balance(n);
    if (bal == OVERLEFT) {
        Balance l_bal = balance(n->left);
        if (l_bal == RIGHT) {
            n->left = rotate_left(n->left);
        }
        n = rotate_right(n);
    }
    return n;
}

AVLNode* rebalance_if_overright(AVLNode* n) {
    Balance bal = balance(n);
    if (bal == OVERRIGHT) {
        Balance r_bal = balance(n->right);
        if (r_bal == LEFT) {
            n->right = rotate_right(n->right);
        }
        n = rotate_left(n);
    }
    return n;
}

bool AVLTree_insert(AVLTree* avl, void* e) {
    const void* k = (*avl->key_of)(e);
    bool done;
    avl->root = avl_insert(avl, avl->root, k, e, &done);
    return done;
}

AVLNode* avl_insert(AVLTree* avl, AVLNode* n, const void* k, void* e, bool* done) {
    if (n) {
        const void* nk = (*avl->key_of)(n->elem);
        int8_t cmp = (*avl->cmp)(k, nk);

        if (cmp < 0) {
            n->left = avl_insert(avl, n->left, k, e, done);
            if (*done) {
                update_height(n);
                n = rebalance_if_overleft(n);
            }
        } else if (cmp > 0) {
            n->right = avl_insert(avl, n->right, k, e, done);
            if (*done) {
                update_height(n);
                n = rebalance_if_overright(n);
            }
        } else {
            *done = false;
        }
    } else {
        n = new_node(e, avl->elem_size);
        *done = true;
    }

    return n;
}

bool AVLTree_remove(AVLTree* avl, const void* k, void* e) {
    bool done;
    avl->root = avl_remove(avl, avl->root, k, e, &done);
    return done;
}

AVLNode* avl_remove(AVLTree* avl, AVLNode* n, const void* k, void* e, bool* done) {
    if (n) {
        const void* nk = (*avl->key_of)(n->elem);
        int8_t cmp = (*avl->cmp)(k, nk);

        if (cmp < 0) {
            n->left = avl_remove(avl, n->left, k, e, done);
            if (*done) {
                update_height(n);
                n = rebalance_if_overright(n);
            }
        } else if (cmp > 0) {
            n->right = avl_remove(avl, n->right, k, e, done);
            if (*done) {
                update_height(n);
                n = rebalance_if_overleft(n);
            }
        } else {
            if (e) memcpy(e, n->elem, avl->elem_size);

            if (n->left && n->right) {
                n->left = remove_max(avl, n->left, n->elem);
            } else {
                AVLNode* t = n;
                if (t->left) {
                    n = t->left;
                } else {
                    // if there is no right either then `n` will be `NULL`,
                    // thats what we want.
                    n = t->right;
                }
                free(t->elem);
                free(t);
            }

            if (n) { // might be done better.
                update_height(n);
                n = rebalance_if_overleft(n);
                n = rebalance_if_overright(n);
            }
            *done = true;
        }
    } else {
        *done = false;
    }

    return n;
}

AVLNode* remove_max(AVLTree* avl, AVLNode* n, void* e) {
    if (n->right) {
        n->right = remove_max(avl, n->right, e);
        update_height(n);
        n = rebalance_if_overleft(n);
    } else {
        memcpy(e, n->elem, avl->elem_size);

        AVLNode* t = n;
        // if there is no left then `n` will be `NULL`,
        // thats what we want because we already know that there is no right.
        n = t->left;
        free(t->elem);
        free(t);

        if (n) { // might be done better.
            update_height(n);
            n = rebalance_if_overleft(n);
            n = rebalance_if_overright(n);
        }
    }

    return n;
}

void AVLTree_clear(AVLTree* avl, void (*drop_elem)(void*)) {
    may_drop_node(avl->root, drop_elem);
    avl->root = NULL;
}

void may_drop_node(AVLNode* n, void (*drop_elem)(void*)) {
    if (n) {
        AVLNode* l = n->left;
        AVLNode* r = n->right;
        drop_elem((n)->elem);
        free(n->elem);
        free(n);
        may_drop_node(l, drop_elem);
        may_drop_node(r, drop_elem);
    }
}

void AVLTree_plain_clear(AVLTree* avl) {
    may_plain_drop_node(avl->root);
    avl->root = NULL;
}

void may_plain_drop_node(AVLNode* n) {
    if (n) {
        AVLNode* l = n->left;
        AVLNode* r = n->right;
        free(n->elem);
        free(n);
        may_plain_drop_node(l);
        may_plain_drop_node(r);
    }
}


const AVLNode* find_sup_eq(const AVLTree* avl, const AVLNode* n, const void* k);

const AVLNode* AVLTree_find_sup_eq(const AVLTree* avl, const void* k) {
    return find_sup_eq(avl, avl->root, k);
}

const AVLNode* find_sup_eq(const AVLTree* avl, const AVLNode* n, const void* k) {
    if (n) {
        const void* nk = (*avl->key_of)(n->elem);
        int8_t cmp = (*avl->cmp)(k, nk);

        if (cmp > 0) {
            n = find_sup_eq(avl, n->right, k);
        }
    }
    return n;
}
