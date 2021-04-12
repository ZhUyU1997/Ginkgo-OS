#include <syscall.h>
#include <errno.h>

#undef __SYSCALL
#define __SYSCALL(nr, call)	[nr] = (call),

long sys_ni_syscall(void)
{
	return -1;
}

extern long sys_putstring(const char *s);
void *sys_call_table[__NR_syscalls] = {
	[0 ... __NR_syscalls - 1] = sys_ni_syscall,
    [__NR_putstring] = sys_putstring,
};