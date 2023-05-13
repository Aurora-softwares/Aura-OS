@echo off
cls
setlocal enabledelayedexpansion

REM VARIABLES
	: Basic OS info
		set os_name=Aura-OS
		set os_version=0.0.1
	: Directories
		set dir_src=.\src\
		set dir_tmp=.\tmp\
		set dir_out=.\out\
		set dir_iso=.\iso\
	: File names
		set kernel_src=%dir_src%kernel.c
		set kernel_out=%dir_out%kernel.bin
		set bootloader_src=%dir_src%bootloader.asm
		set bootloader_out=%dir_out%bootloader.bin
		set os_image=%dir_iso%%os_name%-%os_version%.bin
	: MISC
		set error-d=NULL
		set error-t=NULL
		set bootloader-s1=NULL
		set kernel-s1=NULL
		set kernel-s2=NULL
		set combine-s1=NULL
		set boot-s1=NULL
	goto main
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
		.\bin\qemu-8.0.0\qemu-system-x86_64.exe -drive format=raw,file=%1
		exit /B 0
	:log
		for /f "skip=1 delims=" %%d in ('wmic os get localdatetime') do set datetime=%%d
		set "date=%datetime:~0,8%"
		set "time=%datetime:~8,6%"
		set "formatted_datetime=%date:~0,4%%date:~4,2%%date:~8,2% %time:~0,2%:%time:~3,2%:%time:~6,2%"
		if %1==1 (
			echo [36m[TRACE] [%formatted_datetime%] %2 [0m
		) else if %1==2 (
			echo [96m[DEBUG] [%formatted_datetime%] %2 [0m
		) else if %1==3 (
			echo [92m[ INFO] [%formatted_datetime%] %2 [0m
		) else if %1==4 (
			echo [93m[ WARN] [%formatted_datetime%] %2 [0m
		) else if %1==5 (
			echo [91m[ERROR] [%formatted_datetime%] %2 [0m
		) else if %1==5 (
			echo [107;91m[FATAL] [%formatted_datetime%] %2 [0m
		)
		exit /B 0
REM PROGRAM
	:main
		call :log 3 "Compiling the bootloader."
		call :compile_c %bootloader_out% %bootloader_src%
		call :log 3 "Compiling the kernel."
		call :compile_c %kernel_out% %kernel_src%
		call :log 3 "Combining the bootloader and Kernel bin files."
		call :combine_bin %bootloader_out% %kernel_out% %os_image%
		call :log 3 "Booting OS with NASM."
		call :boot %os_image%

	:error
		echo ERROR: %error-d%
		echo %error-t%
		goto end

	:success
		echo Successfully manages to compile and boot into the OS.
		goto end

	:end
	echo closing.