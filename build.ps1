function Build-OS {
	$OS_Name = "Aura";
	$OS_Version = "0.0.1";
	function CleanUp {
		Write-Host "Cleaning up temporary files..."
		Remove-Item -Path "./out/" -Force -Recurse -ErrorAction SilentlyContinue
	}
	function CompileBootloader {
		Write-Host "Compiling bootloader..."
		New-Item -ItemType Directory -Force -Path "./out/boot" | Out-Null
		.\bin\mingw64-13.1.0\bin\nasm.exe -f bin -o ./out/boot/bootloader.bin ./src/boot/bootloader.asm
	}
	function CompileKernel {
		Write-Host "Compiling kernel..."
		New-Item -ItemType Directory -Force -Path "./out/kernel" | Out-Null
		.\bin\mingw64-13.1.0\bin\gcc.exe -m64 -ffreestanding -c ./src/kernel/kernel.c -o ./out/kernel/kernel.o
	}
	function LinkOS {
		Write-Host "Linking bootloader and kernel..."
		.\bin\mingw64-13.1.0\bin\ld.exe -T ./src/linker.ld -o ./out/$OS_Name-$OS_Version.bin ./out/kernel/kernel.o ./out/boot/bootloader.bin
	}
	function CreateISO {
		Write-Host "Creating bootable ISO image..."
		New-Item -ItemType Directory -Force -Path "./out/iso" | Out-Null
		.\bin\xorriso-1.5.6\xorriso.exe -as mkisofs -b ./out/boot/bootloader.bin -no-emul-boot -boot-load-size 4 -o ./out/iso/$OS_Name-$OS_Version.iso iso_root
	}
	function LaunchQEMU {
		Write-Host "Launching OS using QEMU..."
		.\bin\qemu-8.0.0\qemu-system-x86_64.exe -cdrom ./out/iso/$OS_Name-$OS_Version.iso
	}

	CleanUp
	CompileBootloader
	CompileKernel
	LinkOS
	CreateISO
	LaunchQEMU
	
}