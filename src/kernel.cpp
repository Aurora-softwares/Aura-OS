#include <stdint.h>

static inline __attribute__((always_inline)) void bios_backspace();
static inline __attribute__((always_inline)) void bios_newline();
static inline __attribute__((always_inline)) char bios_read_char();
static inline __attribute__((always_inline)) void clear_screen();
static inline __attribute__((always_inline)) void print_char(char c);
static inline __attribute__((always_inline)) void print(const char* str);
static inline __attribute__((always_inline)) void print_welcome();
static inline __attribute__((always_inline)) void print_prompt();
static inline __attribute__((always_inline)) void print_bar();
static inline __attribute__((always_inline)) void print_unknown();

static inline __attribute__((always_inline)) void debug_outb(uint16_t port, uint8_t val);
static inline __attribute__((always_inline)) uint8_t debug_inb(uint16_t port);
static inline __attribute__((always_inline)) void debug_init();
static inline __attribute__((always_inline)) int debug_can_tx();
static inline __attribute__((always_inline)) void debug_putc(char c);
static inline void debug_write(const void* buf, unsigned len);
static inline void debug_puts(const char* s);
static inline __attribute__((always_inline)) void debug_hexdump(const void* buf, unsigned len);

extern "C" void kernel_main() {
    debug_init();

    debug_puts("{before}\n");




    clear_screen();
    print_welcome();

    for (;;) {
        char buffer[64];
        int length = 0;
        print_prompt();

        for (;;) {
            char c = bios_read_char();

            if (c == '\r') {
                bios_newline();
                break;
            }

            if (c == '\b') {
                if (length > 0) {
                    length--;
                    bios_backspace();
                }
                continue;
            }

            if (c >= ' ' && length < 63) {
                buffer[length++] = c;
                print_char(c);
            }
        }

        buffer[length] = '\0';

    }
}

static inline void bios_backspace() {
    print_char('\b');
    print_char(' ');
    print_char('\b');
}

static inline void bios_newline() {
    print_char('\r');
    print_char('\n');
}

static inline char bios_read_char() {
    uint16_t ax;
    asm volatile (
        "xor %%ah, %%ah\n"
        "int $0x16\n"
        : "=a"(ax)
        :
        : "cc"
    );
    return static_cast<char>(ax & 0x00FF);
}

static inline void clear_screen() {
    asm volatile (
        "int $0x10\n"
        :
        : "a"(static_cast<uint16_t>(0x0600)),
          "b"(static_cast<uint16_t>(0x0700)),
          "c"(static_cast<uint16_t>(0x0000)),
          "d"(static_cast<uint16_t>(0x184F))
        : "cc"
    );
    asm volatile (
        "int $0x10\n"
        :
        : "a"(static_cast<uint16_t>(0x0200)),
          "b"(static_cast<uint16_t>(0x0000)),
          "d"(static_cast<uint16_t>(0x0000))
        : "cc"
    );
}

static inline void print_char(char c) {
    asm volatile (
        "int $0x10\n"
        :
        : "a"(static_cast<uint16_t>(0x0E00 | static_cast<uint8_t>(c))),
          "b"(static_cast<uint16_t>(0x0007))
        : "cc"
    );
}

static inline void print(const char* str) {
    while (*str) {
        print_char(*str++);
    }
}

static inline void print_welcome() {
    print_char('W');
    print_char('e');
    print_char('l');
    print_char('c');
    print_char('o');
    print_char('m');
    print_char('e');
    print_char(' ');
    print_char('t');
    print_char('o');
    print_char(' ');
    print_char('t');
    print_char('h');
    print_char('e');
    print_char(' ');
    print_char('k');
    print_char('e');
    print_char('r');
    print_char('n');
    print_char('e');
    print_char('l');
    bios_newline();
    print_char('e');
    print_char('n');
    print_char('t');
    print_char('e');
    print_char('r');
    print_char(' ');
    print_char('\'');
    print_char('h');
    print_char('e');
    print_char('l');
    print_char('p');
    print_char('\'');
    print_char(' ');
    print_char('f');
    print_char('o');
    print_char('r');
    print_char(' ');
    print_char('a');
    print_char('s');
    print_char('s');
    print_char('i');
    print_char('s');
    print_char('t');
    print_char('a');
    print_char('n');
    print_char('c');
    print_char('e');
    bios_newline();
}

static inline void print_prompt() {
    print_char('>');
    print_char(' ');
}

static inline void print_bar() {
    print_char('b');
    print_char('a');
    print_char('r');
    bios_newline();
}

static inline void print_unknown() {
    print_char('U');
    print_char('n');
    print_char('k');
    print_char('n');
    print_char('o');
    print_char('w');
    print_char('n');
    print_char(' ');
    print_char('c');
    print_char('o');
    print_char('m');
    print_char('m');
    print_char('a');
    print_char('n');
    print_char('d');
    bios_newline();
}

////////// QEMU debug output ////////////

#define COM1 0x3F8

static inline void debug_outb(uint16_t port, uint8_t val){
    asm volatile("outb %0,%1"::"a"(val),"Nd"(port));
}

static inline uint8_t debug_inb(uint16_t port){
    uint8_t r;
    asm volatile("inb %1,%0":"=a"(r):"Nd"(port));
    return r;
}

static inline void debug_init() {
    debug_outb(COM1 + 1, 0x00);    // Disable IRQs
    debug_outb(COM1 + 3, 0x80);    // Enable DLAB
    debug_outb(COM1 + 0, 0x03);    // Divisor low  (115200/3 = 38400 baud)
    debug_outb(COM1 + 1, 0x00);    // Divisor high
    debug_outb(COM1 + 3, 0x03);    // 8N1, clear DLAB
    debug_outb(COM1 + 2, 0xC7);    // FIFO on, clear, 14-byte threshold
    debug_outb(COM1 + 4, 0x0B);    // OUT2|RTS|DTR
}

static inline int debug_can_tx() {
    return debug_inb(COM1 + 5) & 0x20;
}

static inline void debug_putc(char c){
    while (!(debug_inb(COM1 + 5) & 0x20)) {}
    debug_outb(COM1, (uint8_t)c);
}

static inline void debug_puts(const char* s){
    if (!s) return;
    while (*s) {
        char c = *s++;
        if (c == '\n') debug_putc('\r');
        debug_putc(c);
    }
}

static inline void debug_write(const void* buf, unsigned len){
    const char* p = (const char*)buf;
    for (unsigned i=0; i<len; ++i) {
        char c = p[i];
        if (c == '\n') debug_putc('\r');
        debug_putc(c);
    }
}

static inline void debug_hexdump(const void* buf, unsigned len){
    static const char HEX[] = "0123456789ABCDEF";
    const unsigned char* p = (const unsigned char*)buf;

    debug_puts("<HEX:");
    for (unsigned i = 0; i < len; ++i) {
        char out[3] = { HEX[p[i] >> 4], HEX[p[i] & 0xF], ' ' };
        debug_write(out, 3);
    }
    debug_puts(">\n");
}

