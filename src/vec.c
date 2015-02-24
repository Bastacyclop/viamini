#include "vec.h"

Vec Vec_new(size_t elem_size) {
    return (Vec) {
        data: NULL, elem_size: elem_size,
        len: 0, cap: 0
    };
}

Vec Vec_with_capacity(size_t capacity, size_t elem_size) {
    if (capacity == 0) {
        return Vec_new(elem_size);
    }

    void* data = malloc(capacity*elem_size);
    assert_alloc(data);

    return (Vec) {
        data: data, elem_size: elem_size,
        len: 0, cap: capacity
    };
}

size_t Vec_len(const Vec* v) {
    return v->len;
}

size_t Vec_capacity(const Vec* v) {
    return v->cap;
}

bool Vec_is_empty(const Vec* v) {
    return Vec_len(v) == 0;
}

void* Vec_unsafe_get_mut(Vec* v, size_t i) {
    return v->data + i*v->elem_size;
}

const void* Vec_unsafe_get(const Vec* v, size_t i) {
    return v->data + i*v->elem_size;
}

void assert_bound(const Vec* v, size_t i) {
    // using size_t so i > 0
    if (i > v->len) {
        perror("Vec index out of bounds");
        exit(1);
    }
}

void* Vec_get_mut(Vec* v, size_t i) {
    assert_bound(v, i);
    return Vec_unsafe_get_mut(v, i);
}

const void* Vec_get(const Vec* v, size_t i) {
    assert_bound(v, i);
    return Vec_unsafe_get(v, i);
}

void drop_elems(Vec* v, void (*drop_elem)(void*)) {
    for (uint32_t i = 0; i < v->len; i++) {
        (*drop_elem)(Vec_unsafe_get_mut(v, i));
    }
}

void Vec_drop(Vec* v, void (*drop_elem)(void*)) {
    drop_elems(v, drop_elem);
    free(v->data);
}

void Vec_plain_drop(Vec* v) {
    free(v->data);
}

void Vec_reserve(Vec* v, size_t needed) {
    size_t new_len = needed + v->len;
    if (new_len <= v->cap) return;

    size_t new_cap = next_power_of_two(new_len);
    v->data = realloc(v->data, v->elem_size*new_cap);
    assert_alloc(v->data);
    v->cap = new_cap;
}

void Vec_push(Vec* v, void* e) {
    Vec_reserve(v, 1);
    void* back = Vec_unsafe_get_mut(v, v->len);
    memcpy(back, e, v->elem_size);
    v->len++;
}

bool Vec_pop(Vec* v, void* e) {
    if (v->len == 0) return false;
    if (e) {
        const void* back = Vec_unsafe_get(v, v->len - 1);
        memcpy(e, back, v->elem_size);
    }
    v->len--;
    return true;
}

void Vec_clear(Vec* v, void (*drop_elem)(void*)) {
    drop_elems(v, drop_elem);
    v->len = 0;
}

void Vec_plain_clear(Vec* v) {
    v->len = 0;
}

void Vec_swap_remove(Vec* v, void* e, size_t i) {
    assert_bound(v, i);
    size_t len = Vec_len(v);

    if (i == len - 1) {
        Vec_pop(v, e);
    } else {
        void* e_i = Vec_get_mut(v, i);
        if (e) {
            memcpy(e, e_i, v->elem_size);
        }
        const void* back = Vec_unsafe_get(v, len - 1);
        memcpy(e_i, back, v->elem_size);
        v->len--;
    }
}
