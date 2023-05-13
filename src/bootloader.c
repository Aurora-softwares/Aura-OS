void main() {
    char* kernel = (char*)0x1000;
    asm("mov $0x1000, %rdi");
    asm("mov $0x0, %rsi");
    asm("mov $0x2000, %rdx");
    asm("xor %rax, %rax");
    asm("int $0x13");
    asm("jmp 0x1000");
    while(1);
}
