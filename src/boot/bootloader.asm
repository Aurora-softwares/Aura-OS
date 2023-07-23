BITS 64
ORG 0x7C00

start:
    mov ax, 0x07C0
    add ax, 288
    mov ss, ax
    mov sp, 4096

    mov ax, 0x07C0
    mov ds, ax

    call clear_screen
    call print_string

    jmp $

clear_screen:
    mov ax, 0x0003      ; BIOS video function: set video mode 80x25, 16 colors
    int 0x10

    mov ax, 0x0002      ; BIOS video function: set cursor position to row=0, col=0
    int 0x10

    ret

print_string:
    mov si, hello_msg
.next_char:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E        ; BIOS teletype function to print character
    int 0x10
    jmp .next_char
.done:
    ret

hello_msg db 'Hello, World!', 0

times 510-($-$$) db 0
dw 0xAA55
