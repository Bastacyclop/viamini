#include "binary_heap.h"

size_t root_index(void);
size_t left_child_index(size_t parent);
size_t right_child_index(size_t parent);
size_t parent_index(size_t child);
void heap_swap(BinaryHeap* bh, void* a, void* b);
void may_bubble_up(BinaryHeap* bh, size_t i);
void may_bubble_down(BinaryHeap* bh, size_t i);

BinaryHeap BinaryHeap_new(size_t elem_size, bool (*predicate)(const void*, const void*)) {
    return (BinaryHeap) {
        .vec = Vec_new(elem_size),
        .predicate = predicate
    };
}

size_t BinaryHeap_len(const BinaryHeap* bh) {
    return Vec_len(&bh->vec);
}

bool BinaryHeap_is_empty(const BinaryHeap* bh) {
    return Vec_is_empty(&bh->vec);
}

size_t BinaryHeap_capacity(const BinaryHeap* bh) {
    return Vec_capacity(&bh->vec);
}

void BinaryHeap_drop(BinaryHeap* bh, void (*drop_elem)(void*)) {
    Vec_drop(&bh->vec, drop_elem);
}

void BinaryHeap_plain_drop(BinaryHeap* bh) {
    Vec_plain_drop(&bh->vec);
}

size_t root_index() {
    return 0;
}

size_t left_child_index(size_t parent) {
    return 2*(parent) + 1;
}

size_t right_child_index(size_t parent) {
    return left_child_index(parent) + 1;
}

size_t parent_index(size_t child) {
    return (child - 1) / 2;
}

void heap_swap(BinaryHeap* bh, void* a, void* b) {
    size_t len = Vec_len(&bh->vec);
    if (len < Vec_capacity(&bh->vec)) {
        mem_swap_with(a, b, Vec_unsafe_get_mut(&bh->vec, len), bh->vec.elem_size);
    } else {
        mem_swap(a, b, bh->vec.elem_size);
    }
}

void may_bubble_up(BinaryHeap* bh, size_t i) {
    if (i == root_index()) return;
    size_t p = parent_index(i);
    void* parent = Vec_unsafe_get_mut(&bh->vec, p);
    void* elem = Vec_unsafe_get_mut(&bh->vec, i);
    if ((*bh->predicate)(elem, parent)) {
        heap_swap(bh, elem, parent);
        may_bubble_up(bh, p);
    }
}

void BinaryHeap_insert(BinaryHeap* bh, void* elem) {
    size_t i = Vec_len(&bh->vec);
    Vec_push(&bh->vec, elem);
    may_bubble_up(bh, i);
}

void may_bubble_down(BinaryHeap* bh, size_t i) {
    #define try_with(target, index)                 \
        if ((*bh->predicate)(target, elem)) {       \
            heap_swap(bh, elem, target);            \
            may_bubble_down(bh, index);             \
        }

    size_t len = Vec_len(&bh->vec);

    size_t lc = left_child_index(i);
    if (lc >= len) return;
    void* elem = Vec_unsafe_get_mut(&bh->vec, i);
    void* left_child = Vec_unsafe_get_mut(&bh->vec, lc);

    size_t rc = right_child_index(i);
    if (rc >= len) {
        try_with(left_child, lc);
    } else {
        void* right_child = Vec_unsafe_get_mut(&bh->vec, rc);

        if ((*bh->predicate)(left_child, right_child)) {
            try_with(left_child, lc);
        } else {
            try_with(right_child, rc);
        }
    }

    #undef try_with
}

bool BinaryHeap_pop(BinaryHeap* bh, void* e) {
    size_t len = Vec_len(&bh->vec);
    if (len < 2) {
        return Vec_pop(&bh->vec, e);
    }
    void* root = Vec_unsafe_get_mut(&bh->vec, root_index());
    if (e) {
        memcpy(e, root, bh->vec.elem_size);
    }
    Vec_pop(&bh->vec, root);
    may_bubble_down(bh, root_index());
    return true;
}

void BinaryHeap_clear(BinaryHeap* bh, void (*drop_elem)(void*)) {
    Vec_clear(&bh->vec, drop_elem);
}

void BinaryHeap_plain_clear(BinaryHeap* bh) {
    Vec_plain_clear(&bh->vec);
}

