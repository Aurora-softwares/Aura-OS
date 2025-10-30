#include "screen.h"

#include <stdint.h>

#include "assembly.h"

#define VGA_TEXT_MODE_BUFFER_ADDRESS 0x000B8000
#define VGA_LIGHT_GRAY 0x07

static volatile uint16_t *const vga_buffer = (volatile uint16_t *)VGA_TEXT_MODE_BUFFER_ADDRESS;
static uint16_t cursor_x;
static uint16_t cursor_y;

static void screen_update_cursor(void) {
    uint16_t cursor_pos = cursor_y * SCREEN_WIDTH + cursor_x;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(cursor_pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((cursor_pos >> 8) & 0xFF));
}

static void screen_scroll_if_needed(void) {
    if (cursor_y < SCREEN_HEIGHT) {
        return;
    }

    for (uint16_t y = 1; y < SCREEN_HEIGHT; ++y) {
        for (uint16_t x = 0; x < SCREEN_WIDTH; ++x) {
            vga_buffer[(y - 1) * SCREEN_WIDTH + x] = vga_buffer[y * SCREEN_WIDTH + x];
        }
    }

    for (uint16_t x = 0; x < SCREEN_WIDTH; ++x) {
        vga_buffer[(SCREEN_HEIGHT - 1) * SCREEN_WIDTH + x] = (VGA_LIGHT_GRAY << 8) | ' ';
    }

    cursor_y = SCREEN_HEIGHT - 1;
    if (cursor_x >= SCREEN_WIDTH) {
        cursor_x = SCREEN_WIDTH - 1;
    }
}

void setCursorAppearance(void) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x0E);
    outb(0x3D4, 0x0B);
    outb(0x3D5, 0x0F);
}

void moveCursor(uint8_t x, uint8_t y) {
    cursor_x = x;
    cursor_y = y;
    screen_update_cursor();
}

void clearScreen(void) {
    for (uint16_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) {
        vga_buffer[i] = (VGA_LIGHT_GRAY << 8) | ' ';
    }

    cursor_x = 0;
    cursor_y = 0;
    screen_update_cursor();
}

void screen_set_cell(uint8_t x, uint8_t y, uint16_t value) {
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
        return;
    }

    vga_buffer[y * SCREEN_WIDTH + x] = value;
}

uint16_t screen_get_cell(uint8_t x, uint8_t y) {
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
        return (VGA_LIGHT_GRAY << 8) | ' ';
    }

    return vga_buffer[y * SCREEN_WIDTH + x];
}

static void screen_write_printable(char c) {
    vga_buffer[cursor_y * SCREEN_WIDTH + cursor_x] = (VGA_LIGHT_GRAY << 8) | (uint8_t)c;
    cursor_x++;
    if (cursor_x >= SCREEN_WIDTH) {
        cursor_x = 0;
        cursor_y++;
        screen_scroll_if_needed();
    }
}

void screen_write_char(char c) {
    switch (c) {
        case '\n':
            cursor_x = 0;
            cursor_y++;
            screen_scroll_if_needed();
            break;
        case '\r':
            cursor_x = 0;
            break;
        case '\t': {
            const uint8_t tab_size = 4;
            uint8_t spaces = tab_size - (cursor_x % tab_size);
            for (uint8_t i = 0; i < spaces; ++i) {
                screen_write_printable(' ');
            }
            break;
        }
        default:
            screen_write_printable(c);
            break;
    }

    screen_update_cursor();
}

void screen_backspace(void) {
    if (cursor_x == 0 && cursor_y == 0) {
        return;
    }

    if (cursor_x == 0) {
        cursor_y--;
        cursor_x = SCREEN_WIDTH - 1;
    } else {
        cursor_x--;
    }

    vga_buffer[cursor_y * SCREEN_WIDTH + cursor_x] = (VGA_LIGHT_GRAY << 8) | ' ';
    screen_update_cursor();
}

void print(const char *str) {
    while (*str != '\0') {
        screen_write_char(*str++);
    }
}

void printAtPos(const char *str, uint8_t x, uint8_t y) {
    moveCursor(x, y);
    print(str);
}
