#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include <stdint.h>

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
void lidt(void *base, uint16_t limit);

uint32_t inl(uint16_t port);
void outl(uint16_t port, uint32_t value);

#endif // ASSEMBLY_H
