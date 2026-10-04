# Build for x86-64 by default; use `make ARCH=aa64` for ARM64 (AArch64).
ARCH    ?= x64

CC      := clang
LD      := lld-link

ifeq ($(ARCH),x64)
  CLANG_TARGET := x86_64-unknown-windows
  ARCH_CFLAGS  := -mno-red-zone
  LD_MACHINE   := x64
  TARGET       := BOOTX64.EFI
  QEMU         := qemu-system-x86_64
  OVMF         ?= /usr/share/OVMF/OVMF_CODE_4M.fd
  QEMU_ARGS    ?=
  QEMU_DISK    := -drive format=raw,file=fat:rw:esp
else ifeq ($(ARCH),aa64)
  CLANG_TARGET := aarch64-unknown-windows
  ARCH_CFLAGS  :=
  LD_MACHINE   := arm64
  TARGET       := BOOTAA64.EFI
  QEMU         := qemu-system-aarch64
  OVMF         ?= /usr/share/AAVMF/AAVMF_CODE.fd
  QEMU_ARGS    ?= -cpu max
  # The virt machine has no display, so the firmware console is the serial port.
  QEMU_DISK    := -M virt -m 512 -nographic \
                  -drive if=virtio,format=raw,file=fat:rw:esp
else
  $(error Unsupported ARCH '$(ARCH)'; use x64 or aa64)
endif

CFLAGS  := -target $(CLANG_TARGET) \
           -ffreestanding -fshort-wchar -fno-stack-protector \
           $(ARCH_CFLAGS) -Wall -Wextra -O2
LDFLAGS := -subsystem:efi_application -nodefaultlib -entry:efi_main \
           -machine:$(LD_MACHINE)

OBJ     := main-$(ARCH).o

all: $(TARGET)

$(OBJ): main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(LD) $(LDFLAGS) $< -out:$@

# Boot the image in QEMU with UEFI firmware (OVMF for x64, AAVMF for aa64).
run: $(TARGET)
	rm -rf esp
	mkdir -p esp/EFI/BOOT
	cp $(TARGET) esp/EFI/BOOT/
	$(QEMU) -drive if=pflash,format=raw,readonly=on,file=$(OVMF) \
		$(QEMU_DISK) -net none $(QEMU_ARGS)

clean:
	rm -rf main-*.o BOOTX64.EFI BOOTAA64.EFI esp

.PHONY: all run clean
