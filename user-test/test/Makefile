X_CLEAN		= $(obj)/*.asm $(obj)/*.sym

NAME		:= test
SRC			= test.c

define CUSTOM_TARGET_CMD
echo [OUTPUT] $@; \
$(LD) $(X_LDFLAGS) $(X_OBJS) $(patsubst %, -L%, $(X_LIBDIRS)) $(patsubst %, -l:lib%.a, $(X_LIBS)) -o $@.o; \
$(OC) -O binary -S $@.o $@; \
$(OD) -S $@.o > $(obj)/test.asm; \
$(OD) -t $@.o | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $@.o.sym; \
cp $(obj)/test $(obj)/../../rootfs
endef