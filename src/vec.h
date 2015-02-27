#ifndef VEC_H
#define VEC_H

#include "util.h"

/// A dynamically sized array.
typedef struct {
    void* data;
    const size_t elem_size;
    size_t len;
    size_t cap;
} Vec;

// TODO: shrink_to_fit, reserve_exact, truncate, insert, remove, append, sort, ..
//       check overflows ?

/// Creates an empty vector that will contain elements of size `elem_size`.
/// No allocation is done at this call.
Vec Vec_new(size_t elem_size);

/// Creates an empty vector with the specified capacity.
Vec Vec_with_capacity(size_t capacity, size_t elem_size);

/// Returns the number of elements in the vector.
size_t Vec_len(const Vec* v);

/// Returns the capacity of the vector.
size_t Vec_capacity(const Vec* v);

/// Is the vector empty ?
bool Vec_is_empty(const Vec* v);

/// Releases the vector resources.
/// Calls `drop_elem` on each element.
void Vec_drop(Vec* v, void (*drop_elem)(void*));

/// Releases the vector resources.
void Vec_plain_drop(Vec* v);

/// Reserves capacity for at least `needed` more elements.
void Vec_reserve(Vec* v, size_t needed);

/// Reserves capacity for at least `len` elements in total.
void Vec_reserve_len(Vec* v, size_t len);

/// Retrieves a pointer to the element of index `i` in the vector.
/// Index out of bounds results in undefined behavior.
void* Vec_unsafe_get_mut(Vec* v, size_t i);

/// Retrieves an immutable pointer to the element of index `i` in the vector.
/// Index out of bounds results in undefined behavior.
const void* Vec_unsafe_get(const Vec* v, size_t i);

/// Retrieves a pointer to the element of index `i` in the vector.
/// Index out of bounds results in an error.
void* Vec_get_mut(Vec* v, size_t i);

/// Retrieves an immutable pointer to the element of index `i` in the vector.
/// Index out of bounds results in an error.
const void* Vec_get(const Vec* v, size_t i);

/// Pushes an element to the back of the vector.
/// The element is copied from `e`.
void Vec_push(Vec* v, void* e);

/// Pops an element from the back of the vector.
/// Returns `true` if an element was popped.
///   and copies the element to `e` if `e` is not `NULL`.
/// Returns `false` otherwise.
bool Vec_pop(Vec* v, void* e);

/// Clears the vector, removing all elements.
/// Calls `drop_elem` on each removed element.
void Vec_clear(Vec* v, void (*drop_elem)(void*));

/// Clears the vector, removing all elements.
void Vec_plain_clear(Vec* v);

/// Removes an element from the vector, replacing it with the last element.
/// The removed element is copied to `e` if `e` is not `NULL`.
/// Index out of bounds results in an error.
void Vec_swap_remove(Vec* v, void* e, size_t i);

#endif // VEC_H
