#ifndef USB_KEYBOARD_H
#define USB_KEYBOARD_H

#include <stdbool.h>
#include <stdint.h>

bool usb_keyboard_init(void);
bool usb_keyboard_poll_char(char* out_char);

#endif // USB_KEYBOARD_H
