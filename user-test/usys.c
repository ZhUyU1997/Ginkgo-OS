#include "syscall.h"

#define PP_HELP(x) #x
#define SYSFUNC_DEF(name) _SYSFUNC_DEF_(name, __NR_##name)
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

SYSFUNC_DEF(putstring);