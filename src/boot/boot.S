#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6
#define MULTIBOOT2_ARCHITECTURE_I386 0
#define MULTIBOOT2_HEADER_TAG_END 0

#define GDT_NULL 0
#define GDT_KERNEL_CODE 0x00af9a000000ffff
#define GDT_KERNEL_DATA 0x00af92000000ffff

#define PAGE_PRESENT 0x1
#define PAGE_WRITABLE 0x2
#define PAGE_PS 0x80

.section .multiboot
.align 8
.long MULTIBOOT2_HEADER_MAGIC
.long MULTIBOOT2_ARCHITECTURE_I386
.long header_end - header_start
.long -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT2_ARCHITECTURE_I386 + (header_end - header_start))
header_start:
    .short MULTIBOOT2_HEADER_TAG_END
    .short 0
    .long 8
header_end:

.section .bss
.align 16
.global bootstrap_stack_top
bootstrap_stack:
    .skip 16384
bootstrap_stack_top:

.align 4096
pml4_table:
    .quad 0
    .skip 4096 - 8

.align 4096
pdpt_table:
    .quad 0
    .quad 0
    .quad 0
    .quad 0

.align 4096
pd_table:
    .rept 512
        .quad 0
    .endr

.section .data
.align 16
gdt_table:
    .quad GDT_NULL
    .quad GDT_KERNEL_CODE
    .quad GDT_KERNEL_DATA

.align 8
gdt_descriptor:
    .short (3 * 8) - 1
    .quad gdt_table

.section .text
.code32
.global _start
_start:
    cli
    mov $bootstrap_stack_top, %esp

    mov $pd_table, %eax
    mov $0, %ecx
1:
    mov %ecx, %edx
    shl $21, %edx
    or $(PAGE_PRESENT | PAGE_WRITABLE | PAGE_PS), %edx
    mov %edx, (%eax,%ecx,8)
    inc %ecx
    cmp $512, %ecx
    jne 1b

    mov $pdpt_table, %eax
    mov $pd_table, %edx
    or $(PAGE_PRESENT | PAGE_WRITABLE), %edx
    mov %edx, (%eax)

    mov $pml4_table, %eax
    mov $pdpt_table, %edx
    or $(PAGE_PRESENT | PAGE_WRITABLE), %edx
    mov %edx, (%eax)

    mov $pml4_table, %eax
    mov %eax, %cr3

    mov %cr4, %eax
    or $(1 << 5), %eax          # PAE
    mov %eax, %cr4

    mov $0xC0000080, %ecx       # EFER
    rdmsr
    or $(1 << 8), %eax          # LME
    wrmsr

    mov %cr0, %eax
    or $0x80000001, %eax        # PG + PE
    mov %eax, %cr0

    lgdt gdt_descriptor
    ljmp $0x08, $long_mode_entry

.code64
long_mode_entry:
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov %ax, %fs
    mov %ax, %gs

    mov $kernel_stack_top, %rax
    mov %rax, %rsp

    .extern kernel_main
    call kernel_main

halt_loop:
    cli
    hlt
    jmp halt_loop

.section .bss
.align 16
.global kernel_stack_top
kernel_stack:
    .skip 32768
kernel_stack_top:

.section .note.GNU-stack,"",@progbits
