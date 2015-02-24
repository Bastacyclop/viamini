#include "../src/vec.h"

void simple() {
    Vec v = Vec_new(sizeof(uint32_t));
    for (uint32_t i = 0; i < 10; i++) {
        Vec_push(&v, &i);
    }
    assert(Vec_len(&v) == 10);

    uint32_t e = UINT32_MAX;
    for (uint32_t i = 9; i < UINT32_MAX; i--) {
        Vec_pop(&v, &e);
        assert(e == i);
    }

    assert(Vec_len(&v) == 0);
    assert(Vec_pop(&v, NULL) == false);
    Vec_plain_drop(&v);
}

int main() {
    simple();

    return 0;
}
