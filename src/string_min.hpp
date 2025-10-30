// string_min.hpp (freestanding mini string helpers)
#pragma once
#include <stdint.h>

// Returns length of a null-terminated ASCII string
static inline size_t str_len(const char* s) {
    size_t n = 0;
    if (!s) return 0;
    while (s[n] != '\0') ++n;
    return n;
}

// Case-sensitive equality: returns true if a == b (both null-terminated)
static inline bool str_eq(const char* a, const char* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    while (*a && *b) {
        if (*a != *b) return false;
        ++a; ++b;
    }
    return *a == *b; // both must hit '\0'
}

// ASCII to lower (no locale, no libc)
static inline char ascii_lower(char c) {
    if (c >= 'A' && c <= 'Z') return char(c - 'A' + 'a');
    return c;
}

// Case-insensitive ASCII equality
static inline bool str_ieq_ascii(const char* a, const char* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    while (*a && *b) {
        if (ascii_lower(*a) != ascii_lower(*b)) return false;
        ++a; ++b;
    }
    return *a == *b;
}
