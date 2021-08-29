#pragma once

#include <types.h>
#include <csr.h>

#define CONFIG_CPU 1
// Machine Status Register, mstatus
#define MSTATUS_MPP_MASK (3L << 11) // previous mode.
#define MSTATUS_MPP_M (3L << 11)
#define MSTATUS_MPP_S (1L << 11)
#define MSTATUS_MPP_U (0L << 11)
#define MSTATUS_MIE (1L << 3) // machine-mode interrupt enable.
#define MSTATUS_MPIE (1L << 7)
#define MSTATUS_SPIE (1L << 5)

// Supervisor Status Register, sstatus
#define SSTATUS_SPP (1L << 8)  // Previous mode, 1=Supervisor, 0=User
#define SSTATUS_SPIE (1L << 5) // Supervisor Previous Interrupt Enable
#define SSTATUS_UPIE (1L << 4) // User Previous Interrupt Enable
#define SSTATUS_SIE (1L << 1)  // Supervisor Interrupt Enable
#define SSTATUS_UIE (1L << 0)  // User Interrupt Enable

// Supervisor Interrupt Enable
#define SIE_SEIE (1L << 9) // external
#define SIE_STIE (1L << 5) // timer
#define SIE_SSIE (1L << 1) // software

// Machine-mode Interrupt Enable
#define MIE_MEIE (1L << 11) // external
#define MIE_MTIE (1L << 7)  // timer
#define MIE_MSIE (1L << 3)  // software

#define SATP_SV39 (8L << 60)
#define MAKE_SATP(pagetable) (SATP_SV39 | (((uint64)pagetable) >> 12))

// enable device interrupts
static inline void intr_on()
{
    csr_set(sstatus, SSTATUS_SIE);
}

// disable device interrupts
static inline void intr_off()
{
    csr_clear(sstatus, SSTATUS_SIE);
}

// are device interrupts enabled?
static inline int intr_get()
{
    return (csr_read(sstatus) & SSTATUS_SIE) != 0;
}

#define register_read(r)                            \
    ({                                              \
        register unsigned long __v;                 \
        __asm__ __volatile__("mv %0, " __ASM_STR(r) \
                             : "=r"(__v)            \
                             :                      \
                             : "memory");           \
        __v;                                        \
    })

#define register_write(r, val)                         \
    ({                                                 \
        unsigned long __v = (unsigned long)(val);      \
        __asm__ __volatile__("mv " __ASM_STR(r) ", %0" \
                             :                         \
                             : "rJ"(__v)               \
                             : "memory");              \
    })

#define r_sp() register_read(sp)

// read and write tp, the thread pointer, which holds
// this core's hartid (core number), the index into cpus[].
#define r_tp() register_read(tp)
#define r_ra() register_read(ra)

static inline void local_flush_tlb_all(void)
{
	__asm__ __volatile__ ("sfence.vma" : : : "memory");
}

/* Flush one page from local TLB */
static inline void local_flush_tlb_page(unsigned long addr)
{
	__asm__ __volatile__ ("sfence.vma %0" : : "r" (addr) : "memory");
}

#define PGSIZE 4096 // bytes per page
#define PGSHIFT 12  // bits of offset within a page
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12
#define PAGE_MASK (PGSIZE - 1)

#define PGROUNDUP(sz) (((sz) + PGSIZE - 1) & ~(PGSIZE - 1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE - 1))

#define PTE_V (1L << 0) // valid
#define PTE_R (1L << 1)
#define PTE_W (1L << 2)
#define PTE_X (1L << 3)
#define PTE_U (1L << 4) // 1 -> user can access

// shift a physical address to the right place for a PTE.
#define PA2PTE(pa) ((((uint64)pa) >> 12) << 10)

#define PTE2PA(pte) (((pte) >> 10) << 12)

#define PTE_FLAGS(pte) ((pte)&0x3FF)

// extract the three 9-bit page table indices from a virtual address.
#define PXMASK 0x1FF // 9 bits
#define PXSHIFT(level) (PGSHIFT + (9 * (level)))
#define PX(level, va) ((((uint64)(va)) >> PXSHIFT(level)) & PXMASK)

// one beyond the highest possible virtual address.
// MAXVA is actually one bit less than the max allowed by
// Sv39, to avoid having to sign-extend virtual addresses
// that have the high bit set.
#define MAXVA (1L << (9 + 9 + 9 + 12 - 1))

typedef uint64 pte_t;
typedef uint64 *pagetable_t; // 512 PTEs

#define phys_to_virt(x) (x)
#define virt_to_phys(x) (x)