sinclude ../scripts/env.mk

CROSS_COMPILE	:= riscv64-linux-gnu-
export CROSS_COMPILE

W_FLAGS		= -Wall -Werror=implicit-function-declaration -Wno-unused-function -Werror=return-type
X_CFLAGS	= -std=gnu11 -O3 -g -ggdb \
				$(W_FLAGS) \
				-march=rv64g -mabi=lp64d -mcmodel=medany \
				-ffreestanding -fsigned-char \
				-fno-omit-frame-pointer -fno-common -nostdlib -mno-relax \
				-fno-pie

X_INCDIRS	= $(src)/include
X_LDFLAGS	= -z max-page-size=4096  -N -T $(src)/user.ld -no-pie -nostdlib
X_LIBS		= user
X_LIBDIRS	= $(src)/lib


MODULE		= lib test

test: lib

X_PREPARE	:= $(src)/include/syscall_table.h

$(src)/include/syscall_table.h: $(src)/../arch/include/syscall_table.h
	@echo CP $@
	@cp $< $@