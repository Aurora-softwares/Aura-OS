#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include "keyboard.h"
#include "assembly.h"

void keyboard_interrupt_handler();
void keyboard_init();
char read_char();

#endif // KEYBOARD_H
