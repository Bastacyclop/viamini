#include "util.h"

void cut_at_newline(char* s) {
    char* pos = strchr(s, '\n');
    if (pos) { *pos = '\0'; }
}

char* change_extension(const char* path, const char* ext) {
    char* sep = strchr(path, '.');
    assert(sep);
    size_t name_len = sep - path;
    size_t ext_len = strlen(ext);

    // +1 for `.` and +1 for `\0`
    char* res = malloc((name_len + 1 + ext_len + 1)*sizeof(char));
    memcpy(res, path, name_len);
    res[name_len] = '.';
    memcpy(res + name_len + 1, ext, ext_len + 1);
    return res;
}

void ask(const char* msg, const char* format, void* ptr) {
    char input[255];
    LOOP {
        ask_str(msg, input, 255);
        if (sscanf(input, format, ptr) == 1) break;
    }
}

void ask_str(const char* msg, char* s, size_t max_len) {
    fputs(msg, stdout);
    fgets(s, max_len, stdin);
    cut_at_newline(s);
}

char* str_clone(const char* s) {
    size_t len = strlen(s);
    // +1 pour '\0'
    char* clone = malloc((len + 1)*sizeof(char));
    assert_alloc(clone);
    memcpy(clone, s, (len + 1));
    return clone;
}

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
