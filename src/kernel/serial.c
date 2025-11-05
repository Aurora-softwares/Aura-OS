#include "serial.h"

#include <stdint.h>
#include "assembly.h"

#define COM1_PORT 0x3F8

static int serial_initialized;

static void serial_write_hex_nibble(uint8_t value) {
    static const char hex_digits[] = "0123456789ABCDEF";
    serial_write_char(hex_digits[value & 0x0F]);
}

static int serial_is_ready(void) {
    return inb(COM1_PORT + 5) & 0x20;
}

void serial_init(void) {
    outb(COM1_PORT + 1, 0x00); // Disable all interrupts
    outb(COM1_PORT + 3, 0x80); // Enable DLAB
    outb(COM1_PORT + 0, 0x03); // Divisor low byte (38400 baud)
    outb(COM1_PORT + 1, 0x00); // Divisor high byte
    outb(COM1_PORT + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7); // Enable FIFO, clear queues, 14-byte threshold
    outb(COM1_PORT + 4, 0x0B); // IRQs disabled, RTS/DSR set
    serial_initialized = 1;
}

void serial_write_char(char c) {
    if (!serial_initialized) {
        return;
    }
    if (c == '\n') {
        serial_write_char('\r');
    }
    while (!serial_is_ready()) {
        io_wait();
    }
    outb(COM1_PORT, (uint8_t)c);
}

void serial_write(const char *str) {
    if (!serial_initialized || !str) {
        return;
    }
    while (*str) {
        serial_write_char(*str++);
    }
}

void serial_write_hex8(uint8_t value) {
    if (!serial_initialized) {
        return;
    }
    serial_write("0x");
    serial_write_hex_nibble(value >> 4);
    serial_write_hex_nibble(value);
}

void serial_write_hex16(uint16_t value) {
    if (!serial_initialized) {
        return;
    }
    serial_write("0x");
    serial_write_hex_nibble(value >> 12);
    serial_write_hex_nibble(value >> 8);
    serial_write_hex_nibble(value >> 4);
    serial_write_hex_nibble(value);
}

void serial_write_hex32(uint32_t value) {
    if (!serial_initialized) {
        return;
    }
    serial_write("0x");
    for (int shift = 28; shift >= 0; shift -= 4) {
        serial_write_hex_nibble((uint8_t)(value >> shift));
    }
}

void serial_write_uint(uint32_t value) {
    if (!serial_initialized) {
        return;
    }
    char buffer[12];
    int pos = 0;
    if (value == 0) {
        serial_write_char('0');
        return;
    }
    while (value > 0 && pos < (int)sizeof(buffer)) {
        buffer[pos++] = (char)('0' + (value % 10));
        value /= 10;
    }
    for (int i = pos - 1; i >= 0; --i) {
        serial_write_char(buffer[i]);
    }
}
