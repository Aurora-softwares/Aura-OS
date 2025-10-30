#include <aura/interrupts.h>
#include <aura/string.h>
#include <aura/log.h>
#include <stdint.h>

#define IDT_ENTRIES 256

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

extern void isr_stub_0(void);
extern void isr_stub_1(void);
extern void isr_stub_2(void);
extern void isr_stub_3(void);
extern void isr_stub_4(void);
extern void isr_stub_5(void);
extern void isr_stub_6(void);
extern void isr_stub_7(void);
extern void isr_stub_8(void);
extern void isr_stub_9(void);
extern void isr_stub_10(void);
extern void isr_stub_11(void);
extern void isr_stub_12(void);
extern void isr_stub_13(void);
extern void isr_stub_14(void);
extern void isr_stub_15(void);
extern void isr_stub_16(void);
extern void isr_stub_17(void);
extern void isr_stub_18(void);
extern void isr_stub_19(void);
extern void isr_stub_32(void);
extern void isr_stub_33(void);
extern void isr_stub_44(void);

extern void interrupt_dispatch(uint64_t *stack_frame);

static struct idt_entry idt[IDT_ENTRIES];
static isr_handler_t handlers[IDT_ENTRIES];

static void set_idt_gate(int n, void (*handler)(void)) {
    uint64_t addr = (uint64_t)handler;
    idt[n].offset_low = addr & 0xFFFF;
    idt[n].selector = 0x08;
    idt[n].ist = 0;
    idt[n].type_attr = 0x8E;
    idt[n].offset_mid = (addr >> 16) & 0xFFFF;
    idt[n].offset_high = (addr >> 32) & 0xFFFFFFFF;
    idt[n].zero = 0;
}

void idt_init(void) {
    memset(idt, 0, sizeof(idt));
    memset(handlers, 0, sizeof(handlers));

    set_idt_gate(0, isr_stub_0);
    set_idt_gate(1, isr_stub_1);
    set_idt_gate(2, isr_stub_2);
    set_idt_gate(3, isr_stub_3);
    set_idt_gate(4, isr_stub_4);
    set_idt_gate(5, isr_stub_5);
    set_idt_gate(6, isr_stub_6);
    set_idt_gate(7, isr_stub_7);
    set_idt_gate(8, isr_stub_8);
    set_idt_gate(9, isr_stub_9);
    set_idt_gate(10, isr_stub_10);
    set_idt_gate(11, isr_stub_11);
    set_idt_gate(12, isr_stub_12);
    set_idt_gate(13, isr_stub_13);
    set_idt_gate(14, isr_stub_14);
    set_idt_gate(15, isr_stub_15);
    set_idt_gate(16, isr_stub_16);
    set_idt_gate(17, isr_stub_17);
    set_idt_gate(18, isr_stub_18);
    set_idt_gate(19, isr_stub_19);
    set_idt_gate(32, isr_stub_32);
    set_idt_gate(33, isr_stub_33);
    set_idt_gate(44, isr_stub_44);

    struct idt_ptr ptr = {
        .limit = sizeof(idt) - 1,
        .base = (uint64_t)idt,
    };

    __asm__ volatile("lidt %0" : : "m"(ptr));
}

void interrupts_enable(void) {
    __asm__ volatile("sti");
}

void interrupts_disable(void) {
    __asm__ volatile("cli");
}

void register_interrupt_handler(uint8_t vector, isr_handler_t handler) {
    handlers[vector] = handler;
}

void interrupt_dispatch(uint64_t *stack_frame) {
    uint64_t vector = stack_frame[0];
    uint64_t error_code = stack_frame[1];
    (void)error_code;
    interrupt_frame_t frame = {
        .rip = stack_frame[2],
        .cs = stack_frame[3],
        .rflags = stack_frame[4],
        .rsp = 0,
        .ss = 0,
    };

    if (handlers[vector]) {
        handlers[vector](&frame);
    } else {
        log_printf("Unhandled interrupt %u\n", (uint64_t)vector);
    }
}
