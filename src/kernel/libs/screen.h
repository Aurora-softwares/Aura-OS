#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

void setCursorAppearance(void);
void moveCursor(uint8_t x, uint8_t y);
void clearScreen(void);
void screen_write_char(char c);
void screen_backspace(void);
void print(const char *str);
void printAtPos(const char *str, uint8_t x, uint8_t y);
void screen_set_cell(uint8_t x, uint8_t y, uint16_t value);
uint16_t screen_get_cell(uint8_t x, uint8_t y);

#endif // SCREEN_H
