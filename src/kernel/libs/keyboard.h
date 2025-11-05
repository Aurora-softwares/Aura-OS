#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>

void keyboard_interrupt_handler(void);
void keyboard_init(void);
char read_char(void);
bool keyboard_try_read_char(char* out);

#endif // KEYBOARD_H
