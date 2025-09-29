# USB Subsystem

This directory contains the USB stack for Aura OS. The structure splits host
controller implementations (UHCI/OHCI/EHCI/XHCI) from the shared core logic and
USB class drivers such as HID.

## Layout
- `core/`: Host-controller-agnostic helpers for enumeration, descriptor
  handling, transfer management, and device bookkeeping.
- `hci/`: Individual host controller drivers. Each controller lives inside its
  own subdirectory and exposes a small probe/ops interface back to the core.
- `class/`: USB class drivers. Only the HID boot protocol keyboard driver is
  expected at the moment.

## Development Notes
The stack is designed to enumerate devices in a tiered tree and issue standard
requests with strict bounds checking. Interrupt-driven transfers are preferred
when the platform configuration allows them; otherwise, the code should fall
back to time-limited polling loops.

## Limitations & TODO
- Core request handling, transfer rings, and descriptor parsing are not yet
  implemented.
- Host controller drivers currently only have scaffolding and no hardware
  interaction.
- HID keyboard driver is still a stub and does not feed the shell.
