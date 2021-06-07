#include <syscall_table.h>
#include <errno.h>
#include <core/vmo.h>
#include <core/syscall.h>

#undef __SYSCALL
#define __SYSCALL(nr, call) [nr] = (call),

long sys_ni_syscall(void)
{
	return -1;
}

extern void sys_putc(char c);
extern void *sys_create();
extern void sys_present();
extern int sys_process_create();
extern int sys_thread_create();

void *sys_call_table[__NR_syscalls] = {
	[__NR_ni_syscall] = sys_ni_syscall,
	[__NR_putc] = sys_putc,
	[__NR_process_create] = sys_process_create,
	[__NR_thread_create] = sys_thread_create,

	[__NR_vmo_create] = sys_vmo_create,
	[__NR_vmo_write] = sys_vmo_write,
	[__NR_vmo_read] = sys_vmo_read,
	[__NR_vmo_map] = sys_vmo_map,

	[__NR_nanosleep] = sys_nanosleep,
	[__NR_clock_get] = sys_clock_get,
	[__NR_clock_get_monotonic] = sys_clock_get_monotonic,
	[__NR_ticks_get] = sys_ticks_get,
	[__NR_ticks_per_second] = sys_ticks_per_second,
	[__NR_deadline_after] = sys_deadline_after,
};