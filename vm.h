#pragma once

#include "types.h"
#include "riscv.h"

pagetable_t kvmmake(void);
void kvminit(void);
void kvminithart();
pte_t *walk(pagetable_t pagetable, uint64 va, int alloc);
uint64 walkaddr(pagetable_t pagetable, uint64 va);
void kvmmap(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 sz, int perm);
int mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm);
