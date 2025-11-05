#ifndef USB_CORE_H
#define USB_CORE_H

#include <stdbool.h>
#include <stddef.h>

bool usb_init(void);
size_t usb_bus_count(void);

#endif // USB_CORE_H
