#include <aura/kernel.h>
#include <aura/terminal.h>
#include <aura/log.h>
#include <aura/interrupts.h>
#include <aura/pic.h>
#include <aura/timer.h>
#include <aura/keyboard.h>
#include <aura/mouse.h>
#include <aura/usb.h>
#include <aura/shell.h>

void kernel_main(void) {
    terminal_init();
    log_init();
    log_info("AuraOS kernel starting");

    pic_init();
    idt_init();
    timer_init(1000);
    keyboard_init();
    mouse_init();
    usb_init();

    interrupts_enable();
    shell_init();
    shell_run();

    while (1) {
        __asm__ volatile ("hlt");
    }
}
