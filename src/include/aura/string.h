#ifndef AURA_STRING_H
#define AURA_STRING_H

#include <stddef.h>

size_t strlen(const char *str);
int strcmp(const char *a, const char *b);
char *strcpy(char *dest, const char *src);
void *memset(void *dest, int value, size_t count);
void *memcpy(void *dest, const void *src, size_t count);

#endif
