#include "screen.h"

#include <stdint.h>
#include "assembly.h"
#include "serial.h"

#define VGA_TEXT_MODE_BUFFER_ADDRESS 0x000b8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_LIGHT_GRAY 7

static uint16_t cursor_pos;
static uint16_t cursor_x;
static uint16_t cursor_y;

void screen_init() {
    cursor_x = 0;
    cursor_y = 0;
    cursor_pos = 0;
	clearScreen();
}

void clearScreen() {
    serial_write("screen: clearScreen start\n");
    volatile uint16_t *video = (volatile uint16_t *)VGA_TEXT_MODE_BUFFER_ADDRESS;
    const uint16_t blank = ((uint16_t)VGA_LIGHT_GRAY << 8) | ' ';

    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video[i] = blank;
    }

    cursor_x = 0;
    cursor_y = 0;
    cursor_pos = 0;
    moveCursor(0, 0);
    serial_write("screen: clearScreen complete\n");
}

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
    cursor_pos = y * VGA_WIDTH + x;

    // Set the cursor appearance to an underscore
    setCursorAppearance();

    // Send commands to VGA text mode ports to update the cursor position
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(cursor_pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((cursor_pos >> 8) & 0xFF));
}
void print(const char *str) {
    serial_write("screen: print start\n");
    volatile uint16_t *video = (volatile uint16_t *)VGA_TEXT_MODE_BUFFER_ADDRESS;
    const uint16_t attribute = ((uint16_t)VGA_LIGHT_GRAY << 8);

    while (*str != '\0') {
        if (*str == '\n') {
            cursor_x = 0;
            if (cursor_y < VGA_HEIGHT - 1) {
                cursor_y++;
            }
            cursor_pos = cursor_y * VGA_WIDTH;
        } else {
            video[cursor_pos] = attribute | (uint8_t)(*str);
            cursor_pos++;
            cursor_x++;

            if (cursor_x >= VGA_WIDTH) {
                cursor_x = 0;
                if (cursor_y < VGA_HEIGHT - 1) {
                    cursor_y++;
                }
                cursor_pos = cursor_y * VGA_WIDTH;
            }
        }
        str++;
        moveCursor(cursor_x, cursor_y);
    }
    serial_write("screen: print end\n");
}
void printAtPos(const char *str, uint8_t x, uint8_t y) {
	serial_write("screen: printAtPos -> x=");
	serial_write_uint(x);
	serial_write(", y=");
	serial_write_uint(y);
	serial_write("\n");
	moveCursor(x, y);
	print(str);
	serial_write("screen: printAtPos complete\n");
}
