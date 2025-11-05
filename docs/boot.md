# Aura OS Boot Walkthrough

This document dives into the boot journey for Aura OS, from the instant a virtual CPU powers up until the shell banner renders on screen. It is written as a guide for contributors who want to understand how control flows through the current codebase.

---

## High-Level Flow

```
┌──────────────────────────────────────────────────────────────┐
│  Firmware / Virtual BIOS                                     │
│      ▽                                                       │
│  Multiboot2 Bootloader (GRUB)                                │
│      ▽                                                       │
│  Multiboot2 Header (boot.S .multiboot)                       │
│      ▽                                                       │
│  32-bit trampoline (boot.S .text, .boot*)                    │
│      ▽                                                       │
│  Long-mode transition (boot.S _start64)                      │
│      ▽                                                       │
│  C Kernel `kernel_init` (kernel.c)                           │
│      ▽                                                       │
│  Device bring-up (screen.c, keyboard.c, usb/*, etc.)         │
└──────────────────────────────────────────────────────────────┘
```

Each stage below references the key file(s) that execute and a brief summary of their responsibilities.

---

## Stage 0 – Firmware and Boot Device (external to repo)

- **Who runs:** QEMU’s virtual BIOS (SeaBIOS by default) or hardware firmware.
- **What happens:** Firmware enumerates devices, picks the boot medium (our GRUB-based ISO), and jumps into the boot sector supplied on the disc.
- **Where this is defined:** Outside Aura OS; configuration lives in `grub.cfg`.

---

## Stage 1 – Multiboot2 Bootloader (`grub.cfg`)

- **Who runs:** GNU GRUB (from the ISO built in `out/iso/`).
- **What happens:** GRUB reads our Multiboot2 header (embedded in `boot.S`), loads the kernel binary (`src/kernel/kernel`), prepares Multiboot structures, then jumps to the entry point declared there.
- **Key artifacts:**
  - `grub.cfg` – tells GRUB which file to boot.
  - `kernel` – ELF binary produced by `src/kernel/Makefile`.

Timeline snapshot:

| Time | Component | Action |
|------|-----------|--------|
| T0   | GRUB      | Reads `/boot/grub/grub.cfg`. |
| T0+  | GRUB      | Validates Multiboot header in `boot.o (.multiboot)`. |
| T0++ | GRUB      | Loads `kernel` into memory, sets up Multiboot info, jumps to `_start`. |

---

## Stage 2 – 32-bit Bootstrap (`src/kernel/boot.S`)

### 2.1 Multiboot Header Section
`boot.S` begins with a `.multiboot` section containing the constants Multiboot expects (`MULTIBOOT2_HEADER_MAGIC`, architecture tag, length, checksum). No instructions execute here—it is purely metadata read by GRUB.

### 2.2 32-bit Entry `_start`

```
_start (32-bit):
  - Establish temporary stack (`tmp_stack`)
  - Build initial page tables (pml4, pdpt, page directories) covering:
        * Kernel physical window
        * Kernel higher-half mapping (0xFFFFFFFF80400000)
  - Write CR3, CR4, EFER, CR0 to enable paging & long mode
  - Load GDT (`gdt_ptr`)
  - Long jump -> `_start64`
```

Relevant sections in `boot.S`:
- `.bss` – allocates the page table buffers and stack.
- `.text` – contains `_start` and the setup loops for paging and GDT.

### 2.3 Control Registers & MSRs
`arch/x86_64/kernel.h`, `arch/x86_64/mmu.h`, and `arch/x86_64/msr.h` provide the constants used by inline assembly in `boot.S` (CR0, CR4, MSR_EFER, page sizes).

---

## Stage 3 – Long Mode Initialization (`_start64`)

Once the `ljmp` transfers control into 64-bit mode:

```
_start64 (boot.S):
  - Zero out legacy segment registers (DS/ES/FS/GS/SS)
  - Call `kernel_init` (from C)
  - If `kernel_init` ever returns, execute `cli; hlt` loop forever
```

At this point all execution is happening in the higher-half virtual address space (`KERNEL_VIRTUAL_START`).

---

## Stage 4 – C Kernel Bring-Up (`src/kernel/kernel.c`)

`kernel_init` is effectively the C-level entry point. Current responsibilities:

1. **Display Initialization**
   - `clearScreen()` and `setCursorAppearance()` from `screen.c`.
