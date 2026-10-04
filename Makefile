CC      := clang
LD      := lld-link

CFLAGS  := -target x86_64-unknown-windows \
           -ffreestanding -fshort-wchar -fno-stack-protector \
           -mno-red-zone -Wall -Wextra -O2
LDFLAGS := -subsystem:efi_application -nodefaultlib -entry:efi_main

TARGET  := BOOTX64.EFI

all: $(TARGET)

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): main.o
	$(LD) $(LDFLAGS) $< -out:$@

# Boot the image in QEMU with OVMF firmware (needs qemu-system-x86_64 and OVMF).
OVMF ?= /usr/share/OVMF/OVMF_CODE_4M.fd

run: $(TARGET)
	mkdir -p esp/EFI/BOOT
	cp $(TARGET) esp/EFI/BOOT/
	qemu-system-x86_64 -drive if=pflash,format=raw,readonly=on,file=$(OVMF) \
		-drive format=raw,file=fat:rw:esp -net none

clean:
	rm -rf main.o $(TARGET) esp

.PHONY: all run clean
