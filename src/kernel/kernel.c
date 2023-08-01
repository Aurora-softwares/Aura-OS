#include <stdint.h>

#define VGA_TEXT_MODE_BUFFER_ADDRESS 0xb8000
#define VGA_LIGHT_GRAY 7

static uint16_t cursor_pos;
static uint16_t cursor_x;
static uint16_t cursor_y;

void kernel_main();

static inline void outb(uint16_t port, uint8_t value) {
	asm volatile("outb %0, %1" ::"a"(value), "Nd"(port));
}
void clearScreen() {
    unsigned char *video = (unsigned char *)VGA_TEXT_MODE_BUFFER_ADDRESS;
    for (int i = 0; i < 2000; i++) {
        *(video++) = ' ';
        *(video++) = VGA_LIGHT_GRAY;
    }
}
void moveCursor(uint8_t x, uint8_t y) {
	cursor_x = x;
	cursor_y = y;
	cursor_pos = y * 80 + x;
	// VGA text mode uses ports 0x3D4 and 0x3D5 to set the cursor position
	// Low byte to port 0x3D4 (cursor low register)
	// High byte to port 0x3D5 (cursor high register)
	// Cursor position is represented by a 16-bit value (80x25 text mode).
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(cursor_pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((cursor_pos >> 8) & 0xFF));
}
static void print(char *str) {
	unsigned char *video = ((unsigned char *)VGA_TEXT_MODE_BUFFER_ADDRESS);
	while (*str != '\0') {
		*(video++) = *str;
		*(video++) = VGA_LIGHT_GRAY;
		str++;
	}
}
void printAtPos(char *str, uint8_t x, uint8_t y) {
	moveCursor(x, y);
	print(str);
}

void kernel_init() {
	clearScreen();
	// Example code: Print a message to the screen at position (0, 0)
	printAtPos("Hello, world!", 5, 10);
	//printAtPos("Welcome to Aura OS! I am currently on version 0.0.1", 0, 0);
	//printAtPos("Screen Resolution is 80x25.", 0, 1);
	//printAtPos("Example Terminal Ideas.", 0, 3);
	//printAtPos("AURA A:>", 0, 4);
	//printAtPos("AURA ALPHA:>", 0, 5);
	//printAtPos("AURA 1:>", 0, 6);
	moveCursor(10, 6);

	kernel_main();
}

void kernel_main() {
	// Call the kernel entry point written in assembly
	while (1);
}