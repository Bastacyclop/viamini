#include <time.h>

#include "../src/avl_tree.h"

typedef struct {
    int32_t min;
    int32_t max;
} Meta;

void check(const AVLTree* t);
Meta check_node(const AVLNode* n);
void print_node(const AVLNode* n);

int8_t cmp(const int32_t* a, const int32_t* b);

void check(const AVLTree* t) {
    if (!AVLTree_is_empty(t)) {
        check_node(t->root);
    }
}

// `n` can't be `NULL`.
Meta check_node(const AVLNode* n) {
    assert(n);
    int32_t e = *(int32_t*)n->elem;

    Meta m;
    if (n->left) {
        Meta l = check_node(n->left);
        assert(l.max < e);
        m.min = l.min;
    } else {
        m.min = e;
    }
    if (n->right) {
        Meta r = check_node(n->right);
        assert(r.min > e);
        m.max = r.max;
    } else {
        m.max = e;
    }

    size_t h_l = AVLNode_height(n->left);
    size_t h_r = AVLNode_height(n->right);
    assert(n->height == 1 + size_t_max(h_l, h_r));
    int64_t bal = (int64_t)(h_r) - (int64_t)(h_l);
    assert(-2 < bal && bal < 2);

    return m;
}

void print_node(const AVLNode* n) {
    if (n) {
        printf("%d\n", *(int32_t*)n->elem);
        printf("< "); print_node(n->left);
        printf("> "); print_node(n->right);
    } else {
        puts("");
    }
}

int8_t cmp(const int32_t* a, const int32_t* b) {
    if (*a < *b) {
        return -1;
    } else if (*a > *b) {
        return  1;
    } else {
        return 0;
    }
}

int main() {
    srand(time(NULL));

    #define N 40
    int32_t t[N] = {
        0, -3, 8, 13, -2, 4, 7, -10, 29, 16, 31, -23, 9, -7, -14, 19, 24, 38, -32, 43,
        100, -300, 800, 130, -200, 400, 700, -100, 290, 160, 310, -230, 900, -700,
        -140, 190, 240, 380, -320, 430
    };

    int32_t r[N];

    for (size_t i = 0; i < N; i++) {
        size_t ri = rand() % (N - i);
        r[i] = t[ri];
        t[ri] = t[N - 1 - i];
    }

    AVLTree avl = AVLTree_new(sizeof(int32_t),
                              (int8_t (*)(const void*, const void*))cmp);
    assert(AVLTree_is_empty(&avl));

    for (size_t i = 0; i < N; i++) {
        assert(AVLTree_insert(&avl, &r[i]));
        assert(!AVLTree_insert(&avl, &r[i]));
        check(&avl);
    }

    for (size_t i = N - 1; i < SIZE_MAX; i--) {
        int32_t e;
        assert(AVLTree_remove(&avl, &r[i], &e));
        assert(!AVLTree_remove(&avl, &r[i], &e));
        assert(e == r[i]);
        check(&avl);
    }

    assert(AVLTree_is_empty(&avl));
    AVLTree_clear(&avl);

    return EXIT_SUCCESS;
}
