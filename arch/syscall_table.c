#include <log.h>

void sys_ni_syscall(void)
{
	PANIC("invalid syscall");
	return;
}

#define SYSCALL_IMPL

#include <syscall_table.h>
