#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

void serial_init(void);
void serial_write_char(char c);
void serial_write(const char *str);
void serial_write_hex8(uint8_t value);
void serial_write_hex16(uint16_t value);
void serial_write_hex32(uint32_t value);
void serial_write_uint(uint32_t value);

#endif // SERIAL_H
