#pragma once

#include "types.h"
#include "riscv.h"

void do_kvm_init(void);
void map_kernel_page(pagetable_t pagetable);
pagetable_t pagetable_create(void);
int pagetable_map(pagetable_t pagetable, uint64 va, uint64 pa, uint64 size, int perm);
void pagetable_active(pagetable_t pagetable);