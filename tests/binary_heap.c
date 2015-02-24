#include "../src/binary_heap.h"

bool is_inf(const int32_t* a, const int32_t* b);

bool is_inf(const int32_t* a, const int32_t* b) {
    return *a < *b;
}

int main() {
    BinaryHeap bh = BinaryHeap_new(sizeof(int32_t),
                                  (bool (*)(const void*, const void*))is_inf);

    int32_t t[] = { 1, 0, 4, 2, 3, 5 };
    for (size_t i = 0; i <= 5; i++) {
        BinaryHeap_insert(&bh, t + i);
    }

    for (int32_t i = 0; i <= 5; i++) {
        int32_t p;
        assert(BinaryHeap_pop(&bh, &p));
        assert(p == i);
    }

    BinaryHeap_plain_drop(&bh);
}
