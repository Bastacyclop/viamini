#include "avl_tree.h"

typedef enum {
    BALANCED    =  0,
    LEFT        = -1,
    RIGHT       =  1,
    OVERLEFT    = -2,
    OVERRIGHT   =  2
} Balance;

static
AVLNode* new_node(void* e, size_t elem_size);
static
void may_drop_node(AVLNode* n);
static
void may_drop_node_with(AVLNode* n, void (*drop_elem)(void*));

static
size_t height(const AVLNode* n);
static
void update_height(AVLNode* n);
static
Balance balance(const AVLNode* n);

static
AVLNode* rotate_left(AVLNode* n);
static
AVLNode* rotate_right(AVLNode* n);
static
AVLNode* rebalance_if_overleft(AVLNode* n);
static
AVLNode* rebalance_if_overright(AVLNode* n);

static
AVLNode* avl_insert(AVLTree* avl, AVLNode* n, void* e, bool* done);
static
AVLNode* avl_remove(AVLTree* avl, AVLNode* n, const void* e, void* removed, bool* done);
static
AVLNode* remove_max(AVLTree* avl, AVLNode* n, void* e);

AVLTree AVLTree_new(size_t elem_size, int8_t (*compare)(const void*, const void*)) {
    return (AVLTree) {
        .root = NULL,
        .elem_size = elem_size,
        .cmp = compare
    };
}

void AVLTree_clear(AVLTree* avl) {
    may_drop_node(avl->root);
    avl->root = NULL;
}

void may_drop_node(AVLNode* n) {
    if (n) {
        AVLNode* l = n->left;
        AVLNode* r = n->right;
        free(n->elem);
        free(n);
        may_drop_node(l);
        may_drop_node(r);
    }
}

void AVLTree_clear_with(AVLTree* avl, void (*drop_elem)(void*)) {
    may_drop_node_with(avl->root, drop_elem);
    avl->root = NULL;
}

void may_drop_node_with(AVLNode* n, void (*drop_elem)(void*)) {
    if (n) {
        AVLNode* l = n->left;
        AVLNode* r = n->right;
        drop_elem((n)->elem);
        free(n->elem);
        free(n);
        may_drop_node_with(l, drop_elem);
        may_drop_node_with(r, drop_elem);
    }
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

size_t height(const AVLNode* n) {
    if (!n) return 0;
    return n->height;
}

size_t AVLNode_height(const AVLNode* n) {
    return height(n);
}

void update_height(AVLNode* n) {
    n->height = 1 + size_t_max(height(n->left), height(n->right));
}

Balance balance(const AVLNode* n) {
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
    bool done;
    avl->root = avl_insert(avl, avl->root, e, &done);
    return done;
}

AVLNode* avl_insert(AVLTree* avl, AVLNode* n, void* e, bool* done) {
    if (!n) {
        n = new_node(e, avl->elem_size);
        *done = true;
    } else {
        int8_t cmp = (*avl->cmp)(e, n->elem);

        if (cmp < 0) {
            n->left = avl_insert(avl, n->left, e, done);
            if (*done) {
                update_height(n);
                n = rebalance_if_overleft(n);
            }
        } else if (cmp > 0) {
            n->right = avl_insert(avl, n->right, e, done);
            if (*done) {
                update_height(n);
                n = rebalance_if_overright(n);
            }
        } else {
            *done = false;
        }
    }

    return n;
}

bool AVLTree_remove(AVLTree* avl, const void* e, void* removed) {
    bool done;
    avl->root = avl_remove(avl, avl->root, e, removed, &done);
    return done;
}

AVLNode* avl_remove(AVLTree* avl, AVLNode* n, const void* e, void* removed, bool* done) {
    if (n) {
        int8_t cmp = (*avl->cmp)(e, n->elem);

        if (cmp < 0) {
            n->left = avl_remove(avl, n->left, e, removed, done);
            if (*done) {
                update_height(n);
                n = rebalance_if_overright(n);
            }
        } else if (cmp > 0) {
            n->right = avl_remove(avl, n->right, e, removed, done);
            if (*done) {
                update_height(n);
                n = rebalance_if_overleft(n);
            }
        } else {
            if (removed) memcpy(removed, n->elem, avl->elem_size);

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
