#include <stdint.h>
#include "assembly.h"
#include "keyboard.h"
#include "screen.h"
#include "hid.h"

void boot_os() {
	clearScreen();
	printAtPos("Welcome to Aura OS! I am currently on version 0.0.1", 0, 0);
	printAtPos("Screen Resolution is 80x25.", 0, 1);
	printAtPos("Example Terminal Ideas.", 0, 3);
	printAtPos("AURA A:>", 0, 4);
	printAtPos("AURA ALPHA:>", 0, 5);
	printAtPos("AURA 1:>", 0, 6);
	while (1);
}

void kernel_init() {
	clearScreen();
	moveCursor(0, 0);

	print("Initializing HID controller.\n");
	pci_address_t xhci_controller_address;
    // Detect xHCI USB host controller
    if (detect_xhci_usb_host_controller(&xhci_controller_address)) {
        // Initialize the xHCI controller
        xhci_init(xhci_controller_address);

        // Enumerate USB devices and identify HID devices
        usb_enumerate_devices(xhci_controller_address);
    } else {
		print("Unable to find xHCI controller\n");
    }

	print("Setting cursor appearance.\n");
	setCursorAppearance();

	print("Initializing keyboard.\n");
	keyboard_init();

	//boot_os();
}