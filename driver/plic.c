//
// the riscv Platform Level Interrupt Controller (PLIC).
//

#include <types.h>
#include <memlayout.h>
#include <riscv.h>
#include <io.h>
#include <smp.h>

// qemu puts platform-level interrupt controller (PLIC) here.
#define PLIC_PRIORITY(PLIC) (PLIC + 0x0)
#define PLIC_PENDING(PLIC) (PLIC + 0x1000)
#define PLIC_MENABLE(PLIC, hart) (PLIC + 0x2000 + (hart)*0x100)
#define PLIC_SENABLE(PLIC, hart) (PLIC + 0x2080 + (hart)*0x100)
#define PLIC_MPRIORITY(PLIC, hart) (PLIC + 0x200000 + (hart)*0x2000)
#define PLIC_SPRIORITY(PLIC, hart) (PLIC + 0x201000 + (hart)*0x2000)
#define PLIC_MCLAIM(PLIC, hart) (PLIC + 0x200004 + (hart)*0x2000)
#define PLIC_SCLAIM(PLIC, hart) (PLIC + 0x201004 + (hart)*0x2000)

void plicinit(void)
{
    // set desired IRQ priorities non-zero (otherwise disabled).

    for (int i = 1; i <= 10; i++)
    {
        write32(PLIC_ADDR + i * 4, 1);
    }
}

void plicinithart(void)
{
    int hart = smp_processor_id();

    // set uart's enable bit for this hart's S-mode.
    for (int i = 1; i <= 10; i++)
    {
        write32(PLIC_SENABLE(PLIC_ADDR, hart), read32(PLIC_SENABLE(PLIC_ADDR, hart)) |(1 << i) );
    }
    // set this hart's S-mode priority threshold to 0.
    write32(PLIC_SPRIORITY(PLIC_ADDR, hart), 0);
}

// ask the PLIC what interrupt we should serve.
int plic_claim(void)
{
    int hart = smp_processor_id();
    int irq = read32(PLIC_SCLAIM(PLIC_ADDR, hart));
    return irq;
}

// tell the PLIC we've served this IRQ.
void plic_complete(int irq)
{
    int hart = smp_processor_id();
    write32(PLIC_SCLAIM(PLIC_ADDR, hart), irq);
}
