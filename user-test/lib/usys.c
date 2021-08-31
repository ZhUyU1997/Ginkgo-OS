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

SYSFUNC_DEF(console_putc);
SYSFUNC_DEF(console_puts);
SYSFUNC_DEF(process_create);
SYSFUNC_DEF(process_exit);
SYSFUNC_DEF(vmo_create);
SYSFUNC_DEF(vmo_write);
SYSFUNC_DEF(vmo_read);
SYSFUNC_DEF(vmo_map);
SYSFUNC_DEF(register_server);
SYSFUNC_DEF(register_named_server);
SYSFUNC_DEF(register_client);
SYSFUNC_DEF(register_client_by_name);
SYSFUNC_DEF(ipc_call);
SYSFUNC_DEF(ipc_return);
SYSFUNC_DEF(nanosleep);
SYSFUNC_DEF(clock_get);
SYSFUNC_DEF(clock_get_monotonic);
SYSFUNC_DEF(ticks_get);
SYSFUNC_DEF(ticks_per_second);
SYSFUNC_DEF(deadline_after);
SYSFUNC_DEF(yield);
SYSFUNC_DEF(futex_wait);
SYSFUNC_DEF(futex_wake);
SYSFUNC_DEF(block_read);
SYSFUNC_DEF(block_write);
SYSFUNC_DEF(block_capacity);
SYSFUNC_DEF(block_size);
SYSFUNC_DEF(block_count);