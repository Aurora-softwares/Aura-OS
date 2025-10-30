#define DECLARE_ISR(num, has_err) \
    .global isr_stub_##num; \
    isr_stub_##num:; \
        .if !(has_err); \
            pushq $0; \
        .endif; \
        pushq $num; \
        jmp isr_common_stub;

.global isr_common_stub
isr_common_stub:
    cli
    mov %rsp, %rdi
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    mov %rsp, %rdi
    call interrupt_dispatch

    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax

    addq $16, %rsp
    sti
    iretq

DECLARE_ISR(0, 0)
DECLARE_ISR(1, 0)
DECLARE_ISR(2, 0)
DECLARE_ISR(3, 0)
DECLARE_ISR(4, 0)
DECLARE_ISR(5, 0)
DECLARE_ISR(6, 0)
DECLARE_ISR(7, 0)
DECLARE_ISR(8, 1)
DECLARE_ISR(9, 0)
DECLARE_ISR(10, 1)
DECLARE_ISR(11, 1)
DECLARE_ISR(12, 1)
DECLARE_ISR(13, 1)
DECLARE_ISR(14, 1)
DECLARE_ISR(15, 0)
DECLARE_ISR(16, 0)
DECLARE_ISR(17, 1)
DECLARE_ISR(18, 0)
DECLARE_ISR(19, 0)
DECLARE_ISR(32, 0)
DECLARE_ISR(33, 0)
DECLARE_ISR(44, 0)

.section .note.GNU-stack,"",@progbits
