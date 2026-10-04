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

## Testing with QEMU on macOS

This works on both Apple Silicon and Intel Macs. On Apple Silicon, QEMU emulates
the x86-64 CPU in software, so boot takes a few seconds longer.

1. Install the tools with [Homebrew](https://brew.sh). Homebrew's QEMU ships
   the x86-64 UEFI firmware (EDK II/OVMF), so you don't need a separate OVMF
   package.

   ```sh
   brew install qemu llvm lld
   ```

2. Build with Homebrew's clang and lld-link. Apple's Xcode clang has no
   `lld-link`, and Homebrew keeps its LLVM off the `PATH`.

   ```sh
   make CC="$(brew --prefix llvm)/bin/clang" LD="$(brew --prefix lld)/bin/lld-link"
   ```

3. Boot it, pointing `OVMF` at the firmware that came with QEMU:

   ```sh
   make run OVMF="$(brew --prefix qemu)/share/qemu/edk2-x86_64-code.fd"
   ```

   A QEMU window opens and `welcome` appears after the firmware starts the
   program. The firmware then opens its setup menu; close the window to quit.

   To see the output in your terminal instead of a window, add `-nographic` to
   the `qemu-system-x86_64` line in the `Makefile`, or run QEMU directly:

   ```sh
   mkdir -p esp/EFI/BOOT && cp BOOTX64.EFI esp/EFI/BOOT/
   qemu-system-x86_64 \
     -drive if=pflash,format=raw,readonly=on,file="$(brew --prefix qemu)/share/qemu/edk2-x86_64-code.fd" \
     -drive format=raw,file=fat:rw:esp -net none -nographic
   ```

   Press `Ctrl-A` then `X` to quit.
