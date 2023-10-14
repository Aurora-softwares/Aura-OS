#include "assembly.h"

#include <stdint.h>

uint8_t inb(uint16_t port) {
    uint8_t value;
    asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}
void outb(uint16_t port, uint8_t value) {
	asm volatile("outb %0, %1" ::"a"(value), "Nd"(port));
}
// Read a 32-bit value from the specified I/O port
uint32_t inl(uint16_t port) {
    uint32_t value;
    asm volatile("inl %w1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// Write a 32-bit value to the specified I/O port
void outl(uint16_t port, uint32_t value) {
    asm volatile("outl %0, %w1" : : "a"(value), "Nd"(port));
}

void lidt(void *base, uint16_t limit) {
    struct {
        uint16_t limit;
        void* base;
    } __attribute__((packed)) idt_descriptor = { limit, base };

    // Inline assembly to load the IDT using lidt instruction
    asm volatile("lidt %0" : : "m"(idt_descriptor));
}
