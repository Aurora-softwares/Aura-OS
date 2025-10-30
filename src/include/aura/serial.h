#ifndef AURA_SERIAL_H
#define AURA_SERIAL_H

#include <stdint.h>

void serial_init(void);
void serial_write_char(char c);
void serial_write(const char *s);

#endif
