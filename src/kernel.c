void clear_screen() {
    // Video memory begins at address 0xB8000
    char* video_memory = (char*) 0xB8000;
    // Each character on screen is made up of two bytes, a character code and a format code
    // The format code specifies foreground and background colors
    // We want to write a blank character with black background to every position on the screen
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i+1] = 0x0F;
    }
}

void main() {
    clear_screen();
    // Write "Hello" at the top of the screen
    char* hello = "Hello";
    char* video_memory = (char*) 0xB8000;
    for (int i = 0; i < 5; i++) {
        video_memory[i * 2] = hello[i];
        video_memory[i * 2 + 1] = 0x0F;
    }
    // Infinite loop to prevent the kernel from exiting
    while (1) {}
}
