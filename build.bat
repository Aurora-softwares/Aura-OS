@echo off
cls

echo Compiling the kernel...
.\bin\g-2.16.01\nasm.exe -f bin -o .\out\kernel.bin .\src\kernel.asm

echo Compiling the bootloader
.\bin\nasm-2.16.01\nasm.exe -f bin -o .\out\bootloader.bin .\src\bootloader.asm

echo 
cd .\out\ && copy /b bootloader.bin+kernel.bin Aura-OS-Image.bin

echo Booting OS in NASM...
cd ..\ && .\bin\qemu-8.0.0\qemu-system-x86_64.exe -drive format=raw,file= .\out\Aura-OS-Image.bin
