@echo off
cls
setlocal enabledelayedexpansion

REM VARIABLES
	: Script arguments
		set no_compile=
	: Basic OS info
		set os_name=Aura-OS
		set os_version=0.0.1
	: Directories
		set dir_src=.\src\
		set dir_tmp=.\tmp\
		set dir_out=.\out\
		set dir_iso=.\iso\
	: File names
		set kernel_src=%dir_src%kernel.asm
		set kernel_cpp_src=%dir_src%kernel.cpp
		set kernel_cpp_obj=%dir_tmp%kernel.o
		set kernel_cpp_bin=%dir_tmp%kernel_cpp.bin
		set kernel_out=%dir_out%kernel.bin
		set bootloader_src=%dir_src%bootloader.asm
		set bootloader_out=%dir_out%bootloader.bin
		set os_image=%dir_iso%%os_name%-%os_version%.bin
		set disk_size_mb=2
		set disk_image=%dir_iso%%os_name%-disk-%disk_size_mb%MB.img
	: MISC
		set error-d=NULL
		set error-t=NULL
		set bootloader-s1=NULL
		set kernel-s1=NULL
		set kernel-s2=NULL
		set combine-s1=NULL
		set boot-s1=NULL
	goto parse_args
REM FUNCTIONS
	:generate_string
		set "letters=abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"
		set %2=
		set loopcount=%1
		:loop
			set /a "index=%random% %% 62"
			set "letter=!letters:~%index%,1!"
			set result=%result%%letter%
			set %2=%result%

			set /a loopcount=loopcount-1
			if %loopcount%==0 goto exitloop
			goto loop
		:exitloop
			exit /B 0
	:compile_c
		call :generate_string 30 temp_string

		call :log 3 "Converting the file into a temporary object"
		.\bin\mingw64-11.2.0\bin\x86_64-w64-mingw32-gcc.exe -ffreestanding -c %2 -o %dir_tmp%%temp_string%.o

		call :log 3 "Compiling the object"
		.\bin\mingw64-11.2.0\bin\objcopy.exe -O binary %dir_tmp%%temp_string%.o %1

		call :log 3 "removing the temporary object"
		del %dir_tmp%%temp_string%.o
		exit /B 0
	:compile_asm
		.\bin\nasm-2.16.01\nasm.exe -f bin -o %1 %2
		exit /B 0
	:combine_bin
		copy /b %1+%2 %3
		exit /B 0
	:boot
		.\bin\qemu-8.0.0\qemu-system-x86_64.exe -drive file=%1,format=raw,if=ide -boot c -m 64M
		exit /B 0
	:log
		for /f "skip=1 delims=" %%d in ('wmic os get localdatetime') do set datetime=%%d
		set "date=%datetime:~0,8%"
		set "time=%datetime:~8,6%"
		set "formatted_datetime=%date:~0,4%%date:~4,2%%date:~8,2% %time:~0,2%:%time:~3,2%:%time:~6,2%"
		if %1==1 (
			echo [36m[TRACE] [%formatted_datetime%] %2 [0m
		) else if %1==2 (
			echo [96m[DEBUG] [%formatted_datetime%] %2 [0m
		) else if %1==3 (
			echo [92m[ INFO] [%formatted_datetime%] %2 [0m
		) else if %1==4 (
			echo [93m[ WARN] [%formatted_datetime%] %2 [0m
		) else if %1==5 (
			echo [91m[ERROR] [%formatted_datetime%] %2 [0m
		) else if %1==5 (
			echo [107;91m[FATAL] [%formatted_datetime%] %2 [0m
		)
		exit /B 0
REM PROGRAM
	:parse_args
		if "%1"=="" goto main
		if "%1"=="--no-compile" set no_compile=1
		shift
		goto parse_args
	:main
		if not defined no_compile (
		call :log 3 "Compiling the bootloader."
		call :compile_asm %bootloader_out% %bootloader_src%
		call :log 3 "Compiling the C++ kernel stage."
		.\bin\mingw64-11.2.0\bin\x86_64-w64-mingw32-g++.exe -m16 -ffreestanding -fno-exceptions -fno-rtti -nostdlib -fno-stack-protector -fno-asynchronous-unwind-tables -fno-unwind-tables -Os -c %kernel_cpp_src% -o %kernel_cpp_obj%
		.\bin\mingw64-11.2.0\bin\objcopy.exe -O binary --only-section=.text %kernel_cpp_obj% %kernel_cpp_bin%
		call :log 3 "Compiling the kernel."
		call :compile_asm %kernel_out% %kernel_src%
			call :log 3 "Combining the bootloader and Kernel bin files."
			call :combine_bin %bootloader_out% %kernel_out% %os_image%
		) else (
			call :log 3 "Skipping Compile"
		)

		rem Ensure the virtual disk exists and is sized to 2MB; write OS image to its beginning
		set /a disk_size_bytes=%disk_size_mb%*1024*1024
		call :log 3 "Ensuring %disk_size_mb%MB virtual disk exists: %disk_image%"
		powershell -NoProfile -Command ^
			"$img = '%disk_image%'; $size = [int64](%disk_size_mb%*1MB);" ^
			"if (-not (Test-Path $img)) {" ^
			"  $fs = [System.IO.File]::Open($img, 'Create', 'ReadWrite');" ^
			"  try { $fs.SetLength($size) } finally { $fs.Close() }" ^
			"} else {" ^
			"  $fs = [System.IO.File]::Open($img, 'Open', 'ReadWrite');" ^
			"  try { if ($fs.Length -ne $size) { $fs.SetLength($size) } } finally { $fs.Close() }" ^
			"}"
		call :log 3 "Writing boot+kernel image to start of virtual disk"
		powershell -NoProfile -Command ^
			"$img = '%disk_image%'; $boot = '%os_image%';" ^
			"$fs = [System.IO.File]::Open($img, 'Open', 'ReadWrite');" ^
			"try { $b = [System.IO.File]::ReadAllBytes($boot); $fs.Write($b, 0, $b.Length) } finally { $fs.Close() }"

		call :log 3 "Booting OS with QEMU using the 2MB virtual disk."
		call :boot %disk_image%

	:error
		echo ERROR: %error-d%
		echo %error-t%
		goto end

	:success
		echo Successfully manages to compile and boot into the OS.
		goto end

	:end
	echo closing.
