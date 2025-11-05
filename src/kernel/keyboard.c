#include "keyboard.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "drivers/usb/usb_keyboard.h"
#include "serial.h"
#include "libs/assembly.h"

#define KEYBOARD_BUFFER_SIZE 128

static char key_buffer[KEYBOARD_BUFFER_SIZE];
static size_t buffer_head = 0;
static size_t buffer_tail = 0;
static bool buffer_full = false;

static bool usb_ready = false;

static bool buffer_is_empty(void) {
    return (!buffer_full && buffer_head == buffer_tail);
}

static void buffer_advance_pointer(void) {
    if (buffer_full) {
        buffer_tail = (buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;
    }

    buffer_head = (buffer_head + 1) % KEYBOARD_BUFFER_SIZE;
    buffer_full = (buffer_head == buffer_tail);
}

static void buffer_recede_pointer(void) {
    buffer_full = false;
    buffer_tail = (buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;
}

static void buffer_push(char value) {
    serial_write("keyboard: buffer_push char=");
    serial_write_hex8((uint8_t)value);
    serial_write("\n");
    key_buffer[buffer_head] = value;
    buffer_advance_pointer();
    serial_write("keyboard: buffer_push head=");
    serial_write_uint((uint32_t)buffer_head);
    serial_write(" tail=");
    serial_write_uint((uint32_t)buffer_tail);
    serial_write(" full=");
    serial_write(buffer_full ? "true\n" : "false\n");
}

static bool buffer_pop(char* out) {
    if (buffer_is_empty()) {
        serial_write("keyboard: buffer_pop empty\n");
        return false;
    }

    *out = key_buffer[buffer_tail];
    buffer_recede_pointer();
    serial_write("keyboard: buffer_pop char=");
    serial_write_hex8((uint8_t)*out);
    serial_write(" new tail=");
    serial_write_uint((uint32_t)buffer_tail);
    serial_write("\n");
    return true;
}

static void ps2_wait_input_clear(void) {
    serial_write("keyboard: ps2_wait_input_clear\n");
    while (inb(0x64) & 0x02) {
        io_wait();
    }
    serial_write("keyboard: ps2_wait_input_clear done\n");
}

static void ps2_flush_output(void) {
    serial_write("keyboard: ps2_flush_output\n");
    while (inb(0x64) & 0x01) {
        (void)inb(0x60);
    }
    serial_write("keyboard: ps2_flush_output done\n");
}

static void mask_pic_irqs(void) {
    outb(0x21, 0xFF); // mask all IRQs on master PIC
    outb(0xA1, 0xFF); // mask all IRQs on slave PIC
}

static void legacy_keyboard_init(void) {
    // We do not have an IDT/IRQ handler yet, so keep the PS/2 controller quiescent.
    mask_pic_irqs();
    serial_write("legacy_keyboard_init: masked PIC interrupts\n");

    ps2_wait_input_clear();
    outb(0xAD, 0x64); // disable first PS/2 port (keyboard)
    serial_write("legacy_keyboard_init: disabled port 1\n");

    ps2_wait_input_clear();
    outb(0xA7, 0x64); // disable second PS/2 port (mouse), avoids stray data
    serial_write("legacy_keyboard_init: disabled port 2\n");

    ps2_flush_output(); // drain any pending controller output
    serial_write("legacy_keyboard_init: flushed controller output\n");
}

static void poll_sources(void) {
    if (usb_ready) {
        char ch;
        if (usb_keyboard_poll_char(&ch)) {
            buffer_push(ch);
            serial_write("keyboard: poll_sources queued USB char\n");
        }
    } else {
        serial_write("keyboard: poll_sources usb not ready\n");
    }
}

void keyboard_interrupt_handler(void) {
    serial_write("keyboard: interrupt handler invoked\n");
    poll_sources();
}

void keyboard_init(void) {
    legacy_keyboard_init();
    serial_write("keyboard_init: starting USB keyboard init\n");
    usb_ready = usb_keyboard_init();
    serial_write(usb_ready ? "keyboard_init: USB keyboard ready\n"
                           : "keyboard_init: USB keyboard init failed\n");
}

bool keyboard_try_read_char(char* out) {
    if (!out) {
        serial_write("keyboard: keyboard_try_read_char null out\n");
        return false;
    }

    poll_sources();
    bool ok = buffer_pop(out);
    serial_write(ok ? "keyboard: keyboard_try_read_char success\n"
                    : "keyboard: keyboard_try_read_char no data\n");
    return ok;
}

char read_char(void) {
    char value;
    serial_write("keyboard: read_char waiting\n");
    while (!keyboard_try_read_char(&value)) {
        poll_sources();
        io_wait();
    }
    serial_write("keyboard: read_char returning char=");
    serial_write_hex8((uint8_t)value);
    serial_write("\n");
    return value;
}
