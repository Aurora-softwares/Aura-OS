#ifndef HID_H
#define HID_H

#include <stdbool.h>
#include <stdint.h>
#include "assembly.h"
#include "screen.h"

#define XHCI_PORTSC_CONNECTED_MASK	0x00000001
#define XHCI_PORTSC_SPEED_MASK		0x00000C00
#define XHCI_PORTSC_SPEED_SUPER		0x00000400
#define MAX_USB_PORTS				8

// Structure for the PCI Configuration Space Header
struct pci_config_space {
    uint16_t vendor_id;
    uint16_t device_id;
    // Add other fields as needed
};

typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t protocol;
} pci_device_t;

typedef struct {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
} pci_address_t;

// Function to read a 32-bit word from the PCI configuration space
uint32_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

// Function to read a byte from the PCI configuration space
uint8_t pci_config_read_byte(pci_address_t address, uint8_t offset);

// Function to check if the device is an xHCI USB host controller
int is_xhci_controller(uint8_t bus, uint8_t slot, uint8_t func);

// Function to detect if an xHCI USB host controller is present
bool detect_xhci_usb_host_controller(pci_address_t *controller_address);

// Function to initialize the USB host controller
void init_usb_host_controller();

// Function to initialize the xHCI controller
void xhci_init(pci_address_t controller_address);

// Function to enumerate USB devices and identify HID devices
void usb_enumerate_devices(pci_address_t controller_address);

#endif /* HID_H */
