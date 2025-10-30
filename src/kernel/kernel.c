#include <stdint.h>

#include "cli.h"
#include "keyboard.h"
#include "mouse.h"
#include "screen.h"

void kernel_init(void) {
    clearScreen();
    moveCursor(0, 0);
    setCursorAppearance();

    keyboard_init();
    mouse_init();

    cli_run();
}

void boot_os(void) {
    clearScreen();
    printAtPos("Welcome to Aura OS! I am currently on version 0.0.1", 0, 0);
    printAtPos("Screen Resolution is 80x25.", 0, 1);
    printAtPos("Example Terminal Ideas.", 0, 3);
    printAtPos("AURA A:>", 0, 4);
    printAtPos("AURA ALPHA:>", 0, 5);
    printAtPos("AURA 1:>", 0, 6);

    while (1) {
    }
}
