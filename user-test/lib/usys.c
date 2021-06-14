#include "syscall.h"
#include <syscall_table.h>

#define PP_HELP(x) #x
#define SYSFUNC_DEF(name) _SYSFUNC_DEF_(usys_##name, __NR_##name)
#define _SYSFUNC_DEF_(name, nr) __SYSFUNC_DEF__(name, nr)
#define __SYSFUNC_DEF__(name, nr)    \
    asm ( \
        ".global " #name "	\n\t" \
        #name ":\n\t" \
        "li a7, " PP_HELP(nr) "\n\t" \
        "ecall\n\t "	\
        "ret\n\t" \
        ".type	" #name ",	@function \n\t" \
	    ".size " #name", .-"#name "\n\t")

SYSFUNC_DEF(putc);
SYSFUNC_DEF(process_create);
SYSFUNC_DEF(process_exit);
SYSFUNC_DEF(vmo_create);
SYSFUNC_DEF(vmo_write);
SYSFUNC_DEF(vmo_read);
SYSFUNC_DEF(vmo_map);