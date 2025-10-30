#include <aura/pit.h>
#include <aura/io.h>

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43

void pit_init(uint32_t frequency) {
    uint32_t divisor = 1193180 / frequency;
    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}
