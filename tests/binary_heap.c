#include <time.h>

#include "../src/binary_heap.h"

bool is_inf(const size_t* a, const size_t* b);

bool is_inf(const size_t* a, const size_t* b) {
    return *a < *b;
}

int main() {
    srand(time(NULL));

    #define N 10
    size_t t[N];

    for (size_t i = 0; i < N; i++) {
        t[i] = i;
    }

    size_t r[N];

    for (size_t i = 0; i < N; i++) {
        size_t ri = rand() % (N - i);
        r[i] = t[ri];
        t[ri] = t[N - 1 - i];
    }

    BinaryHeap bh = BinaryHeap_new(sizeof(size_t),
                                  (bool (*)(const void*, const void*))is_inf);
    assert(BinaryHeap_is_empty(&bh));

    for (size_t i = 0; i < N; i++) {
        BinaryHeap_push(&bh, &r[i]);
        assert(BinaryHeap_len(&bh) == i + 1);
    }

    for (size_t i = 0; i < N; i++) {
        size_t p = N;
        assert(BinaryHeap_pop(&bh, &p));
        assert(p == i);
        assert(BinaryHeap_len(&bh) == N - i - 1);
    }

    assert(BinaryHeap_is_empty(&bh));
    BinaryHeap_drop(&bh);

    return EXIT_SUCCESS;
}
