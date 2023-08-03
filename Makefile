OS_NAME=AuraOS
OS_VERSION=$(shell cat VERSION)
ISO_FILE=$(OS_NAME)-$(OS_VERSION).iso

.PHONY: all
.PHONY: kernel
.PHONY: qemu
.PHONY: iso
.PHONY: clean

ifneq (, $(shell which grub2-mkrescue 2> /dev/null))
  GRUB_MKRESCUE = grub2-mkrescue
else ifneq (, $(shell which grub-mkrescue 2> /dev/null))
  GRUB_MKRESCUE = grub-mkrescue
else
    $(error "Cannot find grub-mkrescue or grub2-mkrescue")
endif

all: kernel

kernel:
	make -C src/kernel all

iso:
	mkdir -p out/iso/
	mkdir -p out/raw/boot/grub/
	cp grub.cfg out/raw/boot/grub/
	cp src/kernel/kernel out/raw/boot/
	$(GRUB_MKRESCUE) -o ./out/iso/$(ISO_FILE) ./out/raw

qemu:
	qemu-system-x86_64 -cdrom ./out/iso/$(ISO_FILE) -serial stdio -m 1024M

clear:
	make -C src/kernel clean

clean: clear
	rm -rf out/