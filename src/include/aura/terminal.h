#ifndef AURA_TERMINAL_H
#define AURA_TERMINAL_H

#include <stddef.h>
#include <stdint.h>

void terminal_init(void);
void terminal_clear(void);
void terminal_setcolor(uint8_t color);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void terminal_putchar(char c);
void terminal_write(const char *str);
void terminal_writeline(const char *str);

#endif
