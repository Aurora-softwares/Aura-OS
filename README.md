# AuraOS

AuraOS is a lightweight hobby operating system targeting x86_64 hardware. The
system boots through a Multiboot2 compatible stage, switches to long mode, and
runs a tiny freestanding kernel. The focus of this reboot is simplicity: a
text-based shell, a handful of builtin commands, timer support, and input
through PS/2 or boot-protocol USB keyboards and mice.

## Features

- 64-bit kernel entered from a Multiboot2 header.
- VGA text console with scrolling support and a serial log channel.
- PIC/IDT based interrupt handling with a PIT-backed system timer.
- Simple shell with commands for `help`, `clear`, `uptime`, `devices`, and
  `reboot`.
- Input drivers for legacy PS/2 devices with experimental UHCI USB polling to
  support QEMU's `usb-kbd` and `usb-mouse` devices.

## Building

The build uses the host `gcc` toolchain. A cross compiler is not required.

```bash
make kernel
```

The `kernel.elf` image will be placed under `out/`. Creating an ISO requires
`grub-mkrescue` (or `grub2-mkrescue`). The command will fail if the tool is not
installed.

```bash
make iso
```

## Running in QEMU

Once the ISO has been generated you can boot it with QEMU:

```bash
qemu-system-x86_64 -m 256M -serial stdio -cdrom out/iso/AuraOS-$(cat VERSION).iso \
    -machine pc -device qemu-xhci -device usb-kbd -device usb-mouse
```

The shell will greet you on the VGA console and over the serial port. USB
support is best-effort; if enumeration fails the PS/2 fallback remains
available.

## License

AuraOS is distributed under the Apache 2.0 license. See `LICENSE` for details.
