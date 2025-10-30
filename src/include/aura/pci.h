#ifndef AURA_PCI_H
#define AURA_PCI_H

#include <stdint.h>

typedef struct {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;
} pci_device_t;

int pci_enumerate(pci_device_t *out, int max_devices);
uint32_t pci_read_config32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
void pci_write_config32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);

#endif
