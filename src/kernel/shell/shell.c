#include <aura/shell.h>
#include <aura/terminal.h>
#include <aura/keyboard.h>
#include <aura/mouse.h>
#include <aura/timer.h>
#include <aura/string.h>
#include <aura/log.h>
#include <aura/usb.h>
#include <aura/io.h>
#include <stdint.h>

static void print_prompt(void) {
    terminal_write("> ");
}

static void command_help(void) {
    terminal_writeline("Available commands:");
    terminal_writeline("  help      - Show this message");
    terminal_writeline("  clear     - Clear the screen");
    terminal_writeline("  uptime    - Show system ticks");
    terminal_writeline("  devices   - List input devices");
    terminal_writeline("  reboot    - Reboot the machine");
}

static void command_clear(void) {
    terminal_clear();
}

static void command_uptime(void) {
    uint64_t ticks = timer_ticks();
    char buffer[32];
    int pos = 0;
    if (ticks == 0) {
        buffer[pos++] = '0';
    } else {
        uint64_t value = ticks;
        char tmp[32];
        int tpos = 0;
        while (value > 0 && tpos < (int)sizeof(tmp)) {
            tmp[tpos++] = '0' + (value % 10);
            value /= 10;
        }
        while (tpos > 0) {
            buffer[pos++] = tmp[--tpos];
        }
    }
    buffer[pos] = '\0';
    terminal_write("Ticks: ");
    terminal_writeline(buffer);
}

static void command_devices(void) {
    const usb_device_t *kbd = usb_find_keyboard();
    const usb_device_t *mouse = usb_find_mouse();
    terminal_writeline("Keyboard: ");
    if (kbd) {
        terminal_writeline("  USB keyboard (boot protocol)");
    } else {
        terminal_writeline("  PS/2 keyboard");
    }
    terminal_writeline("Mouse: ");
    if (mouse) {
        terminal_writeline("  USB mouse (boot protocol)");
    } else {
        terminal_writeline("  PS/2 mouse");
    }
}

static void command_reboot(void) {
    terminal_writeline("Rebooting...");
    while (1) {
        outb(0x64, 0xFE);
    }
}

void shell_init(void) {
    terminal_writeline("AuraOS shell. Type 'help' for commands.");
    print_prompt();
}

void shell_run(void) {
    char buffer[128];
    size_t length = 0;

    while (1) {
        char ch;
        if (!keyboard_read_char(&ch)) {
            mouse_packet_t packet;
            if (mouse_read_packet(&packet)) {
                continue;
            }
            __asm__ volatile ("hlt");
            continue;
        }

        if (ch == '\n') {
            terminal_putchar('\n');
            buffer[length] = '\0';
            if (strcmp(buffer, "help") == 0) {
                command_help();
            } else if (strcmp(buffer, "clear") == 0) {
                command_clear();
            } else if (strcmp(buffer, "uptime") == 0) {
                command_uptime();
            } else if (strcmp(buffer, "devices") == 0) {
                command_devices();
            } else if (strcmp(buffer, "reboot") == 0) {
                command_reboot();
            } else if (length > 0) {
                terminal_writeline("Unknown command");
            }
            length = 0;
            print_prompt();
        } else if (ch == '\b') {
            if (length > 0) {
                length--;
                terminal_write("\b \b");
            }
        } else {
            if (length < sizeof(buffer) - 1) {
                buffer[length++] = ch;
                terminal_putchar(ch);
            }
        }
    }
}
