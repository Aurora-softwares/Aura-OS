void print_string(const char* str) {
    volatile char* video_memory = (volatile char*)0xb8000;
    while(*str != 0) {
        *video_memory++ = *str++;
        *video_memory++ = 0x07;
    }
}

void main() {
    print_string("Hello, world!");
    while(1);
}
