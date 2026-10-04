# uefix64

A minimal x64 UEFI application that prints `welcome`.

It is a single freestanding C file (`main.c`) with only the UEFI definitions it
needs, so it builds with plain clang + lld and no gnu-efi or EDK II.

## Build

Requires `clang` and `lld-link` (from LLVM/lld).

```sh
make
```

This produces `BOOTX64.EFI`, a PE32+ EFI application.

## Run

With `qemu-system-x86_64` and OVMF installed (`apt install qemu-system-x86 ovmf`):

```sh
make run
```

`make run` copies the image to `esp/EFI/BOOT/BOOTX64.EFI`, the removable-media
boot path, so the firmware boots it automatically. Override the firmware path
with `make run OVMF=/path/to/OVMF_CODE.fd` if yours is elsewhere.

To boot on real hardware, copy `BOOTX64.EFI` to `EFI/BOOT/` on a FAT32 USB
drive and boot it with Secure Boot disabled.
