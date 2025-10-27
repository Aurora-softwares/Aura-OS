[org 0x7C00]
[bits 16]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [boot_drive], dl

    mov ax, 0x1000
    mov es, ax
    mov word [load_offset], 0

    mov byte [cylinder], 0
    mov byte [head], 0
    mov byte [sector], 2

    call read_current_sector
    jc disk_error
    call advance_chs
    add word [load_offset], 512

    mov ax, [es:0]
    mov [total_sectors], ax
    mov ax, [es:2]
    mov [entry_offset], ax

    mov cx, [total_sectors]
    dec cx

load_loop:
    cmp cx, 0
    je load_done
    mov bx, [load_offset]
    call read_current_sector
    jc disk_error
    add word [load_offset], 512
    call advance_chs
    dec cx
    jmp load_loop

load_done:
    mov dl, [boot_drive]
    push word 0x1000
    push word [entry_offset]
    retf

disk_error:
    mov si, disk_err_msg
    call print_string
hang:
    hlt
    jmp hang

read_current_sector:
    push ax
    push cx
    push dx
    mov bx, [load_offset]
    mov ah, 0x02
    mov al, 1
    mov ch, [cylinder]
    mov cl, [sector]
    mov dh, [head]
    mov dl, [boot_drive]
    int 0x13
    pop dx
    pop cx
    pop ax
    ret

advance_chs:
    inc byte [sector]
    cmp byte [sector], 19
    jb .done
    mov byte [sector], 1
    inc byte [head]
    cmp byte [head], 2
    jb .done
    mov byte [head], 0
    inc byte [cylinder]
.done:
    ret

print_string:
    push ax
    push bx
    push si
.next:
    lodsb
    or al, al
    jz .end
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp .next
.end:
    pop si
    pop bx
    pop ax
    ret

boot_drive db 0
cylinder db 0
head db 0
sector db 0
total_sectors dw 0
entry_offset dw 0
load_offset dw 0
disk_err_msg db 'Disk read error', 0

times 510 - ($ - $$) db 0
dw 0xAA55
