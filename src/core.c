#include "core.h"

void mem_swap(void* a, void* b, size_t n) {
    void* t = alloca(n);
    mem_swap_with(a, b, t, n);
}

void mem_swap_with(void* a, void* b, void* t, size_t n) {
    memcpy(t, a, n);
    memcpy(a, b, n);
    memcpy(b, t, n);
}

void assert_alloc(void* ptr) {
    if (!ptr) {
        perror("allocation error");
        exit(1);
    }
}

bool is_power_of_two(size_t n) {
    // bits.stephan-brumme.com
    return ((n & (n - 1)) == 0) && (n != 0);
}

size_t next_power_of_two(size_t n) {
    // bits.stephan-brumme.com
    n--;
    n |= (n >> 1);  //  2 bits
    n |= (n >> 2);  //  4 bits
    n |= (n >> 4);  //  8 bits
    n |= (n >> 8);  // 16 bits
    n |= (n >> 16); // 32 bits
    n |= (n >> 32); // 64 bits
    n++;
    return n;
}

size_t checked_next_power_of_two(size_t n) {
    size_t mb = next_power_of_two(n);
    if (mb < n) {
        perror("overflow while computing next power of two");
        exit(1);
    }
    return mb;
}

#define impl_max_min_for(T)         \
    T T##_max(T a, T b) {           \
        return (a > b) ? (a) : (b); \
    }                               \
                                    \
    T T##_min(T a, T b) {           \
        return (a < b) ? (a) : (b); \
    }

impl_max_min_for(size_t)
impl_max_min_for(uint8_t)
impl_max_min_for(uint16_t)
impl_max_min_for(uint32_t)
impl_max_min_for(uint64_t)
impl_max_min_for(int8_t)
impl_max_min_for(int16_t)
impl_max_min_for(int32_t)
impl_max_min_for(int64_t)
impl_max_min_for(float)
impl_max_min_for(double)
impl_max_min_for(int)
