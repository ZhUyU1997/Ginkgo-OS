sinclude ../scripts/env.mk

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
X_LDFLAGS	+= -z max-page-size=4096  -N -e main -Ttext 0x20000000 -no-pie -nostdlib

X_CLEAN		+= $(obj)/*.asm $(obj)/*.sym

NAME		:= test
SRC			+= test.c usys.c

define CUSTOM_TARGET_CMD
echo [OUTPUT] $@; \
$(LD) $(X_LDFLAGS) -o $@.o $(X_OBJS); \
$(OC) -O binary -S $@.o $@; \
$(OD) -S $@.o > $(obj)/test.asm; \
$(OD) -t $@.o | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $@.o.sym; \
cp $(obj)/test $(obj)/../rootfs
endef