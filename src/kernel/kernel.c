#include <stdint.h>
#include "assembly.h"
#include "keyboard.h"
#include "screen.h"

void boot_os() {
	// Call the kernel entry point written in assembly
	while (1);
}

void kernel_init() {
	//Initialize Screen
	clearScreen();
	// Initialize Cursor
	setCursorAppearance();
	// Initialize Keyboard
	keyboard_init();
	// Print welcome message
	printAtPos("Welcome to Aura OS! I am currently on version 0.0.1", 0, 0);
	printAtPos("Screen Resolution is 80x25.", 0, 1);
	printAtPos("Example Terminal Ideas.", 0, 3);
	printAtPos("AURA A:>", 0, 4);
	printAtPos("AURA ALPHA:>", 0, 5);
	printAtPos("AURA 1:>", 0, 6);
	// Boot OS
	boot_os();
}