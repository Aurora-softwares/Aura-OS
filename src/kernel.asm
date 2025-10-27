[org 0]
[bits 16]

kernel_header:
    dw (kernel_end - kernel_header + 511) / 512
    dw kernel_entry

kernel_entry:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0xFFFE
    sti

    call cpp_kernel_entry

halt_forever:
    hlt
    jmp halt_forever

align 16
cpp_kernel_entry:
    incbin "tmp\kernel_cpp.bin"
cpp_kernel_end:

kernel_end:
