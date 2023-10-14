#include "hid.h"

// Function to read a 32-bit word from the PCI configuration space
uint32_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
	uint32_t address = 0x80000000 | (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC);
	outl(0xCF8, address);
	return inl(0xCFC);
}

// Function to read a byte from the PCI configuration space
uint8_t pci_config_read_byte(pci_address_t address, uint8_t offset) {
	uint32_t config_address = 0x80000000 | (address.bus << 16) | (address.device << 11) | (address.function << 8) | offset;
	outl(0xCF8, config_address);
	return (inl(0xCFC) >> ((offset & 3) * 8)) & 0xFF;
}

// Function to check if the device is an xHCI USB host controller
int is_xhci_controller(uint8_t bus, uint8_t slot, uint8_t func) {
	uint32_t header = pci_config_read_word(bus, slot, func, 0);
	struct pci_config_space *config_space = (struct pci_config_space *)&header;

	// Check if the Vendor ID and Device ID match the xHCI controller
	if (config_space->vendor_id == 0x8086 && config_space->device_id == 0x1E31) {
		return 1; // It's an xHCI controller
	}

	return 0; // Not an xHCI controller
}

// Function to check if the port is connected
bool xhci_is_port_connected(pci_address_t controller_address, uint8_t port_number) {
    // Read the Port Status and Control Register (PORTSC) for the specified port
    uint32_t portsc = pci_config_read_word(controller_address.bus, controller_address.device, controller_address.function, 0x400 + port_number * 4);

    // Check the CONNECTED bit (Bit 0) of the PORTSC register
    return (portsc & XHCI_PORTSC_CONNECTED_MASK) != 0;
}

// Function to check the speed of the port
uint8_t xhci_get_port_speed(pci_address_t controller_address, uint8_t port_number) {
    // Read the Port Status and Control Register (PORTSC) for the specified port
    uint32_t portsc = pci_config_read_word(controller_address.bus, controller_address.device, controller_address.function, 0x400 + port_number * 4);

    // Extract the speed bits (Bit 10 and 11) from the PORTSC register
    uint16_t speed_bits = (portsc & XHCI_PORTSC_SPEED_MASK) >> 10;

    // Map the speed bits to the actual speed value
    if (speed_bits == XHCI_PORTSC_SPEED_SUPER) {
        return 3; // SuperSpeed (5 Gbps)
    } else {
        // You can add other speed mappings here based on the xHCI specification
        return 0; // Unknown or other speed
    }
}

// Function to detect xHCI USB host controller
bool detect_xhci_usb_host_controller(pci_address_t *controller_address) {
	// Search for the xHCI controller on the PCI bus
	for (uint16_t bus = 0; bus < 256; bus++) {
		for (uint8_t device = 0; device < 32; device++) {
			for (uint8_t function = 0; function < 8; function++) {
				pci_address_t address = {bus, device, function};
				pci_device_t device_info;

				device_info.vendor_id = pci_config_read_byte(address, 0x00) | (pci_config_read_byte(address, 0x01) << 8);
				device_info.device_id = pci_config_read_byte(address, 0x02) | (pci_config_read_byte(address, 0x03) << 8);
				device_info.class_code = pci_config_read_byte(address, 0x0B);
				device_info.subclass = pci_config_read_byte(address, 0x0A);
				device_info.protocol = pci_config_read_byte(address, 0x09);

				// Check if the device is an xHCI controller
				if (device_info.vendor_id == 0x8086 && device_info.class_code == 0x0C && device_info.subclass == 0x03 && device_info.protocol == 0x30) {
					*controller_address = address;
					return true;
				}
			}
		}
	}
	return false;
}

void init_usb_host_controller() {
	// Initialize the PCI bus or use a library that already provides PCI enumeration.

	// Detect and initialize the xHCI USB host controller.
	//detect_xhci_usb_host_controller();
}

// Function to initialize the xHCI controller
void xhci_init(pci_address_t controller_address) {
	// Read the BAR (Base Address Register) to get the base address of xHCI registers
	uint32_t bar_value = pci_config_read_word(controller_address.bus, controller_address.device, controller_address.function, 0x10);

	// Extract the base address from the BAR value (bits 0-3 are reserved)
	uint32_t xhci_base_address = bar_value & 0xFFFFFFF0;

	// Perform necessary initialization steps for the xHCI controller
	// For example, you might need to:
	// 1. Enable xHCI controller by setting the Run/Stop bit in the command register
	// 2. Set up memory structures like command ring, transfer ring, etc.
	// 3. Enable interrupts for xHCI

	// For this example, let's just print the base address of the xHCI registers
	print("xHCI Base Address: ");
	char str[17]; // Buffer to hold the hexadecimal representation of the base address
	snprintf(str, sizeof(str), "%016lX", (unsigned long)xhci_base_address);
	print(str);
}

// Function to enumerate USB devices and identify HID devices
void usb_enumerate_devices(pci_address_t controller_address) {
    // Initialize the xHCI controller (assuming xhci_init is already defined and implemented)
    xhci_init(controller_address);

    // You need to perform the necessary xHCI controller setup and configuration here
    // to enable USB ports, set up data structures (transfer rings, event rings, etc.),
    // and enable interrupts for USB events.

    // In this example, we are simply printing information about connected devices.
    for (uint8_t port = 0; port < MAX_USB_PORTS; port++) {
        // Check if the port is connected (status bit 0 is set)
        if (xhci_is_port_connected(controller_address, port)) {
            // Check the speed of the connected device
            uint16_t speed = xhci_get_port_speed(controller_address, port);

            // Print information about the connected device
            print("Connected USB device on Port ");
            char port_num_str[3];
            snprintf(port_num_str, sizeof(port_num_str), "%u", port + 1);
            print(port_num_str);

            if (speed == XHCI_PORTSC_SPEED_SUPER) {
                print(" (SuperSpeed USB 3.0)");
            } else {
                print(" (High-Speed USB 2.0)");
            }

            print("\n");

            // At this point, you can proceed with identifying the device,
            // configuring endpoints, and communicating with the HID device if it's one.
            // This involves parsing USB descriptors and interacting with the device
            // according to the USB specification.

            // For simplicity, we are not performing device enumeration and HID communication here.
        }
    }
}
