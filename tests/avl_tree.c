#include "../src/avl_tree.h"

void print(const AVLTree* avl);
void print_rec(const AVLNode* n, uint8_t shift);
int8_t cmp(size_t a, size_t b);

void print(const AVLTree* avl) {
    print_rec(avl->root, 1);
}

void print_rec(const AVLNode* n, uint8_t shift) {
    if (n) {
        printf("%*zu\n", shift, (uint64_t)n->elem);
        print_rec(n->left, shift + 1);
        printf("%*c\n", shift, '-');
        print_rec(n->right, shift + 1);
    } else {
        printf("%*c\n", shift, '.');
    }
}

int8_t cmp(size_t a, size_t b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

int main() {
    AVLTree avl = AVLTree_new((int8_t (*)(const void*, const void*))cmp);

    size_t t[] = { 1, 0, 4, 2, 3, 5 };
    for (size_t i = 0; i <= 5; i++) {
        AVLTree_insert(&avl, (void*)t[i]);
    }

    for (size_t i = 0; i <= 5; i++) {
        bool removed = AVLTree_remove(&avl, (void*)i);
        assert(removed);
    }

    AVLTree_plain_clear(&avl);
}
