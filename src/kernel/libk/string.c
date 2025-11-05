#include <stddef.h>

void *memset(void *dest, int value, size_t count) {
    unsigned char *d = (unsigned char *)dest;
    unsigned char v = (unsigned char)value;
    __asm__ __volatile__("rep stosb"
                         : "+D"(d), "+c"(count)
                         : "a"(v)
                         : "memory");
    return dest;
}

void *memcpy(void *dest, const void *src, size_t count) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    __asm__ __volatile__("rep movsb"
                         : "+D"(d), "+S"(s), "+c"(count)
                         :
                         : "memory");
    return dest;
}
