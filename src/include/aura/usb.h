#ifndef AURA_USB_H
#define AURA_USB_H

#include <stdint.h>
#include <aura/mouse.h>

typedef enum {
    USB_DEVICE_NONE = 0,
    USB_DEVICE_KEYBOARD,
    USB_DEVICE_MOUSE,
} usb_device_type_t;

typedef struct {
    usb_device_type_t type;
    uint8_t address;
    uint8_t interface_number;
    uint8_t endpoint_in;
    uint8_t endpoint_out;
} usb_device_t;

void usb_init(void);
const usb_device_t *usb_find_keyboard(void);
const usb_device_t *usb_find_mouse(void);
int usb_poll_keyboard(char *ch);
int usb_poll_mouse(mouse_packet_t *packet);

#endif
