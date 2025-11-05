#include <stdbool.h>
#include <stdint.h>
#include "assembly.h"
#include "serial.h"
#include "screen.h"
#include "drivers/usb/usb_core.h"

bool text_mode = true;

void boot_os() {
	// Call the kernel entry point written in assembly
	serial_write("boot_os: entering main loop\n");
	while (1);
}

void kernel_init() {
	//Initialize Serial
	serial_init();
	serial_write("kernel_init: serial ready\n");

	//Initialize Screen
	serial_write("kernel_init: initializing Screen...\n");
	screen_init();
	serial_write("kernel_init: screen initialized\n");

	// Initialize Cursor
	serial_write("kernel_init: initializing Cursor...\n");
	setCursorAppearance();
	serial_write("kernel_init: Cursor initialized\n");

	// Scan USB topology
	serial_write("kernel_init: scanning USB controllers...\n");
	bool usb_ok = usb_init();
	serial_write(usb_ok ? "kernel_init: USB scan completed\n"
	                    : "kernel_init: USB scan unavailable\n");
	serial_write("kernel_init: USB bus count=");
	serial_write_uint(usb_bus_count());
	serial_write("\n");

	// Initialize HDMI
	//serial_write("kernel_init: initializing HDMI...\n");
	//hdmi_init();
	//serial_write("kernel_init: HDMI initialized\n");
	
	if(text_mode) {
		serial_write("kernel_init: Booting Text Mode\n");

		// Print welcome message
		serial_write("kernel_init: printing welcome passage\n");
		printAtPos("Welcome to Aura OS! I am currently on version 0.0.1", 0, 0);
		printAtPos("Screen Resolution is 80x25.", 0, 1);
		printAtPos("Example Terminal Ideas.", 0, 3);
		printAtPos("AURA A:>", 0, 4);
		printAtPos("AURA ALPHA:>", 0, 5);
		printAtPos("AURA 1:>", 0, 6);
	} else {
		// Boot OS
		serial_write("kernel_init: Booting OS\n");
		boot_os();
	}
}
