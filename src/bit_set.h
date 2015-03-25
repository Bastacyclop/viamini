#ifndef BIT_SET_H
#define BIT_SET_H

#include "vec.h"

/// A dynamic bit set.

// TODO is_disjoint, is_subset, is_superset, ...
//      shrink_to_fit, reserve, check overflows, ...

typedef struct {
    Vec storage;
    size_t nbits;
} BitSet;

/// Creates an empty set.
/// No allocation is done at this call.
BitSet BitSet_new(void);

/// Creates an empty set with the specified capacity.
BitSet BitSet_with_capacity(size_t nbits);

/// Releases the set resources.
void BitSet_drop(BitSet* set);

/// Returns the number of elements in the set.
size_t BitSet_len(const BitSet* set);

/// Is the set empty ?
bool BitSet_is_empty(const BitSet* set);

/// Returns the capacity of the set.
size_t BitSet_capacity(const BitSet* set);

/// Inserts a value in the set.
/// Returns `true` if the value was inserted.
/// Returns `false` otherwise.
bool BitSet_insert(BitSet* set, size_t value);

/// Does the set contains this value ?
bool BitSet_contains(const BitSet* set, size_t value);

/// Removes a value from the set.
/// Returns `true` if the value was removed.
/// Returns `false` otherwise.
bool BitSet_remove(BitSet* set, size_t value);

/// Clears the set, removing all elements.
void BitSet_clear(BitSet* set);

#endif // BIT_SET_H
