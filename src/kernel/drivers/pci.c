#include "pci.h"

#include "assembly.h"
#include "serial.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define PCI_ENABLE_BIT 0x80000000

static uint32_t pci_config_address(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    return (uint32_t)(PCI_ENABLE_BIT |
                      ((uint32_t)bus << 16) |
                      ((uint32_t)slot << 11) |
                      ((uint32_t)function << 8) |
                      (offset & 0xFC));
}

uint32_t pci_config_read32(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    outl(PCI_CONFIG_ADDRESS, pci_config_address(bus, slot, function, offset));
    return inl(PCI_CONFIG_DATA);
}

uint16_t pci_config_read16(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    uint32_t value = pci_config_read32(bus, slot, function, offset);
    return (uint16_t)((value >> ((offset & 2) * 8)) & 0xFFFF);
}

uint8_t pci_config_read8(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    uint32_t value = pci_config_read32(bus, slot, function, offset);
    return (uint8_t)((value >> ((offset & 3) * 8)) & 0xFF);
}

void pci_config_write16(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint16_t value) {
    uint32_t address = pci_config_address(bus, slot, function, offset);
    outl(PCI_CONFIG_ADDRESS, address);
    uint32_t current = inl(PCI_CONFIG_DATA);
    uint32_t shift = (offset & 2) * 8;
    current &= ~(0xFFFFu << shift);
    current |= ((uint32_t)value << shift);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, current);
}

static bool pci_device_present(uint8_t bus, uint8_t slot, uint8_t function) {
    uint16_t vendor = pci_config_read16(bus, slot, function, 0x00);
    return vendor != 0xFFFF;
}

bool pci_find_by_class(uint8_t class_code, uint8_t subclass, uint8_t prog_if, struct pci_device* out_device) {
    serial_write("pci: find_by_class start\n");
    for (uint16_t bus = 0; bus < 256; ++bus) {
        uint8_t bus_index = (uint8_t)bus;
        for (uint8_t slot = 0; slot < 32; ++slot) {
            if (!pci_device_present(bus_index, slot, 0)) {
                continue;
            }

            uint8_t header_type = pci_config_read8(bus_index, slot, 0, 0x0E);
            uint8_t function_count = (header_type & 0x80) ? 8 : 1;

            for (uint8_t function = 0; function < function_count; ++function) {
                if (!pci_device_present(bus_index, slot, function)) {
                    continue;
                }

                uint8_t cc = pci_config_read8(bus_index, slot, function, 0x0B);
                uint8_t sc = pci_config_read8(bus_index, slot, function, 0x0A);
                uint8_t pi = pci_config_read8(bus_index, slot, function, 0x09);
                if (cc == class_code && sc == subclass && (prog_if == 0xFF || pi == prog_if)) {
                    serial_write("pci: match bus=");
                    serial_write_uint(bus_index);
                    serial_write(" slot=");
                    serial_write_uint(slot);
                    serial_write(" func=");
                    serial_write_uint(function);
                    serial_write("\n");
                    if (out_device) {
                        out_device->bus = bus_index;
                        out_device->slot = slot;
                        out_device->function = function;
                        out_device->vendor_id = pci_config_read16(bus_index, slot, function, 0x00);
                        out_device->device_id = pci_config_read16(bus_index, slot, function, 0x02);
                        out_device->class_code = cc;
                        out_device->subclass = sc;
                        out_device->prog_if = pi;
                    }
                    serial_write("pci: find_by_class success\n");
                    return true;
                }
            }
        }
    }

    serial_write("pci: find_by_class no match\n");
    return false;
}
