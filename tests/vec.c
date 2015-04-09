#include "../src/vec.h"

int main() {
    Vec v = Vec_new(sizeof(uint32_t));
    assert(Vec_is_empty(&v));

    #define N 10
    for (uint32_t i = 0; i < N; i++) {
        Vec_push(&v, &i);
        assert(Vec_len(&v) == i + 1);
    }

    for (uint32_t i = N - 1; i < UINT32_MAX; i--) {
        uint32_t e = UINT32_MAX;
        assert(Vec_pop(&v, &e));
        assert(e == i);
    }

    assert(Vec_is_empty(&v));
    assert(Vec_pop(&v, NULL) == false);
    Vec_drop(&v);

    return EXIT_SUCCESS;
}
