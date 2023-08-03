#include "keyboard.h"

#include <stdint.h>
#include "assembly.h"

void keyboard_init() {
    // Wait for the input buffer to be empty
    uint8_t status;
    do {
        status = inb(0x64);
    } while (status & 0x02);

    // Send the initialization command (0xAA) to perform a self-test
    outb(0xAA, 0x64);

    // Check if the self-test was successful
    uint8_t result;
        result = inb(0x60);
    if (result != 0x55) {
        // Handle self-test failure if needed
        return;
    }

    // Enable the keyboard interface
    outb(0xAE, 0x64);

    // Set the keyboard LEDs (optional)
    // To set the LEDs, you can send the appropriate command (e.g., 0xED) to the data port (0x60)
    // and then the desired LED state (bit 0 for Scroll Lock, bit 1 for Num Lock, bit 2 for Caps Lock).
    // ...
}