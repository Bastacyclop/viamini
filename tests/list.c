#include "../src/list.h"

const int32_t N = 10;

int main() {
    List l = List_new(sizeof(int32_t));
    assert(List_is_empty(&l));

    for (int32_t i = 0; i < N; i++) {
        List_push(&l, &i);
        assert(List_len(&l) == (size_t)(i + 1));
    }

    assert(*(int32_t*)List_peek(&l) == 9);

    for (int32_t i = 0; i < N; i++) {
        int32_t e = INT32_MAX;
        assert(List_pop(&l, &e));
        assert(e == (9 - i));
        assert(List_len(&l) == (size_t)(N - (i + 1)));
    }

    assert(List_is_empty(&l));

    return EXIT_SUCCESS;
}
