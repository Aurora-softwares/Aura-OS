#include <aura/serial.h>
#include <aura/io.h>

#define COM1_PORT 0x3F8

static int serial_ready = 0;

void serial_init(void) {
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x80);
    outb(COM1_PORT + 0, 0x03);
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x03);
    outb(COM1_PORT + 2, 0xC7);
    outb(COM1_PORT + 4, 0x0B);
    serial_ready = 1;
}

static int serial_is_transmit_empty(void) {
    return inb(COM1_PORT + 5) & 0x20;
}

void serial_write_char(char c) {
    if (!serial_ready) {
        serial_init();
    }
    while (!serial_is_transmit_empty()) {
    }
    outb(COM1_PORT, (uint8_t)c);
}

void serial_write(const char *s) {
    if (!serial_ready) {
        serial_init();
    }
    while (*s) {
        if (*s == '\n') {
            serial_write_char('\r');
        }
        serial_write_char(*s++);
    }
}
