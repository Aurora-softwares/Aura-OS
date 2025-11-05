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
![Last Commit](https://img.shields.io/github/last-commit/Aurora-softwares/Aura-OS?style=for-the-badge)
![Repo Size](https://img.shields.io/github/repo-size/Aurora-Softwares/Aura-OS?style=for-the-badge)
![Current Issues](https://img.shields.io/github/issues/Aurora-softwares/Aura-OS?style=for-the-badge)

![Total Languages](https://img.shields.io/github/languages/count/Aurora-softwares/Aura-OS?style=for-the-badge)
![Top Language](https://img.shields.io/github/languages/top/Aurora-Softwares/Aura-OS?style=for-the-badge)
![Vulnerabilities](https://img.shields.io/snyk/vulnerabilities/github/Aurora-softwares/Aura-OS?style=for-the-badge)

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

## Input Devices
- Initial USB HID keyboard support via UHCI host controllers (works in QEMU with `-device usb-kbd`); the legacy PS/2 path remains available as a fallback.


# Build Requirements

To build the Operating system you will need to meet the following requirement;

## Requirements
  - Windows 10 or later 64-bit
  - [Visual Studio Code (latest)](https://code.visualstudio.com/)
  - [Windows Subsystem For Linux (WSL) ubuntu](https://learn.microsoft.com/en-us/windows/wsl/install)
    - `wsl --install -d Ubuntu`
    - `wsl sudo apt -y update`
    - `wsl sudo apt upgrade`
  - [Make (wsl)]()
    - `wsl sudo apt-get install make`
  - [QEMU (8.0.0)](https://qemu.weilnetz.de/w64/qemu-w64-setup-20250826.exe)
    - Downloaded to `C:\Program Files\qemu`
  - [MinGW-w64 (11.2.0)](https://netix.dl.sourceforge.net/project/mingw-w64/mingw-w64/mingw-w64-release/mingw-w64-v11.0.0.zip?viasf=1)
    - Downloaded to `C:\Program Files\mingw64`

Once you clone the repository to build the OS, it is as easy as opening it in VS Code and pressing F5! That will start the debug process and launch the os in QEMU once completed.

# Licence

Please observe the Apache 2.0 license that is listed in this repository.