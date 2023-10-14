#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdio.h>
#include "assembly.h"
#include "screen.h"

void keyboard_init();
void keyboard_interrupt_handler();

#endif // KEYBOARD_H
