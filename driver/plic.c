//
// the riscv Platform Level Interrupt Controller (PLIC).
//

#include <types.h>
#include <memlayout.h>
#include <riscv.h>
#include <io.h>
#include <smp.h>
#include <log.h>
#include <malloc.h>
#include <core/device.h>
#include <core/class.h>
#include <interrupt/interrupt.h>
#include <plic.h>

// Each register is 4-bytes (u32)
// The PLIC is an external interrupt controller. The one
// used by QEMU virt is the same as the SiFive PLIC.
// https://sifive.cdn.prismic.io/sifive%2F834354f0-08e6-423c-bf1f-0cb58ef14061_fu540-c000-v1.0.pdf

// Chapter 10 explains the priority, pending, interrupt enable, threshold and claims

// The virt machine has the following external interrupts (from Qemu source):
// Interrupt 0 is a "null" interrupt and is hardwired to 0.
// VIRTIO = [1..8]
// UART0 = 10
// PCIE = [32..35]

#define PLIC_PRIORITY(PLIC) (PLIC + 0x0)
#define PLIC_PENDING(PLIC) (PLIC + 0x1000)
#define PLIC_MENABLE(PLIC, hart) (PLIC + 0x2000 + (hart)*0x100)
#define PLIC_SENABLE(PLIC, hart) (PLIC + 0x2080 + (hart)*0x100)
#define PLIC_MPRIORITY(PLIC, hart) (PLIC + 0x200000 + (hart)*0x2000)
#define PLIC_SPRIORITY(PLIC, hart) (PLIC + 0x201000 + (hart)*0x2000)
#define PLIC_MCLAIM(PLIC, hart) (PLIC + 0x200004 + (hart)*0x2000)
#define PLIC_SCLAIM(PLIC, hart) (PLIC + 0x201004 + (hart)*0x2000)

// ask the PLIC what interrupt we should serve.
static int plic_claim(void)
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

class_impl(plic_irqchip_t, irqchip_t){};

static void plic_irqchip_enable(struct irqchip_t *this, int offset)
{
    irqchip_t *chip = dynamic_cast(irqchip_t)(this);
    int hart = smp_processor_id();
    write32(PLIC_SENABLE(PLIC_ADDR, hart), read32(PLIC_SENABLE(PLIC_ADDR, hart)) | (1 << (offset + chip->base)));
}

static void plic_irqchip_disable(struct irqchip_t *this, int offset)
{
    irqchip_t *chip = dynamic_cast(irqchip_t)(this);
    int hart = smp_processor_id();
    write32(PLIC_SENABLE(PLIC_ADDR, hart), read32(PLIC_SENABLE(PLIC_ADDR, hart)) & ~(1 << (offset + chip->base)));
}

static void plic_irqchip_dispatch(irqchip_t *chip)
{
    int irq = plic_claim();
    int offset = irq - chip->base;

    if ((offset >= 0) && (offset < chip->nirq))
    {
        if (!chip->handler[offset].func)
        {
            PANIC("chip->handler[offset].func == NULL");
        }

        (chip->handler[offset].func)(chip->handler[offset].data);
    }
    else
    {
        LOGE("unexpected interrupt irq=" $(irq));
    }

    if (irq)
        plic_complete(irq);
}

static int plic_irqchip_probe(device_t *this, xjil_value_t *value)
{
    // set desired IRQ priorities non-zero (otherwise disabled).
    irqchip_t *chip = dynamic_cast(irqchip_t)(this);

    chip->base = xjil_read_int(value, "interrupt-base", -1);
    chip->nirq = xjil_read_int(value, "interrupt-count", -1);

    if ((chip->base < 0) || (chip->nirq <= 0))
        return -1;

    chip->handler = (struct irq_handler_t *)calloc(1, sizeof(struct irq_handler_t) * chip->nirq);

    for (int i = chip->base; i < chip->base + chip->nirq; i++)
    {
        write32(PLIC_ADDR + i * 4, 1);
    }

    int hart = smp_processor_id();

    // set uart's enable bit for this hart's S-mode.
    for (int i = chip->base; i < chip->base + chip->nirq; i++)
    {
        write32(PLIC_SENABLE(PLIC_ADDR, hart), read32(PLIC_SENABLE(PLIC_ADDR, hart)) & ~(1 << i));
    }
    // set this hart's S-mode priority threshold to 0.
    write32(PLIC_SPRIORITY(PLIC_ADDR, hart), 0);

    register_irqchip(chip);
    return 0;
}

constructor(plic_irqchip_t)
{
    dynamic_cast(device_t)(this)->name = "plic_irqchip";
    dynamic_cast(device_t)(this)->probe = plic_irqchip_probe;

    dynamic_cast(irqchip_t)(this)->enable = plic_irqchip_enable;
    dynamic_cast(irqchip_t)(this)->disable = plic_irqchip_disable;
    dynamic_cast(irqchip_t)(this)->dispatch = plic_irqchip_dispatch;
}

register_driver(plic_irqchip_t);