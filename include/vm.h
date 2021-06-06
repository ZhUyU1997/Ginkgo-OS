#pragma once

#include "types.h"
#include "riscv.h"

void do_kvm_init(void);
void map_kernel_page(pagetable_t pagetable);
pagetable_t pagetable_create(void);
int map_range_in_pgtbl(pagetable_t pagetable, vaddr_t va, paddr_t pa, size_t size, int perm);
int unmap_range_in_pgtbl(pagetable_t pagetable, vaddr_t va, size_t size);
void pagetable_active(pagetable_t pagetable);