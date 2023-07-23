#include "kernel.h"

void kernel_main() {
    // This is where you can add more functionality to your OS
    // For this example, we'll just print a message to the screen
    char* message = "Welcome to My OS!";
    print_string(message);
}

void print_string(const char* str) {
    // Here, you need to implement the code to print the string to the screen
    // This could involve interacting with the hardware (e.g., VGA) directly or using BIOS interrupts.
    // However, for simplicity, we'll leave this function empty for now.
}
