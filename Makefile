ifeq ($(strip $(filter-out clean,$(MAKECMDGOALS))),)
sinclude scripts/env.mk


CROSS_COMPILE	:= riscv64-unknown-elf-
export CROSS_COMPILE

# What is -ffreestanding?
# https://gcc.gnu.org/onlinedocs/gcc/Standards.html
# The ISO C standard defines (in clause 4) two classes of conforming implementation.
# A conforming hosted implementation supports the whole standard including all the library facilities;
# a conforming freestanding implementation is only required to provide certain library facilities:
# those in <float.h>, <limits.h>, <stdarg.h>, and <stddef.h>;
# since AMD1, also those in <iso646.h>;
# since C99, also those in <stdbool.h> and <stdint.h>;
# and since C11, also those in <stdalign.h> and <stdnoreturn.h>.
# In addition, complex types, added in C99, are not required for freestanding implementations.

W_FLAGS		= -Wall -Werror=implicit-function-declaration -Wno-unused-function -Werror=return-type -Wno-unused-but-set-variable -Wno-unused-variable
X_CFLAGS	+= -std=gnu11 -O3 -g -ggdb \
				$(W_FLAGS) \
				-march=rv64g -mabi=lp64d -mcmodel=medany \
				-ffreestanding -fsigned-char \
				-fno-omit-frame-pointer -fno-common -nostdlib -mno-relax \
				-fno-pie
X_ASFLAGS	+= -std=gnu11 -O3 -g -ggdb \
				$(W_FLAGS) \
				-march=rv64g -mabi=lp64d -mcmodel=medany \
				-ffreestanding -fsigned-char \
				-fno-omit-frame-pointer -fno-common -nostdlib -mno-relax \
				-fno-pie

X_INCDIRS	+= include arch/include
X_LDFLAGS	+= -z max-page-size=4096 -T kernel.ld -no-pie -nostdlib

X_CLEAN		+= kernel.asm kernel.sym
NAME		:= kernel
SRC			+= arch/ arch/head.S arch/entry.S arch/context_switch.S arch/fpu.S \
				arch/start.c arch/vm.c arch/exception.c arch/device.c arch/syscall_table.c \
				init/*.c core/*.c core/class/*.c core/graphic/*.c core/sys/*.c core/ipc/*.c mm/*.c \
				fs/cpio/*.c fs/*.c \
				driver/*.c driver/virtio/*.c \
				lib/*.c lib/libc/*.c \
				lib/xjil/*.c

ifeq ("$(origin UT)", "command line")
X_DEFINES	+= UNIT_TEST=1
SRC			+= ut/
endif

# If the dependency files of arch/asm-offsets.gen are changed, must clean & rebuild.
X_PREPARE	:= arch/include/asm-offsets.h arch/include/syscall_table.h
X_CLEAN		:= arch/include/asm-offsets.h arch/include/syscall_table.h

MODULE		+= user-test

arch/include/asm-offsets.h: arch/asm-offsets.gen
	@echo GEN $@
	@$(CC) -xc $(X_CFLAGS) $(X_CPPFLAGS) -S -o $<.s $<
	@cat $<.s | sed -n '/^.ascii/s/\.ascii\ \"\([^"]*\)\"/\1/p' > $@
	@rm $<.s

arch/include/syscall_table.h: arch/syscall_gen.py
	@echo GEN $@
	@arch/syscall_gen.py > $@

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
	@$(if $(wildcard rootfs),:,@echo [MKDIR] rootfs && mkdir rootfs)
	@cd rootfs && $(FIND) . -not -name . | $(CPIO) > ../fs.img

cloc:
	cloc . --include-ext=c,h,S --exclude-dir=scripts,rootfs --by-file

FORCE: ;

.PHONY : FORCE
endif