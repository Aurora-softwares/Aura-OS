#include <aura/string.h>

size_t strlen(const char *str) {
    size_t len = 0;
    while (str && str[len]) {
        len++;
    }
    return len;
}

int strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return *(const unsigned char *)a - *(const unsigned char *)b;
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++)) {
    }
    return dest;
}

void *memset(void *dest, int value, size_t count) {
    unsigned char *d = dest;
    while (count--) {
        *d++ = (unsigned char)value;
    }
    return dest;
}

void *memcpy(void *dest, const void *src, size_t count) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (count--) {
        *d++ = *s++;
    }
    return dest;
}
