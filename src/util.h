#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <alloca.h>

#define LOOP while (true)

/// Replaces the first '\n' found by '\0'.
void cut_at_newline(char* s);

/// Prints a message and waits for an input.
/// Loops while the format is not respected.
void ask(const char* msg, const char* format, void* ptr);

/// Prints a message and waits for a string input.
void ask_str(const char* msg, char* s, size_t max_len);

/// Clones the given string, allocating the necessary memory.
char* str_clone(const char* s);

/// Swaps the given values using a temporary stack variable.
void mem_swap(void* a, void* b, size_t n);

/// Swaps the given values using `t` as temporary variable.
void mem_swap_with(void* a, void* b, void* t, size_t n);

/// Fires an error if `ptr` is `NULL`.
void assert_alloc(void* ptr);

/// Is `n` a power of two ?
bool is_power_of_two(size_t n);

/// Returns the power of two following `n`.
size_t next_power_of_two(size_t n);

/// Returns the power of two following `n`.
/// Fires an error if there is an overflow.
size_t checked_next_power_of_two(size_t n);

#define def_max_min_for(T)      \
    T T##_max(T a, T b);        \
    T T##_min(T a, T b);

def_max_min_for(size_t)
def_max_min_for(uint8_t)
def_max_min_for(uint16_t)
def_max_min_for(uint32_t)
def_max_min_for(uint64_t)
def_max_min_for(int8_t)
def_max_min_for(int16_t)
def_max_min_for(int32_t)
def_max_min_for(int64_t)
def_max_min_for(float)
def_max_min_for(double)

#endif // UTIL_H
