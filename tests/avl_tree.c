#include <time.h>

#include "../src/avl_tree.h"

void check_node(const AVLNode* n);
void print_node(const AVLNode* n);

const int32_t* key_from_elem(const int32_t* n);
int8_t cmp(const int32_t* a, const int32_t* b);

void check_node(const AVLNode* n) {
    if (n) {
        check_node(n->left);
        check_node(n->right);
        if (n->left) {
            assert(*(int32_t*)n->left->elem < *(int32_t*)n->elem);
        }
        if (n->right) {
            assert(*(int32_t*)n->right->elem > *(int32_t*)n->elem);
        }
        assert(n->height == 1 + size_t_max(AVLNode_height(n->left), AVLNode_height(n->right)));
        int64_t bal = (int64_t)AVLNode_height(n->right) - (int64_t)AVLNode_height(n->left);
        assert(-2 < bal && bal < 2);
    }
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

const int32_t* key_from_elem(const int32_t* n) {
    return n;
}

int8_t cmp(const int32_t* a, const int32_t* b) {
    if (*a < *b) return -1;
    if (*a > *b) return 1;
    return 0;
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
                              (const void* (*)(const void*))key_from_elem,
                              (int8_t (*)(const void*, const void*))cmp);
    assert(AVLTree_is_empty(&avl));

    for (size_t i = 0; i < N; i++) {
        assert(AVLTree_insert(&avl, &r[i]));
        assert(!AVLTree_insert(&avl, &r[i]));
        check_node(avl.root);
    }

    for (size_t i = N - 1; i < SIZE_MAX; i--) {
        int32_t e;
        assert(AVLTree_remove(&avl, &r[i], &e));
        assert(!AVLTree_remove(&avl, &r[i], &e));
        assert(e == r[i]);
        check_node(avl.root);
    }

    assert(AVLTree_is_empty(&avl));
    AVLTree_plain_clear(&avl);

    return EXIT_SUCCESS;
}
