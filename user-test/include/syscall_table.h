#pragma once
#define __NR_ni_syscall 0
#define __NR_putc 1
#define __NR_process_create 2
#define __NR_process_exit 3
#define __NR_thread_create 4
#define __NR_thread_exit 5
#define __NR_vmo_create 6
#define __NR_vmo_write 7
#define __NR_vmo_read 8
#define __NR_vmo_map 9
#define __NR_register_server 10
#define __NR_register_named_server 11
#define __NR_register_client 12
#define __NR_register_client_by_name 13
#define __NR_ipc_call 14
#define __NR_ipc_return 15
#define __NR_nanosleep 16
#define __NR_clock_get 17
#define __NR_clock_get_monotonic 18
#define __NR_ticks_get 19
#define __NR_ticks_per_second 20
#define __NR_deadline_after 21
#define __NR_yield 22
#define __NR_syscalls 23

#ifdef SYSCALL_IMPL

extern void sys_ni_syscall();
extern void sys_putc();
extern void sys_process_create();
extern void sys_process_exit();
extern void sys_thread_create();
extern void sys_thread_exit();
extern void sys_vmo_create();
extern void sys_vmo_write();
extern void sys_vmo_read();
extern void sys_vmo_map();
extern void sys_register_server();
extern void sys_register_named_server();
extern void sys_register_client();
extern void sys_register_client_by_name();
extern void sys_ipc_call();
extern void sys_ipc_return();
extern void sys_nanosleep();
extern void sys_clock_get();
extern void sys_clock_get_monotonic();
extern void sys_ticks_get();
extern void sys_ticks_per_second();
extern void sys_deadline_after();
extern void sys_yield();

void *sys_call_table[__NR_syscalls] = {
	[__NR_ni_syscall] = sys_ni_syscall,
	[__NR_putc] = sys_putc,
	[__NR_process_create] = sys_process_create,
	[__NR_process_exit] = sys_process_exit,
	[__NR_thread_create] = sys_thread_create,
	[__NR_thread_exit] = sys_thread_exit,
	[__NR_vmo_create] = sys_vmo_create,
	[__NR_vmo_write] = sys_vmo_write,
	[__NR_vmo_read] = sys_vmo_read,
	[__NR_vmo_map] = sys_vmo_map,
	[__NR_register_server] = sys_register_server,
	[__NR_register_named_server] = sys_register_named_server,
	[__NR_register_client] = sys_register_client,
	[__NR_register_client_by_name] = sys_register_client_by_name,
	[__NR_ipc_call] = sys_ipc_call,
	[__NR_ipc_return] = sys_ipc_return,
	[__NR_nanosleep] = sys_nanosleep,
	[__NR_clock_get] = sys_clock_get,
	[__NR_clock_get_monotonic] = sys_clock_get_monotonic,
	[__NR_ticks_get] = sys_ticks_get,
	[__NR_ticks_per_second] = sys_ticks_per_second,
	[__NR_deadline_after] = sys_deadline_after,
	[__NR_yield] = sys_yield,
};
#endif
