OS_NAME := AuraOS
OS_VERSION := $(shell cat VERSION)
BUILD_DIR := out
ISO_DIR := $(BUILD_DIR)/iso
SYSROOT := $(BUILD_DIR)/sysroot
KERNEL_ELF := $(BUILD_DIR)/kernel.elf
ISO_FILE := $(ISO_DIR)/$(OS_NAME)-$(OS_VERSION).iso
GRUB_MKRESCUE := $(shell command -v grub-mkrescue || command -v grub2-mkrescue)

CC := gcc
LD := ld
OBJCOPY := objcopy

CFLAGS := -ffreestanding -fno-stack-protector -fno-pic -m64 -mno-red-zone -mcmodel=kernel \
          -Wall -Wextra -Werror -std=gnu11 -g -I$(abspath src/include)
LDFLAGS := -nostdlib -z max-page-size=0x1000

KERNEL_SRCS := $(shell find src/kernel -name '*.c')
KERNEL_ASM := $(shell find src -name '*.S')
KERNEL_OBJS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(KERNEL_SRCS)) \
               $(patsubst src/%.S,$(BUILD_DIR)/%.o,$(KERNEL_ASM))

.PHONY: all kernel iso qemu clean distclean directories

all: iso

kernel: directories $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) -T src/kernel/linker.ld -o $(KERNEL_ELF) $(KERNEL_OBJS)

iso: kernel
	@test -n "$(GRUB_MKRESCUE)" || { echo "grub-mkrescue is required"; exit 1; }
	mkdir -p $(SYSROOT)/boot/grub
	cp $(KERNEL_ELF) $(SYSROOT)/boot/kernel.elf
	cp grub.cfg $(SYSROOT)/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o $(ISO_FILE) $(SYSROOT)

qemu: iso
	qemu-system-x86_64 -m 256M -serial stdio -cdrom $(ISO_FILE) -machine pc \
		-device qemu-xhci -device usb-kbd -device usb-mouse

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: src/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

directories:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

