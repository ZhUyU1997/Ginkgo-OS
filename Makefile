ifeq ($(strip $(filter-out clean,$(MAKECMDGOALS))),)
sinclude scripts/env.mk


CROSS_COMPILE	:= riscv64-linux-gnu-
export CROSS_COMPILE

W_FLAGS		= -Wall -Werror=implicit-function-declaration -Wno-unused-function -Werror=return-type
X_CFLAGS	+= -std=gnu11 -O3 -g -ggdb \
				$(W_FLAGS) \
				-march=rv64g -mabi=lp64d -mcmodel=medany \
				-ffreestanding -fsigned-char \
				-fno-omit-frame-pointer -fno-common -nostdlib -mno-relax \
				-fno-pie
				
X_INCDIRS	+= include
X_LDFLAGS	+= -z max-page-size=4096 -T kernel.ld -no-pie -nostdlib

X_CLEAN		+= kernel.asm kernel.sym
NAME		:= kernel
SRC			+= arch/ arch/head.S arch/entry.S arch/context_switch.S \
				arch/start.c arch/vm.c arch/exception.c arch/device.c \
				init/*.c core/*.c core/class/*.c mm/*.c \
				fs/cpio/*.c fs/*.c \
				driver/*.c driver/virtio/*.c \
				lib/*.c lib/libc/*.c \
				lib/xjil/*.c

define CUSTOM_TARGET_CMD
echo [KERNEL] $@; \
$(LD) $(X_LDFLAGS) -o $@ $(X_OBJS); \
$(OD) -S kernel > kernel.asm; \
$(OD) -t kernel | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > kernel.sym
endef

else

QEMU		= qemu-system-riscv64
QEMUOPTS	= -machine virt -bios none -kernel kernel -m 128M -smp 1 -serial stdio
# QEMUOPTS	+= -nographic
QEMUOPTS	+= -drive file=fs.img,if=none,format=raw,id=x0
QEMUOPTS	+= -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0
QEMUOPTS	+= -device virtio-gpu-device,bus=virtio-mmio-bus.1
QEMUOPTS	+= -device virtio-net-device,bus=virtio-mmio-bus.2
QEMUOPTS	+= -device virtio-tablet-device,bus=virtio-mmio-bus.3
QEMUOPTS	+= -device virtio-keyboard-device,bus=virtio-mmio-bus.4
QEMUOPTS	+= -device virtio-mouse-device,bus=virtio-mmio-bus.5

# When using -nographic, press 'ctrl-a h' to get some help.
# C-a h    print this help
# C-a x    exit emulator
# C-a s    save disk data back to file (if -snapshot)
# C-a t    toggle console timestamps
# C-a b    send break (magic sysrq)
# C-a c    switch between console and monitor
# C-a C-a  sends C-a

qemu: fs.img
	@echo [RUN] $@;
	@$(QEMU) $(QEMUOPTS)

qemu-gdb: fs.img
	@echo [RUN] $@;
	@$(QEMU) $(QEMUOPTS) -S -gdb tcp::10001,ipv4

FIND		:=	find
CPIO		:=	cpio -o -H newc --quiet

fs.img: FORCE
	@echo [CPIO] $@;
	@cd rootfs && $(FIND) . -not -name . | $(CPIO) > ../fs.img

FORCE: ;

.PHONY : FORCE
endif