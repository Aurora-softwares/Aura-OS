<div align="center">

<img src="https://raw.githubusercontent.com/Aurora-softwares/Aura-OS/main/docs/images/aura.png" width="400px">

**An all new OS, entirely made from the ground up.**

[![license](https://img.shields.io/badge/License-Apache%202.0-blue?style=for-the-badge)](https://github.com/Aurora-softwares/Aura-OS/blob/main/LICENSE)
![OS Version](https://img.shields.io/badge/OS_Version-0.0.1-green?style=for-the-badge)

[![NASM](https://img.shields.io/badge/NASM-2.16.01-green?style=for-the-badge)](https://www.nasm.us/pub/nasm/releasebuilds/?C=M;O=D)
[![QEMU](https://img.shields.io/badge/QEMU-8.0.0-green?style=for-the-badge)](https://www.qemu.org/download/#windows)
[![GCC](https://img.shields.io/badge/GCC-11.2.0-green?style=for-the-badge)](https://winlibs.com/)
[![MinGW](https://img.shields.io/badge/MinGW-10.0.0-green?style=for-the-badge)](https://winlibs.com/)

![All contributers](https://img.shields.io/github/contributors/Aurora-softwares/Aura-OS?style=for-the-badge)
![All contributers](https://img.shields.io/github/last-commit/Aurora-softwares/Aura-OS?style=for-the-badge)
![All contributers](https://img.shields.io/github/repo-size/Aurora-Softwares/Aura-OS?style=for-the-badge)
![All contributers](https://img.shields.io/github/issues/Aurora-softwares/Aura-OS?style=for-the-badge)

![All contributers](https://img.shields.io/github/languages/count/Aurora-softwares/Aura-OS?style=for-the-badge)
![All contributers](https://img.shields.io/github/languages/top/Aurora-Softwares/Aura-OS?style=for-the-badge)
![All contributers](https://img.shields.io/snyk/vulnerabilities/github/Aurora-softwares/Aura-OS?style=for-the-badge)

______________________________________________________________________

<p align="center">
  <a href="#what-is-aura-os">What is Aura OS</a> •
  <a href="#requirements">Software Requirements</a> •
  <a href="https://github.com/Aurora-softwares/Aura-OS/wiki/">Docs</a> •
  <a href="#license">License</a>
</p>

______________________________________________________________________

</div>

# What is Aura OS

Aurora OS is a project OS, meaning it has no end goal at the moment other than to be a usable, graphical OS. To get it to be graphical there will be many hurdles to overcome. Hopefully this repo can show that if you put you mind to building what you what to build and research hard enough it can be done.

# Build Requirements

To build the Operating system you will need to meet the following requirement;

## Requirements
 - Windows 10 or later 64-bit
 - [Visual Studio Code (latest)](https://code.visualstudio.com/)
 - Supplied
   - NASM (2.16.01)
   - QEMU (8.0.0)
   - MinGW-w64 (11.2.0)

Once you clone the repository to build the OS, it is as easy as opening it in VS Code and pressing F5! That will start the debug process and launch the os in QEMU once completed.

# Licence

Please observe the Apache 2.0 license that is listed in this repository. In addition, the Lightning framework is Patent Pending.
## Roadmap: USB, Video, and Shell Overhaul

> **Status:** These features are currently under active development on the
> `feature/usb-video-shell` branch. The codebase does not yet include a USB host
> stack, native-resolution video pipeline, or interactive shell. The sections
> below capture the intended workflow once the implementation is complete.

### Building (planned)
- `make qemu-bios` – build a BIOS-compatible image targeting legacy VBE modes.
- `make qemu-uefi` – build an image that boots under OVMF/UEFI using GOP.

### Running in QEMU (planned)
```bash
# BIOS boot (planned)
qemu-system-x86_64 -m 512 -enable-kvm -d int -no-reboot \
  -drive format=raw,file=build/aura-bios.img \
  -device usb-kbd

# UEFI boot (planned)
qemu-system-x86_64 -m 512 -enable-kvm -d int -no-reboot \
  -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE.fd \
  -drive if=pflash,format=raw,file=build/ovmf_vars.fd \
  -drive format=raw,file=build/aura-uefi.img \
  -device usb-kbd
```

### Shell test flow (planned)
Once the shell is implemented, the following commands will be available on the
primary console:

| Command  | Description |
|----------|-------------|
| `help`   | List all registered commands with a short synopsis. |
| `echo`   | Echo the supplied text back to the console. |
| `cls`/`clear` | Clear the framebuffer console. |
| `reboot` | Trigger a warm reboot. |
| `meminfo`| Show basic memory usage statistics. |
| `lsusb`  | Display detected USB devices in `bus:addr vendor:product class` format. |
| `vidinfo`| Print the current video mode. |

Refer to `tests/manual.md` for the full manual validation checklist.
