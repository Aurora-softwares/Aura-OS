#define VIDEO_START 0xb8000
#define VGA_LIGHT_GRAY 7

static void print(char *str) {
  unsigned char *video = ((unsigned char *)VIDEO_START);
  while(*str != '\0') {
    *(video++) = *str++;
    *(video++) = VGA_LIGHT_GRAY;
  }
}

void kernel_init() {
    // Initialize the Interrupt Descriptor Table (IDT)
    //init_idt();

    // Initialize keyboard handling
    //init_keyboard();

    // Your OS logic and code goes here

    // Example code: Print a message to the screen
    print("Welcome to Aura OS! I am currently on version 0.0.1 - This is just a test.");

    // Endless loop to halt the system
    while (1);
}

void kernel_main() {
    // Call the kernel entry point written in assembly
}
