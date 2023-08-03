#include "screen.h"

#include <stdint.h>
#include "assembly.h"

#define VGA_TEXT_MODE_BUFFER_ADDRESS 0x000b8000
#define VGA_LIGHT_GRAY 7

static uint16_t cursor_pos;
static uint16_t cursor_x;
static uint16_t cursor_y;

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
void clearScreen() {
    unsigned char *video = (unsigned char *)VGA_TEXT_MODE_BUFFER_ADDRESS;
    for (int i = 0; i < 2000; i++) {
        *(video++) = ' ';
        *(video++) = VGA_LIGHT_GRAY;
    }
}
void print(char *str) {
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
