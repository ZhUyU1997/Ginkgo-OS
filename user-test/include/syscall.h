#pragma once

#include <types.h>

void usys_putc(char c);
int usys_process_create();
void usys_process_exit(int);

int usys_vmo_create(u64_t size, u64_t type);
int usys_vmo_write(u64_t slot, u64_t offset, u64_t user_ptr, u64_t len);
int usys_vmo_read(u64_t slot, u64_t offset, u64_t user_ptr, u64_t len);
int usys_vmo_map(u64_t target_process_slot, u64_t slot, u64_t addr, u64_t prot, u64_t flags);