ifeq ($(strip $(filter-out clean,$(MAKECMDGOALS))),)
sinclude scripts/env.mk
endif

CROSS_COMPILE	:= riscv64-linux-gnu-
export CROSS_COMPILE

X_CFLAGS	+= -std=gnu11 -O0 -g -ggdb \
				-Wall -Werror \
				-march=rv64g -mabi=lp64d -mcmodel=medany \
				-ffreestanding -fsigned-char \
				-fno-omit-frame-pointer -fno-common -nostdlib -mno-relax \
				-fno-pie

X_LDFLAGS	+= -z max-page-size=4096 -T kernel.ld -no-pie -nostdlib

NAME	:= kernel
SRC		+= *.S *.c

define CUSTOM_TARGET_CMD
echo [KERNEL] $@; \
$(LD) $(X_LDFLAGS) -o $@ $(X_OBJS); \
$(OD) -S kernel > kernel.asm; \
$(OD) -t kernel | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > kernel.sym
endef

QEMU		= qemu-system-riscv64
QEMUOPTS	= -machine virt -bios none -kernel kernel -m 128M -smp 1 -nographic
# QEMUOPTS += -drive file=fs.img,if=none,format=raw,id=x0
# QEMUOPTS += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

qemu:
	$(QEMU) $(QEMUOPTS)