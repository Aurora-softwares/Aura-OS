#include <stdint.h>

static inline __attribute__((always_inline)) void bios_print_char(char c);
static inline __attribute__((always_inline)) void bios_backspace();
static inline __attribute__((always_inline)) void bios_newline();
static inline __attribute__((always_inline)) char bios_read_char();
static inline __attribute__((always_inline)) void print_welcome();
static inline __attribute__((always_inline)) void print_prompt();
static inline __attribute__((always_inline)) void print_bar();
static inline __attribute__((always_inline)) void print_unknown();
static inline void bios_clear_screen();

extern "C" void kernel_main() {
    bios_clear_screen();
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
                bios_print_char(c);
            }
        }

        buffer[length] = '\0';

        if (length == 3 &&
            buffer[0] == 'f' &&
            buffer[1] == 'o' &&
            buffer[2] == 'o') {
            print_bar();
            continue;
        }

        if (length > 0) {
            print_unknown();
        }
    }
}

static inline __attribute__((always_inline)) void bios_print_char(char c) {
    asm volatile (
        "int $0x10\n"
        :
        : "a"(static_cast<uint16_t>(0x0E00 | static_cast<uint8_t>(c))),
          "b"(static_cast<uint16_t>(0x0007))
        : "cc"
    );
}

static inline void bios_backspace() {
    bios_print_char('\b');
    bios_print_char(' ');
    bios_print_char('\b');
}

static inline void bios_newline() {
    bios_print_char('\r');
    bios_print_char('\n');
}

static inline void bios_clear_screen() {
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

static inline __attribute__((always_inline)) void print_welcome() {
    bios_print_char('h');
    bios_print_char('e');
    bios_print_char('l');
    bios_print_char('l');
    bios_print_char('o');
    bios_newline();
}

static inline __attribute__((always_inline)) void print_prompt() {
    bios_print_char('>');
    bios_print_char(' ');
}

static inline __attribute__((always_inline)) void print_bar() {
    bios_print_char('b');
    bios_print_char('a');
    bios_print_char('r');
    bios_newline();
}

static inline __attribute__((always_inline)) void print_unknown() {
    bios_print_char('U');
    bios_print_char('n');
    bios_print_char('k');
    bios_print_char('n');
    bios_print_char('o');
    bios_print_char('w');
    bios_print_char('n');
    bios_print_char(' ');
    bios_print_char('c');
    bios_print_char('o');
    bios_print_char('m');
    bios_print_char('m');
    bios_print_char('a');
    bios_print_char('n');
    bios_print_char('d');
    bios_newline();
}
