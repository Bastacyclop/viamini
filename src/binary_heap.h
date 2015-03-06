#ifndef BINARY_HEAP_H
#define BINARY_HEAP_H

#include "vec.h"

// TODO: with_capacity, reserve, reserve_exact, shrink_to_fit, ...
//       replace, push_pop, peek

/// A dynamically allocated binary heap.
typedef struct {
    Vec vec;
    bool (*strict_order)(const void*, const void*);
} BinaryHeap;

/// Creates an empty heap that will contain elements of size `elem_size`
/// and be sorted with `strict_order`.
/// No allocation is done at this call.
BinaryHeap BinaryHeap_new(size_t elem_size, bool (*strict_order)(const void*, const void*));

/// Returns the number of elements in the heap.
size_t BinaryHeap_len(const BinaryHeap* bh);

/// Is the heap empty ?
bool BinaryHeap_is_empty(const BinaryHeap* bh);

/// Returns the capacity of the heap.
size_t BinaryHeap_capacity(const BinaryHeap* bh);

/// Releases the heap resources.
/// Calls `drop_elem` on each element.
void BinaryHeap_drop(BinaryHeap* bh, void (*drop_elem)(void*));

/// Releases the heap resources.
void BinaryHeap_plain_drop(BinaryHeap* bh);

/// Pushes an element in the heap.
/// The element is copied from `e`.
/// The heap might be reorganised.
void BinaryHeap_push(BinaryHeap* bh, void* e);

/// Pops the lower element of the heap (according to its ordering).
/// Returns `true` if an element was popped.
///   and copies the element to `e` if `e` is not `NULL`.
/// Returns `false` otherwise.
/// The heap might be reorganised.
bool BinaryHeap_pop(BinaryHeap* bh, void* e);

/// Clears the heap, removing all elements.
/// Calls `drop_elem` on each removed element.
void BinaryHeap_clear(BinaryHeap* bh, void (*drop_elem)(void*));

/// Clears the heap, removing all elements.
void BinaryHeap_plain_clear(BinaryHeap* bh);

#endif // BINARY_HEAP_H
