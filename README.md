# uefix64

A minimal UEFI application that prints `welcome` and waits for a key press.
It builds for x86-64 (`BOOTX64.EFI`) and ARM64/AArch64 (`BOOTAA64.EFI`).

It is a single freestanding C file (`main.c`) with only the UEFI definitions it
needs, so it builds with plain clang + lld and no gnu-efi or EDK II.

## Build

Requires `clang` and `lld-link` (from LLVM/lld).

```sh
make              # x86-64 -> BOOTX64.EFI
make ARCH=aa64    # ARM64  -> BOOTAA64.EFI
```

Both are PE32+ EFI applications. The file names are the ones UEFI firmware
looks for on removable media (`EFI/BOOT/BOOTX64.EFI` / `EFI/BOOT/BOOTAA64.EFI`),
so firmware boots them automatically.

## Run in QEMU (Linux)

Install QEMU and the UEFI firmware:

```sh
sudo apt install qemu-system-x86 ovmf               # for x86-64
sudo apt install qemu-system-arm qemu-efi-aarch64   # for ARM64
```

Then:

```sh
make run              # x86-64, in a QEMU window
make run ARCH=aa64    # ARM64, in your terminal
```

`make run` copies the image into `esp/EFI/BOOT/` and boots that folder as a
FAT drive. The program prints `welcome` and `Press any key to exit.`, then
returns to the firmware, which opens its setup menu.

- To see the x86-64 output in your terminal instead of a window, use
  `make run QEMU_ARGS=-nographic`.
- The ARM64 run always uses the terminal, because QEMU's `virt` machine has no
  display. Quit QEMU with `Ctrl-A` then `X`.
- If your firmware is somewhere else, pass `OVMF=/path/to/firmware.fd`.

## Run in QEMU (macOS)

This works on both Apple Silicon and Intel Macs.

1. Install the tools with [Homebrew](https://brew.sh). Homebrew's QEMU ships
   the x86-64 and ARM64 UEFI firmware (EDK II), so you don't need separate
   firmware packages.

   ```sh
   brew install qemu llvm lld
   ```

2. Build with Homebrew's clang and lld-link. Apple's Xcode clang has no
   `lld-link`, and Homebrew keeps its LLVM off the `PATH`.

   ```sh
   CC="$(brew --prefix llvm)/bin/clang" LD="$(brew --prefix lld)/bin/lld-link"
   make CC="$CC" LD="$LD"              # x86-64
   make CC="$CC" LD="$LD" ARCH=aa64    # ARM64
   ```

3. Boot it, pointing `OVMF` at the firmware that came with QEMU.

   x86-64:

   ```sh
   make run CC="$CC" LD="$LD" OVMF="$(brew --prefix qemu)/share/qemu/edk2-x86_64-code.fd"
   ```

   A QEMU window opens and, after a few seconds of firmware startup, shows
   `welcome`. Click inside the window so it has keyboard focus before pressing
   a key. Add `QEMU_ARGS=-nographic` to use the terminal instead. On Apple
   Silicon, QEMU emulates the x86-64 CPU in software, so boot is slower.

   ARM64:

   ```sh
   make run CC="$CC" LD="$LD" ARCH=aa64 OVMF="$(brew --prefix qemu)/share/qemu/edk2-aarch64-code.fd"
   ```

   The output appears in your terminal; quit with `Ctrl-A` then `X`. On Apple
   Silicon you can run it on the real CPU instead of emulating one by adding
   `QEMU_ARGS="-accel hvf -cpu host"`.

## Run on real hardware

### x86-64 PC

Copy `BOOTX64.EFI` to `EFI/BOOT/` on a FAT32 USB drive and boot from it with
Secure Boot disabled.

### Raspberry Pi 5

The Pi 5 has an ARM64 CPU, so use the `BOOTAA64.EFI` build. It doesn't come
with UEFI firmware, so you need to install some first. These steps haven't been
tested on a real Pi 5.

1. Install a UEFI firmware for the Pi 5 on a FAT32-formatted microSD card,
   following that project's instructions. The usual choice is the community
   EDK II port (search GitHub for `rpi5-uefi`); it's experimental, so check that
   it's still maintained. U-Boot can also run UEFI programs (`bootefi`), but its
   Pi 5 support is newer and less complete.
2. Build with `make ARCH=aa64` and copy `BOOTAA64.EFI` to `EFI/BOOT/` on the
   same card (or on a FAT32 USB drive).
3. Connect an HDMI display and a USB keyboard, then power on. The firmware
   should boot `EFI/BOOT/BOOTAA64.EFI`, show `welcome`, and wait for a key. If
   it doesn't start automatically, pick the card or drive from the firmware's
   **Boot Manager**.
