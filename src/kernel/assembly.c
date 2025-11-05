#include "assembly.h"

#include <stdint.h>

void outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" ::"a"(value), "Nd"(port));
}

void outw(uint16_t port, uint16_t value) {
    asm volatile("outw %0, %1" ::"a"(value), "Nd"(port));
}

void outl(uint16_t port, uint32_t value) {
    asm volatile("outl %0, %1" ::"a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t value;
    asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

uint16_t inw(uint16_t port) {
    uint16_t value;
    asm volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

uint32_t inl(uint16_t port) {
    uint32_t value;
    asm volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void io_wait(void) {
    asm volatile("outb %0, $0x80" ::"a"((uint8_t)0));
}
