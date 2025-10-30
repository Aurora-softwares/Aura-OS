#include "keyboard.h"

#include <stdint.h>

#include "assembly.h"

#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_DATA_PORT 0x60

#define KEYBOARD_STATUS_OUTPUT_BUFFER 0x01
#define KEYBOARD_STATUS_INPUT_BUFFER 0x02

static int keyboard_wait_for_output(void) {
    for (uint32_t timeout = 0; timeout < 100000; ++timeout) {
        if (inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_OUTPUT_BUFFER) {
            return 1;
        }
    }
    return 0;
}

static void keyboard_wait_for_input_clear(void) {
    while (inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_INPUT_BUFFER) {
    }
}

static void keyboard_flush_output(void) {
    while (inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_OUTPUT_BUFFER) {
        (void)inb(KEYBOARD_DATA_PORT);
    }
}

void keyboard_init(void) {
    keyboard_wait_for_input_clear();
    outb(KEYBOARD_STATUS_PORT, 0xAD); // Disable first PS/2 port
    keyboard_wait_for_input_clear();
    outb(KEYBOARD_STATUS_PORT, 0xA7); // Disable second PS/2 port

    keyboard_flush_output();

    keyboard_wait_for_input_clear();
    outb(KEYBOARD_STATUS_PORT, 0xAA); // Controller self-test
    if (keyboard_wait_for_output()) {
        (void)inb(KEYBOARD_DATA_PORT);
    }

    keyboard_wait_for_input_clear();
    outb(KEYBOARD_STATUS_PORT, 0xAE); // Enable first port
}

enum {
    KEYBOARD_SCANCODE_RELEASE_MASK = 0x80,
    KEYBOARD_SCANCODE_EXTENDED = 0xE0,
};

static const char scancode_set[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const char scancode_set_shift[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static uint8_t keyboard_flags;

int keyboard_poll_char(void) {
    if (!(inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_OUTPUT_BUFFER)) {
        return KEYBOARD_NO_EVENT;
    }

    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    if (scancode == KEYBOARD_SCANCODE_EXTENDED) {
        return KEYBOARD_NO_EVENT;
    }

    uint8_t keycode = scancode & 0x7F;
    if (scancode & KEYBOARD_SCANCODE_RELEASE_MASK) {
        if (keycode == 0x2A || keycode == 0x36) {
            keyboard_flags &= ~KEYBOARD_FLAG_SHIFT;
        }
        return KEYBOARD_NO_EVENT;
    }

    switch (keycode) {
        case 0x2A: // Left Shift
        case 0x36: // Right Shift
            keyboard_flags |= KEYBOARD_FLAG_SHIFT;
            return KEYBOARD_NO_EVENT;
        case 0x3A: // Caps Lock
            keyboard_flags ^= KEYBOARD_FLAG_CAPS;
            return KEYBOARD_NO_EVENT;
        default:
            break;
    }

    char ch = scancode_set[keycode];
    if (keyboard_flags & KEYBOARD_FLAG_SHIFT) {
        char shifted = scancode_set_shift[keycode];
        if (shifted != 0) {
            ch = shifted;
        }
    }

    if (keyboard_flags & KEYBOARD_FLAG_CAPS) {
        if (ch >= 'a' && ch <= 'z') {
            if ((keyboard_flags & KEYBOARD_FLAG_SHIFT) == 0) {
                ch = (char)(ch - ('a' - 'A'));
            }
        } else if (ch >= 'A' && ch <= 'Z') {
            if (keyboard_flags & KEYBOARD_FLAG_SHIFT) {
                ch = (char)(ch + ('a' - 'A'));
            }
        }
    }

    if (ch == 0) {
        if (keycode == 0x0E) {
            return KEYBOARD_BACKSPACE;
        }
        return KEYBOARD_NO_EVENT;
    }

    return (int)ch;
}

