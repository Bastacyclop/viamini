#include "../src/avl_tree.h"

const int32_t* key_from_elem(const int32_t* n);
int8_t cmp(const int32_t* a, const int32_t* b);

const int32_t* key_from_elem(const int32_t* n) {
    return n;
}

int8_t cmp(const int32_t* a, const int32_t* b) {
    if (*a < *b) return -1;
    if (*a > *b) return 1;
    return 0;
}

int main() {
    AVLTree avl = AVLTree_new(sizeof(int32_t),
                              (const void* (*)(const void*))key_from_elem,
                              (int8_t (*)(const void*, const void*))cmp);

    int32_t t[] = { 1, -6, 4, 2, -3, 5 };
    for (size_t i = 0; i < 6; i++) {
        assert(AVLTree_insert(&avl, &t[i]));
    }

    int32_t r[] = { 2, -3, 5, 1, -6, 4 };
    for (size_t i = 0; i < 6; i++) {
        int32_t e;
        assert(AVLTree_remove(&avl, &r[i], &e));
        assert(e == r[i]);
    }

    AVLTree_plain_clear(&avl);

    return EXIT_SUCCESS;
}
