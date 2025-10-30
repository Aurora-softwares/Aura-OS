#ifndef AURA_INTERRUPTS_H
#define AURA_INTERRUPTS_H

#include <stdint.h>

typedef struct interrupt_frame {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} interrupt_frame_t;

typedef void (*isr_handler_t)(interrupt_frame_t *frame);

void idt_init(void);
void interrupts_enable(void);
void interrupts_disable(void);
void register_interrupt_handler(uint8_t vector, isr_handler_t handler);

#endif
