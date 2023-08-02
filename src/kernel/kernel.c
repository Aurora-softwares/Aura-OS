
#include <stdint.h>
#include <keyboard.h>

#define VGA_TEXT_MODE_BUFFER_ADDRESS 0x000b8000
#define VGA_LIGHT_GRAY 7

static uint16_t cursor_pos;
static uint16_t cursor_x;
static uint16_t cursor_y;

void boot_os();

static inline void outb(uint16_t port, uint8_t value) {
	asm volatile("outb %0, %1" ::"a"(value), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}
static inline void wait_for_input_buffer_empty() {
    uint8_t status;
    do {
        status = inb(0x64);
    } while (status & 0x02);
}
//----- Cursor -----//
void setCursorAppearance() {
    // The ASCII value for underscore is 0x5F
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x0E);
    outb(0x3D4, 0x0B);
    outb(0x3D5, 0x0F);
}

void moveCursor(uint8_t x, uint8_t y) {
    cursor_x = x;
    cursor_y = y;
    cursor_pos = y * 80 + x;

    // Set the cursor appearance to an underscore
    setCursorAppearance();

    // Send commands to VGA text mode ports to update the cursor position
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(cursor_pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((cursor_pos >> 8) & 0xFF));
}

//----- Keyboard controller -----//
void keyboard_init() {
    // Wait for the input buffer to be empty
    wait_for_input_buffer_empty();

    // Send the initialization command (0xAA) to perform a self-test
    outb(0xAA, 0x64);

    // Wait for the self-test to complete
	uint8_t status;
    do {
        status = inb(0x64);
    } while (!(status & 0x01));

    // Check if the self-test was successful
    uint8_t result;
        result = inb(0x60);
    if (result != 0x55) {
        // Handle self-test failure if needed
        // ...

        // Return or take appropriate action
        return;
    }

    // Enable the keyboard interface
    outb(0xAE, 0x64);

    // Set the keyboard LEDs (optional)
    // To set the LEDs, you can send the appropriate command (e.g., 0xED) to the data port (0x60)
    // and then the desired LED state (bit 0 for Scroll Lock, bit 1 for Num Lock, bit 2 for Caps Lock).
    // ...
}
//----- screen -----//
void clearScreen() {
    unsigned char *video = (unsigned char *)VGA_TEXT_MODE_BUFFER_ADDRESS;
    for (int i = 0; i < 2000; i++) {
        *(video++) = ' ';
        *(video++) = VGA_LIGHT_GRAY;
    }
}
static void print(char *str) {
	unsigned char *video = ((unsigned char *)VGA_TEXT_MODE_BUFFER_ADDRESS) + cursor_pos * 2;
	while (*str != '\0') {
		if (*str == '\n') {
            // Move cursor to the beginning of the next line
            cursor_x = 0;
            cursor_y++;
            cursor_pos = cursor_y * 80;
        } else {
            // Print the character and update cursor position
            *(video++) = *str;
            *(video++) = VGA_LIGHT_GRAY;
            cursor_pos++;
            cursor_x++;
            if (cursor_x >= 80) {
                // Move to the next line if the end of the line is reached
                cursor_x = 0;
                cursor_y++;
                cursor_pos = cursor_y * 80;
            }
        }
        str++;
		moveCursor(cursor_x, cursor_y);
	}
}
void printAtPos(char *str, uint8_t x, uint8_t y) {
	moveCursor(x, y);
	print(str);
}
//----- OS Functions -----//
void boot_os() {
	// Call the kernel entry point written in assembly
	while (1);
}

void kernel_init() {
	// Initialize Cursor
	setCursorAppearance();
	// Initialize Keyboard
	keyboard_init();
	//Initialize Screen
	clearScreen();
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