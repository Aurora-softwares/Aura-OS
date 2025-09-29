# Manual Test Plan

These steps document the intended manual validation flow for the forthcoming
USB, video, and shell subsystems. They currently describe expected behaviour
that is not yet implemented.

## 1. Boot verification (UEFI GOP)
1. Build the UEFI image with `make qemu-uefi`.
2. Launch QEMU with the documented command line using OVMF.
3. After boot, run `vidinfo` in the shell and confirm the native monitor
   resolution is reported; otherwise verify the fallback 1024x768 mode.

## 2. Boot verification (BIOS VBE)
1. Build the BIOS image with `make qemu-bios`.
2. Launch QEMU with the documented BIOS command line.
3. Run `vidinfo` and confirm the reported mode.

## 3. USB keyboard input
1. Attach a virtual USB keyboard to QEMU (`-device usb-kbd`).
2. Ensure the shell accepts input from the keyboard without missing characters.
3. Confirm debounced typing and ASCII translation for the US layout.

## 4. Shell command checks
1. `help` lists all available commands with short descriptions.
2. `echo <text>` writes the provided text back to the console.
3. `cls`/`clear` clears the framebuffer console.
4. `reboot` triggers a warm reboot sequence.
5. `meminfo` prints high-level memory usage statistics.
6. `lsusb` lists detected USB devices in `bus:addr vendor:product class`
   format.
7. `vidinfo` prints the active video mode details.

> **Status:** Pending implementation of the USB stack, video subsystem, and
> interactive shell.
