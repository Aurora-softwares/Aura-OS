# Aura Shell

The Aura shell provides a basic command interface for interacting with the
system once booted.

## Goals
- Offer commands such as `help`, `echo`, `clear`, `reboot`, `meminfo`, `lsusb`,
  and `vidinfo`.
- Consume keyboard events via the USB HID stack and render to the framebuffer
  console.
- Remain modular so future scripting support can hook into the same interfaces.

## Limitations & TODO
- The shell loop and command handlers are not yet implemented.
- There is no integration with keyboard or framebuffer backends.
- Command registration and help output are pending design work.
