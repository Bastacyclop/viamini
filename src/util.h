#ifndef UTIL_H
#define UTIL_H

#include "vec.h"

#define TERM_GREEN(str) "\x1B[32m"str"\033[0m"

/// Replaces the first '\n' found by '\0'.
void cut_at_newline(char* s);

/// Returns a new file path with the extension `ext`.
char* change_extension(const char* path, const char* ext);

typedef Vec StrVec;

/// Returns the lines outputed by the system's `ls` command.
StrVec find(const char* pattern);

/// Prints a message and waits for an input.
/// Loops while the format is not respected.
void ask(const char* msg, const char* format, void* ptr);

/// Prints a message and waits for a string input.
void ask_str(const char* msg, char* s, size_t max_len);

/// Clones the given string, allocating the necessary memory.
char* str_clone(const char* s);

/// Surrounds the given string with the given prefix and suffix.
char* str_surround(const char* prefix, const char* s, const char* suffix);

#endif // UTIL_H
