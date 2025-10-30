#include <aura/log.h>
#include <aura/serial.h>
#include <aura/terminal.h>
#include <aura/string.h>
#include <stdarg.h>

static void log_vprintf(const char *fmt, va_list args) {
    char buffer[512];
    size_t pos = 0;
    for (size_t i = 0; fmt[i] && pos < sizeof(buffer) - 1; ++i) {
        if (fmt[i] != '%') {
            buffer[pos++] = fmt[i];
            continue;
        }

        ++i;
        char spec = fmt[i];
        if (spec == 's') {
            const char *str = va_arg(args, const char *);
            if (!str) {
                str = "(null)";
            }
            while (*str && pos < sizeof(buffer) - 1) {
                buffer[pos++] = *str++;
            }
        } else if (spec == 'c') {
            char ch = (char)va_arg(args, int);
            buffer[pos++] = ch;
        } else if (spec == 'x' || spec == 'X') {
            uint64_t value = va_arg(args, uint64_t);
            char tmp[17];
            for (int j = 0; j < 16; ++j) {
                uint8_t nibble = (value >> ((15 - j) * 4)) & 0xF;
                tmp[j] = nibble < 10 ? ('0' + nibble) : ((spec == 'x' ? 'a' : 'A') + (nibble - 10));
            }
            tmp[16] = '\0';
            const char *start = tmp;
            while (*start == '0' && *(start + 1)) {
                ++start;
            }
            while (*start && pos < sizeof(buffer) - 1) {
                buffer[pos++] = *start++;
            }
        } else if (spec == 'u') {
            uint64_t value = va_arg(args, uint64_t);
            char tmp[32];
            int j = 0;
            if (value == 0) {
                tmp[j++] = '0';
            } else {
                while (value && j < (int)sizeof(tmp) - 1) {
                    tmp[j++] = '0' + (value % 10);
                    value /= 10;
                }
            }
            while (j > 0 && pos < sizeof(buffer) - 1) {
                buffer[pos++] = tmp[--j];
            }
        } else if (spec == '%') {
            buffer[pos++] = '%';
        }
    }
    buffer[pos] = '\0';
    serial_write(buffer);
    terminal_write(buffer);
}

void log_init(void) {
    serial_init();
}

void log_info(const char *msg) {
    serial_write(msg);
    serial_write("\r\n");
    terminal_writeline(msg);
}

void log_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_vprintf(fmt, args);
    va_end(args);
}
