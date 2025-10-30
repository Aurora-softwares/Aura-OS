#include "cli.h"

#include <stddef.h>

#include "keyboard.h"
#include "mouse.h"
#include "screen.h"

#define CLI_BUFFER_SIZE 128

static char input_buffer[CLI_BUFFER_SIZE];
static size_t input_length;

static size_t cli_strlen(const char *str) {
    size_t length = 0;
    while (str[length] != '\0') {
        ++length;
    }
    return length;
}

static int cli_strings_equal(const char *lhs, size_t lhs_len, const char *rhs) {
    size_t rhs_len = cli_strlen(rhs);
    if (lhs_len != rhs_len) {
        return 0;
    }

    for (size_t i = 0; i < lhs_len; ++i) {
        if (lhs[i] != rhs[i]) {
            return 0;
        }
    }

    return 1;
}

static char *cli_trim(char *str) {
    while (*str == ' ') {
        ++str;
    }

    size_t len = cli_strlen(str);
    while (len > 0 && str[len - 1] == ' ') {
        str[--len] = '\0';
    }

    return str;
}

static void cli_print_banner(void) {
    print("Aura OS shell ready. Type 'help' for assistance.\n\n");
}

static void cli_print_prompt(void) {
    print("AURA> ");
}

static void cli_command_help(void) {
    print("Available commands:\n");
    print("  help    - Show this help text\n");
    print("  clear   - Clear the screen\n");
    print("  echo    - Echo the provided text\n");
    print("  meminfo - Show basic memory information\n");
    print("  lsusb   - List detected USB devices\n");
    print("  vidinfo - Display screen information\n");
}

static void cli_command_clear(void) {
    clearScreen();
    moveCursor(0, 0);
    cli_print_banner();
}

static void cli_command_echo(const char *args) {
    if (args[0] == '\0') {
        print("\n");
        return;
    }

    print(args);
    print("\n");
}

static void cli_command_meminfo(void) {
    print("Memory information is not available in this build.\n");
}

static void cli_command_lsusb(void) {
    print("USB devices:\n");
    print("  HID stack initialisation is stubbed; no devices enumerated.\n");
}

static void cli_command_vidinfo(void) {
    print("Display: 80x25 text mode\n");
}

static void cli_execute_command(char *line) {
    char *trimmed = cli_trim(line);
    if (*trimmed == '\0') {
        return;
    }

    char *cursor = trimmed;
    while (*cursor != '\0' && *cursor != ' ') {
        ++cursor;
    }

    size_t command_len = (size_t)(cursor - trimmed);
    char *args = cursor;
    if (*args != '\0') {
        *args++ = '\0';
        while (*args == ' ') {
            ++args;
        }
    }

    if (cli_strings_equal(trimmed, command_len, "help")) {
        cli_command_help();
    } else if (cli_strings_equal(trimmed, command_len, "clear")) {
        cli_command_clear();
    } else if (cli_strings_equal(trimmed, command_len, "echo")) {
        cli_command_echo(args);
    } else if (cli_strings_equal(trimmed, command_len, "meminfo")) {
        cli_command_meminfo();
    } else if (cli_strings_equal(trimmed, command_len, "lsusb")) {
        cli_command_lsusb();
    } else if (cli_strings_equal(trimmed, command_len, "vidinfo")) {
        cli_command_vidinfo();
    } else {
        print("Unknown command. Type 'help' for a list of commands.\n");
    }
}

void cli_run(void) {
    input_length = 0;
    cli_print_banner();
    cli_print_prompt();
    mouse_refresh_pointer();

    while (1) {
        mouse_poll();

        int ch = keyboard_poll_char();
        if (ch == KEYBOARD_NO_EVENT) {
            continue;
        }

        if (ch == KEYBOARD_BACKSPACE) {
            if (input_length > 0) {
                --input_length;
                input_buffer[input_length] = '\0';
                screen_backspace();
                mouse_refresh_pointer();
            }
            continue;
        }

        if (ch == '\n') {
            print("\n");
            input_buffer[input_length] = '\0';
            cli_execute_command(input_buffer);
            input_length = 0;
            cli_print_prompt();
            mouse_refresh_pointer();
            continue;
        }

        if (ch >= 32 && ch <= 126) {
            if (input_length + 1 < CLI_BUFFER_SIZE) {
                input_buffer[input_length++] = (char)ch;
                screen_write_char((char)ch);
                mouse_refresh_pointer();
            }
        }
    }
}
