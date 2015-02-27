#ifndef BIT_SET_H
#define BIT_SET_H

#include "vec.h"

typedef struct {
    Vec storage;
    size_t nbits;
} BitSet;

// TODO is_disjoint, is_subset, is_superset, ...
//      shrink_to_fit, reserve, check overflows, ...

BitSet BitSet_new(void);
BitSet BitSet_with_capacity(size_t nbits);
void BitSet_drop(BitSet* set);

size_t BitSet_len(const BitSet* set);
bool BitSet_is_empty(const BitSet* set);
size_t BitSet_capacity(const BitSet* set);

bool BitSet_insert(BitSet* set, size_t value);
bool BitSet_contains(const BitSet* set, size_t value);
bool BitSet_remove(BitSet* set, size_t value);

void BitSet_clear(BitSet* set);

#endif // BIT_SET_H
