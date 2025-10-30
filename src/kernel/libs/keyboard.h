#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

enum {
    KEYBOARD_NO_EVENT = -1,
    KEYBOARD_BACKSPACE = 0x08,
};

enum {
    KEYBOARD_FLAG_SHIFT = 0x01,
    KEYBOARD_FLAG_CAPS = 0x02,
};

void keyboard_init(void);
int keyboard_poll_char(void);

#endif // KEYBOARD_H
