#include <aura/pci.h>
#include <aura/io.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static uint32_t pci_config_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    return (uint32_t)(1u << 31) | ((uint32_t)bus << 16) | ((uint32_t)device << 11) |
           ((uint32_t)function << 8) | (offset & 0xFC);
}

uint32_t pci_read_config32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    outl(PCI_CONFIG_ADDRESS, pci_config_address(bus, device, function, offset));
    return inl(PCI_CONFIG_DATA);
}

void pci_write_config32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value) {
    outl(PCI_CONFIG_ADDRESS, pci_config_address(bus, device, function, offset));
    outl(PCI_CONFIG_DATA, value);
}

int pci_enumerate(pci_device_t *out, int max_devices) {
    int count = 0;
    for (uint16_t bus = 0; bus < 256 && count < max_devices; ++bus) {
        for (uint8_t device = 0; device < 32 && count < max_devices; ++device) {
            uint32_t vendor = pci_read_config32(bus, device, 0, 0);
            if ((vendor & 0xFFFF) == 0xFFFF) {
                continue;
            }
            uint8_t header_type = (pci_read_config32(bus, device, 0, 0x0C) >> 16) & 0xFF;
            uint8_t functions = (header_type & 0x80) ? 8 : 1;
            for (uint8_t function = 0; function < functions && count < max_devices; ++function) {
                uint32_t vid_did = pci_read_config32(bus, device, function, 0x00);
                if ((vid_did & 0xFFFF) == 0xFFFF) {
                    continue;
                }
                uint32_t class_reg = pci_read_config32(bus, device, function, 0x08);
                out[count].bus = bus;
                out[count].device = device;
                out[count].function = function;
                out[count].vendor_id = vid_did & 0xFFFF;
                out[count].device_id = (vid_did >> 16) & 0xFFFF;
                out[count].class_code = (class_reg >> 24) & 0xFF;
                out[count].subclass = (class_reg >> 16) & 0xFF;
                out[count].prog_if = (class_reg >> 8) & 0xFF;
                count++;
            }
        }
    }
    return count;
}
