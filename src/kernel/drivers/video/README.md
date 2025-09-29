# Video Subsystem

The video subsystem selects the preferred graphics mode at boot and exposes a
linear framebuffer abstraction for other kernel subsystems.

## Layout
- `gop/`: UEFI Graphics Output Protocol support.
- `vbe/`: Legacy BIOS VBE handling.
- `fb/`: Shared framebuffer helpers, drawing primitives, and font routines.

## Limitations & TODO
- Mode detection and switching logic are not implemented yet.
- No framebuffer primitives or text rendering routines are available.
- Integration with the shell still relies on the legacy text mode code.
