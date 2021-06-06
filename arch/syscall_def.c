#include "build-asm.h"

#define SYSCALL(name) DEFINE(__NR_##name, index++);

void asm_offsets(void)
{
	int index = 0;
	SYSCALL(ni_syscall);
	SYSCALL(putstring);
	SYSCALL(process_create);
	SYSCALL(thread_create);
	SYSCALL(vmo_create);
	SYSCALL(vmo_write);
	SYSCALL(vmo_read);
	SYSCALL(vmo_map);
	SYSCALL(nanosleep);
	SYSCALL(clock_get);
	SYSCALL(clock_get_monotonic);
	SYSCALL(ticks_get);
	SYSCALL(ticks_per_second);
	SYSCALL(deadline_after);
	SYSCALL(syscalls);
}