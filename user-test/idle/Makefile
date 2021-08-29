X_CLEAN		= $(obj)/*.asm $(obj)/*.sym

NAME		:= idle
SRC			= test.c

X_PREPARE	+= $(obj)/../lib/libuser.a

define CUSTOM_TARGET_CMD
echo [OUTPUT] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) $(patsubst %, -L%, $(X_LIBDIRS)) $(patsubst %, -l:lib%.a, $(X_LIBS)) -o $@.o; \
$(OC) -O binary -S $@.o $@; \
$(OD) -S $@.o > $@.o.asm; \
$(OD) -t $@.o | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $@.o.sym; \
cp $@ $(obj)/../../rootfs
endef