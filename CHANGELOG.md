# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Added
- Introduced a NASM-based stage-1 bootloader (`src/bootloader.asm`) that loads a multi-sector kernel header, prints disk errors, and transfers control via far return.
- Added a stage-2 real-mode kernel wrapper (`src/kernel.asm`) that embeds the C++ payload and safely halts on return.
- Implemented a freestanding C++ shell (`src/kernel.cpp`) that prints `Welcome`, echoes typed characters using BIOS services, and recognises the `foo` command to output `bar`.

### Changed
- Switched the build pipeline (`build.bat`) to assemble both boot stages with NASM and compile/strip the C++ kernel code before packaging the final image.

### Removed
- Removed the previous C-based bootloader and kernel stubs (`src/bootloader.c`, `src/kernel.c`) replaced by the new NASM/C++ flow.

