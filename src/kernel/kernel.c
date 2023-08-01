#include <stdint.h>

#define VIDEO_START 0xb8000
#define VGA_LIGHT_GRAY 7

void kernel_init();
void kernel_main();

static void print(char *str) {
  unsigned char *video = ((unsigned char *)VIDEO_START);
  while(*str != '\0') {
    *(video++) = *str++;
    *(video++) = VGA_LIGHT_GRAY;
  }
}

void kernel_init() {

    // Example code: Print a message to the screen
    print(
		"Welcome to Aura OS! I am currently on version 0.0.1                             "
		"Screen Resolution is 80x25.                                                     "
		"                                                                                "
		"Example Terminal Ideas.                                                         "
		"AURA A:>                                                                        "
		"AURA ALPHA:>                                                                    "
		"AURA 1:>                                                                        "
	);
	kernel_main();
}

void kernel_main() {
    // Call the kernel entry point written in assembly
    while (1);
}
