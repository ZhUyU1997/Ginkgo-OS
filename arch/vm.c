#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "printv.h"
#include "vm.h"
#include "kalloc.h"
#include "string.h"
#include "log.h"

pagetable_t kernel_pagetable_create(void);
pte_t *walk(pagetable_t pagetable, uint64 va, int alloc);
uint64 walk_addr(pagetable_t pagetable, uint64 va);

/*
 * the kernel's page table.
 */
pagetable_t kernel_pagetable;

extern char etext[]; // kernel.ld sets this to end of kernel code.

extern char trampoline[]; // trampoline.S

// Make a direct-map page table for the kernel.
void map_kernel_page(pagetable_t pagetable)
{
    // clint registers
    map_range_in_pgtbl(pagetable, CLINT, CLINT, 0x10000, PTE_R | PTE_W);
    // uart registers
    map_range_in_pgtbl(pagetable, UART0, UART0, PGSIZE, PTE_R | PTE_W);
    // virtio mmio disk interface
    map_range_in_pgtbl(pagetable, VIRTIO0, VIRTIO0, PGSIZE * 8, PTE_R | PTE_W);
    // PLIC
    map_range_in_pgtbl(pagetable, PLIC_ADDR, PLIC_ADDR, 0x400000, PTE_R | PTE_W);
    // map kernel text executable and read-only.
    map_range_in_pgtbl(pagetable, KERNBASE, KERNBASE, (uint64)etext - KERNBASE, PTE_R | PTE_X);
    // map kernel data and the physical RAM we'll make use of.
    map_range_in_pgtbl(pagetable, (uint64)etext, (uint64)etext, PHYSTOP - (uint64)etext, PTE_R | PTE_W);
}

pagetable_t pagetable_create(void)
{
    pagetable_t pt = (pagetable_t)alloc_page(1);
    memset(pt, 0, PGSIZE);
    return pt;
}

// Initialize the one kernel_pagetable
// Switch h/w page table register to the kernel's page table,
// and enable paging.
void do_kvm_init(void)
{
    kernel_pagetable = pagetable_create();
    map_kernel_page(kernel_pagetable);
    pagetable_active(kernel_pagetable);
}

void pagetable_active(pagetable_t pagetable)
{
    csr_write(satp, MAKE_SATP(pagetable));
    local_flush_tlb_all();
}

pte_t *walk(pagetable_t pagetable, uint64 va, int alloc)
{
    if (va >= MAXVA)
        PANIC("walk");

    for (int level = 2; level > 0; level--)
    {
        pte_t *pte = &pagetable[PX(level, va)];
        if (*pte & PTE_V)
        {
            pagetable = (pagetable_t)PTE2PA(*pte);
        }
        else
        {
            if (alloc)
            {
                pagetable = (pde_t *)alloc_page(1);
                // LOGI("create sub pagetable");
                if (pagetable == 0)
                {
                    return 0;
                }
            }
            else
            {
                return 0;
            }

            memset(pagetable, 0, PGSIZE);
            *pte = PA2PTE(pagetable) | PTE_V;
        }
    }
    return &pagetable[PX(0, va)];
}

uint64 walk_addr(pagetable_t pagetable, uint64 va)
{
    pte_t *pte;
    uint64 pa;

    if (va >= MAXVA)
        return 0;

    pte = walk(pagetable, va, 0);
    if (pte == 0)
        return 0;
    if ((*pte & PTE_V) == 0)
        return 0;
    if ((*pte & PTE_U) == 0)
        return 0;
    pa = PTE2PA(*pte);
    return pa;
}

int map_range_in_pgtbl(pagetable_t pagetable, vaddr_t va, paddr_t pa, size_t size, int perm)
{
    if ((va & PAGE_MASK) || (va & PAGE_MASK) || (size & PAGE_MASK))
    {
        PANIC("Invalid Map: [" $(va) " ~ " $(va + size - 1) "] => [" $(pa) " ~ " $(pa + size - 1) "]");
        return -1;
    }

    uint64 a = PGROUNDDOWN(va);
    uint64 last = PGROUNDDOWN(va + size - 1);

    for (;;)
    {
        pte_t *pte;

        if ((pte = walk(pagetable, a, 1)) == 0)
        {
            LOGE("walk(pagetable, a, 1)) == 0");
            return -1;
        }

        if (*pte & PTE_V)
        {
            LOGE("Remap! [" $(a) " => " $(pa) "]");
        }
        *pte = PA2PTE(pa) | perm | PTE_V;
        if (a == last)
            break;
        a += PGSIZE;
        pa += PGSIZE;
    }
    return 0;
}

int unmap_range_in_pgtbl(pagetable_t pagetable, vaddr_t va, size_t size)
{
    uint64 a, last;
    pte_t *pte;

    a = PGROUNDDOWN(va);
    last = PGROUNDDOWN(va + size - 1);
    for (;;)
    {
        if ((pte = walk(pagetable, a, 0)) == 0)
        {
            PANIC("walk(pagetable, a, 0)) == 0");
            return -1;
        }

        if (*pte & PTE_V)
        {
            LOGE("Remap!");
        }
        *pte = 0;
        if (a == last)
            break;
        a += PGSIZE;
    }
    return 0;
}