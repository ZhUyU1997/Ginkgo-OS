#include <syscall_table.h>
#include <log.h>

void sys_ni_syscall(void)
{
	PANIC("invalid syscall");
	return;
}

#define SYSCALL_DEC(x) extern void sys_##x()

SYSCALL_DEC(ni_syscall);
SYSCALL_DEC(putc);
SYSCALL_DEC(process_create);
SYSCALL_DEC(thread_create);
SYSCALL_DEC(vmo_create);
SYSCALL_DEC(vmo_write);
SYSCALL_DEC(vmo_read);
SYSCALL_DEC(vmo_map);
SYSCALL_DEC(nanosleep);
SYSCALL_DEC(clock_get);
SYSCALL_DEC(clock_get_monotonic);
SYSCALL_DEC(ticks_get);
SYSCALL_DEC(ticks_per_second);
SYSCALL_DEC(deadline_after);

#define SYSCALL_ITEM(x) [__NR_##x] = sys_##x

void *sys_call_table[__NR_syscalls] = {
	SYSCALL_ITEM(ni_syscall),
	SYSCALL_ITEM(putc),
	SYSCALL_ITEM(process_create),
	SYSCALL_ITEM(thread_create),
	SYSCALL_ITEM(vmo_create),
	SYSCALL_ITEM(vmo_write),
	SYSCALL_ITEM(vmo_read),
	SYSCALL_ITEM(vmo_map),
	SYSCALL_ITEM(nanosleep),
	SYSCALL_ITEM(clock_get),
	SYSCALL_ITEM(clock_get_monotonic),
	SYSCALL_ITEM(ticks_get),
	SYSCALL_ITEM(ticks_per_second),
	SYSCALL_ITEM(deadline_after),
};