#ifndef USB_UHCI_H
#define USB_UHCI_H

#include <stdbool.h>
#include <stdint.h>

#include "drivers/pci.h"
#include "drivers/usb/usb.h"

#define UHCI_MAX_PORTS 2

struct uhci_controller;

struct uhci_setup_data {
    const struct usb_setup_packet* setup;
    void* data;
    uint16_t length;
    bool direction_in;
};

struct uhci_interrupt_transfer {
    uint8_t endpoint;
    void* buffer;
    uint16_t length;
};

struct uhci_controller* uhci_controller_get(void);
bool uhci_controller_init(const struct pci_device* device);
bool uhci_control_transfer(struct uhci_controller* controller, uint8_t address, const struct uhci_setup_data* transfer);
bool uhci_interrupt_poll(struct uhci_controller* controller, uint8_t address, struct uhci_interrupt_transfer* transfer);
void uhci_set_address(struct uhci_controller* controller, uint8_t address);
void uhci_select_port(struct uhci_controller* controller, uint8_t port_index);
uint8_t uhci_port_count(const struct uhci_controller* controller);
bool uhci_port_connected(const struct uhci_controller* controller, uint8_t port_index);
bool uhci_port_low_speed(const struct uhci_controller* controller, uint8_t port_index);

#endif // USB_UHCI_H
