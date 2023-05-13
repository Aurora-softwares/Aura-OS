org 0x7C00          ; Set origin to the beginning of the boot sector
bits 16             ; Set mode to 16-bit

jmp main            ; Jump to the main entry point

; Data section
kernel_start:
    ; This is where the kernel will be loaded to
    ; We set this as a label so we can refer to it later
    ; Its value is set by the bootloader when the kernel is loaded
    dd 0

; Bootloader entry point
main:
    ; Set up the stack pointer
    mov esp, 0x7C00

    ; Load the kernel into memory
    ; We'll load it to 0x10000 (1 MB)
    ; The kernel should be in ELF format
    ; The kernel_start label is used to set the starting address
    mov ah, 0x02        ; BIOS read sector function
    mov al, 1           ; Read one sector
    mov ch, 0           ; Cylinder number
    mov cl, 2           ; Sector number (starting at 1)
    mov dh, 0           ; Head number
    mov dl, 0           ; Drive number (0 = floppy disk)
    mov bx, kernel_start    ; Address to read to
    mov es, bx          ; Set ES to the address of kernel_start
    xor bx, bx          ; Offset 0
    int 0x13            ; Call BIOS disk I/O function

    ; Check if the read was successful
    jc boot_error

    ; Jump to the kernel
    jmp far [kernel_start]

; Error handling
boot_error:
    ; Print an error message to the screen
    mov ah, 0x0E        ; BIOS teletype function
    mov al, 'E'
    int 0x10
    mov al, 'r'
    int 0x10
    mov al, 'r'
    int 0x10
    mov al, 'o'
    int 0x10
    mov al, 'r'
    int 0x10
    ; Halt the system
    cli
    hlt
