#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>
#include "keyboard.h"
#include "assembly.h"

void setCursorAppearance();
void moveCursor(uint8_t x, uint8_t y);
void clearScreen();
void print(char *str);
void printAtPos(char *str, uint8_t x, uint8_t y);

#endif // SCREEN_H
