#include "binary_heap.h"

static
size_t root_index(void);
static
size_t left_child_index(size_t parent);
static
size_t right_child_index(size_t parent);
static
size_t parent_index(size_t child);

static
void swap(BinaryHeap* bh, void* a, void* b);
static
void swap_root_pop(BinaryHeap* bh, void* e);

static
void may_bubble_up(BinaryHeap* bh, size_t i);
static
void may_bubble_down(BinaryHeap* bh, size_t i);

static
size_t heap_get_mut_lower_child(BinaryHeap* bh, size_t i, void** e);

BinaryHeap BinaryHeap_new(size_t elem_size, bool (*strict_order)(const void*, const void*)) {
    return (BinaryHeap) {
        .vec = Vec_new(elem_size),
        .strict_order = strict_order
    };
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

void BinaryHeap_push(BinaryHeap* bh, void* elem) {
    size_t i = Vec_len(&bh->vec);
    Vec_push(&bh->vec, elem);
    may_bubble_up(bh, i);
}

void may_bubble_up(BinaryHeap* bh, size_t i) {
    if (i != root_index()) {
        size_t p = parent_index(i);
        void* parent = Vec_unsafe_get_mut(&bh->vec, p);
        void* elem = Vec_unsafe_get_mut(&bh->vec, i);

        if ((*bh->strict_order)(elem, parent)) {
            swap(bh, elem, parent);
            may_bubble_up(bh, p);
        }
    }
}

void swap(BinaryHeap* bh, void* a, void* b) {
    size_t len = Vec_len(&bh->vec);
    if (len < Vec_capacity(&bh->vec)) {
        mem_swap_with(a, b, Vec_unsafe_get_mut(&bh->vec, len), bh->vec.elem_size);
    } else {
        mem_swap(a, b, bh->vec.elem_size);
    }
}

bool BinaryHeap_pop(BinaryHeap* bh, void* e) {
    size_t len = Vec_len(&bh->vec);
    if (len < 2) return Vec_pop(&bh->vec, e);

    swap_root_pop(bh, e);
    may_bubble_down(bh, root_index());
    return true;
}

void swap_root_pop(BinaryHeap* bh, void* e) {
    void* root = Vec_unsafe_get_mut(&bh->vec, root_index());
    if (e) {
        memcpy(e, root, bh->vec.elem_size);
    }
    Vec_pop(&bh->vec, root);
}

void may_bubble_down(BinaryHeap* bh, size_t i) {
    void* target = NULL;
    size_t index = heap_get_mut_lower_child(bh, i, &target);

    if (target) {
        void* elem = Vec_unsafe_get_mut(&bh->vec, i);
        if ((*bh->strict_order)(target, elem)) {
            swap(bh, elem, target);
            may_bubble_down(bh, index);
        }
    }
}

size_t heap_get_mut_lower_child(BinaryHeap* bh, size_t i, void** e) {
    size_t len = Vec_len(&bh->vec);

    size_t lc = left_child_index(i);
    if (lc >= len) return 0;

    void* left_child = Vec_unsafe_get_mut(&bh->vec, lc);
    size_t rc = right_child_index(i);
    if (rc >= len) {
        *e = left_child;
        return lc;
    }

    void* right_child = Vec_unsafe_get_mut(&bh->vec, rc);
    if ((*bh->strict_order)(left_child, right_child)) {
        *e = left_child;
        return lc;
    } else {
        *e = right_child;
        return rc;
    }
}

void BinaryHeap_clear(BinaryHeap* bh, void (*drop_elem)(void*)) {
    Vec_clear(&bh->vec, drop_elem);
}

void BinaryHeap_plain_clear(BinaryHeap* bh) {
    Vec_plain_clear(&bh->vec);
}
