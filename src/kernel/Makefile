CC=gcc
SHARED_FLAGS = -fno-builtin -O2 -s -nostdinc -nostdlib -ffreestanding -Wall -Wextra -Werror -I. -Ilibs -Iglibc -Iinclude -MMD -mno-red-zone -mcmodel=kernel -fno-pie
CFLAGS = $(SHARED_FLAGS)
ASFLAGS = $(SHARED_FLAGS) -Wa,--divide

OBJS := boot.o
OBJS += assembly.o
OBJS += kernel.o
OBJS += keyboard.o
OBJS += screen.o

DFILES = $(patsubst %.o,%.d,$(OBJS))

all: kernel

kernel: $(OBJS) kernel.ld Makefile
	$(CC) -z max-page-size=0x1000 $(CFLAGS) -no-pie -Wl,--build-id=none -s -T kernel.ld -o $@ $(OBJS)

clean:
	find -name "*~" -delete
	rm -rf $(OBJS) $(DFILES) kernel

$(OBJS): Makefile
-include $(DFILES)
