#include "../src/bit_set.h"

int main() {
    BitSet set = BitSet_new();
    assert(BitSet_is_empty(&set));

    assert(!BitSet_contains(&set, 2));
    assert(!BitSet_remove(&set, 2));
    assert(BitSet_insert(&set, 2));
    assert(BitSet_contains(&set, 2));
    assert(BitSet_len(&set) == 1);
    assert(!BitSet_insert(&set, 2));
    assert(BitSet_remove(&set, 2));
    assert(!BitSet_contains(&set, 2));
    assert(!BitSet_remove(&set, 2));

    for (size_t i = 0; i < 80; i += 5) {
        assert(BitSet_insert(&set, i));
    }

    assert(BitSet_len(&set) == 80 / 5);

    for (size_t i = 0; i < 80; i += 5) {
        assert(BitSet_contains(&set, i));
        assert(!BitSet_contains(&set, i + 1));
    }

    for (size_t i = 0; i < 80; i += 10) {
        assert(BitSet_remove(&set, i));
    }

    assert(BitSet_len(&set) == 80 / 10);

    BitSet_clear(&set);
    assert(BitSet_is_empty(&set));

    BitSet_drop(&set);

    return EXIT_SUCCESS;
}
