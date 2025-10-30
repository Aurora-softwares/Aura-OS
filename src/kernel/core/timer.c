#include <aura/timer.h>
#include <aura/pit.h>
#include <aura/interrupts.h>
#include <aura/pic.h>
#include <stdint.h>

static volatile uint64_t tick_count = 0;

static void timer_handler(interrupt_frame_t *frame) {
    (void)frame;
    tick_count++;
    pic_send_eoi(0);
}

void timer_init(uint32_t frequency) {
    pit_init(frequency);
    register_interrupt_handler(32, timer_handler);
}

uint64_t timer_ticks(void) {
    return tick_count;
}

void timer_sleep(uint64_t ticks) {
    uint64_t target = tick_count + ticks;
    while (tick_count < target) {
        __asm__ volatile("hlt");
    }
}