2. **Input Initialization**
   - `keyboard_init()` from `keyboard.c` (now responsible for both PS/2 fallback and USB HID setup).
3. **User Feedback**
   - Several `printAtPos()` calls to render the welcome banner.
4. **Idle Loop**
   - Calls `boot_os()` which currently spins (`while (1);`).

Pseudo timeline:

```
kernel_init:
  clearScreen()
  setCursorAppearance()
  keyboard_init()
  printAtPos("Welcome ...")
  ...
  boot_os()  --> infinite loop
```

---

## Stage 5 – Subsystem Details

### 5.1 Screen (`src/kernel/screen.c`)

- Writes directly to VGA text memory (`0xB8000`).
- Maintains cursor position, provides `print`, `clearScreen`, `moveCursor`, `printAtPos`.
- Uses port I/O helpers from `assembly.c` (`outb`).

### 5.2 Keyboard (`src/kernel/keyboard.c`)

```
keyboard_init():
  -> legacy_keyboard_init()    // enables PS/2 if present
  -> usb_ready = usb_keyboard_init()

keyboard_try_read_char():
  -> polls USB (if ready) and PS/2 buffer
  -> returns buffered characters through a ring buffer
```

Key collaborators:
- `drivers/usb/usb_keyboard.c` – enumerates USB HID keyboard via UHCI.
- `drivers/usb/uhci.c` – implements the UHCI host controller driver (control + interrupt transfers).
- `drivers/pci.c` – scans PCI bus to locate UHCI controllers.
- `assembly.c` – low-level port I/O primitives (`inb/outb/outw/outl`, `io_wait`).

ASCII view of the new USB stack:

```
┌────────────┐
│ keyboard.c │  buffer + API
└─────┬──────┘
      │ uses
┌─────▼───────────┐       ┌─────────────────────┐
│ usb_keyboard.c  │──────▶│ uhci.c (host driver)│
└─────┬───────────┘       └──────────┬──────────┘
      │ PCI scan                            │ MMIO/IO
┌─────▼───────────┐                       ┌──▼───────┐
│ pci.c           │◀──────────────────────│ assembly │
└─────────────────┘                       └──────────┘
```

### 5.3 Assembly Helpers (`src/kernel/assembly.c`)

Contains the inline assembly glue for `in{b,w,l}`, `out{b,w,l}`, and `io_wait`. These are essential for:
- Configuring UHCI I/O registers.
- Talking to PS/2 controller.
- Writing to VGA index/data registers.

---

## Stage 6 – Build Artifacts and Layout

The linker script (`kernel.ld`) arranges the sections so the kernel can be loaded at 4 MiB physical while executing in the higher-half address space. Important symbols exported:
- `_kernel_physical_start`, `_kernel_physical_end`
- `_kernel_virtual_start`, `_kernel_virtual_end`

The ISO build (`make iso`) places `kernel` at `/boot/kernel` and includes `grub.cfg` so GRUB can locate it.

```
iso/
 └── AuraOS-<VERSION>.iso
raw/
 └── boot/
     ├── kernel        (ELF binary)
     └── grub/
         └── grub.cfg  (menu entry)
```

---

## Quick Reference Timeline (Condensed)

| Stage | Responsible File(s) | Description |
|-------|---------------------|-------------|
| 0 | Firmware (external) | QEMU firmware picks the ISO and starts GRUB. |
| 1 | `grub.cfg` | GRUB hands off using Multiboot2 conventions. |
| 2a | `boot.S` (.multiboot) | Multiboot header recognized by GRUB. |
| 2b | `boot.S` (`_start`) | Paging setup, GDT install, long-mode jump. |
| 3 | `boot.S` (`_start64`) | Switches to C world via `kernel_init`. |
| 4 | `kernel.c` | Screen + keyboard init, banner print, idle loop. |
| 5 | `screen.c`, `keyboard.c`, `drivers/usb/*`, `drivers/pci.c`, `assembly.c` | Subsystems invoked from C entry point. |

---

## Where to Go Next

- For paging/GDT constants: `src/kernel/arch/x86_64/*.h`
- For device bring-up details: `src/kernel/drivers/`
- For build options: `src/kernel/Makefile`
- For GRUB setup on the ISO: top-level `Makefile`, `grub.cfg`

Feel free to extend this document with oscilloscope captures, additional ASCII art, or real images kept under `assets/` or `docs/images/` if you add them to the repository.

---

*Last updated: 01 Nov 2025*
