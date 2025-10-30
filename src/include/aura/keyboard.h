#ifndef AURA_KEYBOARD_H
#define AURA_KEYBOARD_H

#include <stdint.h>

void keyboard_init(void);
int keyboard_read_char(char *out);

#endif
