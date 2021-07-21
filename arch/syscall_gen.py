#!/usr/bin/env python3

syscall_table = [
    "ni_syscall",
    "putc",
    "process_create",
    "process_exit",
    "thread_create",
    "thread_exit",
    "vmo_create",
    "vmo_write",
    "vmo_read",
    "vmo_map",
    "register_server",
    "register_named_server",
    "register_client",
    "register_client_by_name",
    "ipc_call",
    "ipc_return",
    "nanosleep",
    "clock_get",
    "clock_get_monotonic",
    "ticks_get",
    "ticks_per_second",
    "deadline_after",
    "yield",
]

print("#pragma once")

for k, v in enumerate(syscall_table):
    print(f"#define __NR_{v} {k}")

print(f"#define __NR_syscalls {len(syscall_table)}")
print();
print("#ifdef SYSCALL_IMPL")
print();
for k, v in enumerate(syscall_table):
    print(f"extern void sys_{v}();")

print();

print("void *sys_call_table[__NR_syscalls] = {")
for k, v in enumerate(syscall_table):
    print(f"\t[__NR_{v}] = sys_{v},")
print("};")

print("#endif")

