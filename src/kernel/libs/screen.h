#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>
#include "keyboard.h"
#include "assembly.h"

void setCursorAppearance();
void moveCursor(uint8_t x, uint8_t y);
void clearScreen();
void print(const char *str);
void printAtPos(const char *str, uint8_t x, uint8_t y);
void screen_init();

#endif // SCREEN_H
