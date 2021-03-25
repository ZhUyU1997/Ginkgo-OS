#include <types.h>
#include <riscv.h>
#include <memlayout.h>
#include <kalloc.h>
#include <vm.h>
#include <printv.h>
#include <task.h>
#include <log.h>
#include <interrupt/interrupt.h>

extern void kernelvec();
extern void timervec();

void timertrap()
{
    // LOGI("timertrap");
    // each CPU has a separate source of timer interrupts.
    int id = csr_read(mhartid);
    // ask the CLINT for a timer interrupt.
    int interval = 10000000; // cycles; about 1/10th second in qemu.
    *(uint64 *)CLINT_MTIMECMP(id) = *(uint64 *)CLINT_MTIME + interval;
}

void do_timer_init()
{
    // each CPU has a separate source of timer interrupts.
    int id = csr_read(mhartid);
    // ask the CLINT for a timer interrupt.
    int interval = 10000000; // cycles; about 1/10th second in qemu.
    *(uint64 *)CLINT_MTIMECMP(id) = *(uint64 *)CLINT_MTIME + interval;
    // set the machine-mode trap handler.
    csr_write(mtvec, (uint64)timervec);
    // enable machine-mode timer interrupts.
    csr_set(mie, MIE_MTIE);
}


void riscv64_handle_exception()
{
    // LOGI();
    uint64 sepc = csr_read(sepc);
    uint64 sstatus = csr_read(sstatus);
    uint64 scause = csr_read(scause);

    if ((sstatus & SSTATUS_SPP) == 0)
    {
        PANIC("not from supervisor mode");
    }

    if (intr_get() != 0)
    {
        PANIC("interrupts enabled");
    }

    // LOGI("devce_interrupt");
    unsigned long cause = scause & ~(1UL << 63);
    if ((scause & 0x8000000000000000L))
    {
        if (cause == 9)
            interrupt_handle_exception();
        else if (cause == 1)
        {
            // software interrupt from a machine-mode timer interrupt,
            // forwarded by timervec in kernelvec.S.

            // LOGI("timer");

            // acknowledge the software interrupt by clearing
            // the SSIP bit in sip.
            csr_clear(sip, 2);
        }
    }
    else
    {
        unsigned long spec = csr_read(sepc);
        unsigned long stval = csr_read(stval);

        PANIC(
            "scause:" $(scause) "\n",
            "sepc: " $(spec) "\n",
            "stval: " $(stval) "\n");
    }
    // the yield() may have caused some traps to occur,
    // so restore trap registers for use by kernelvec.S's sepc instruction.
    csr_write(sepc, sepc);
    csr_write(sstatus, sstatus);
}

void do_init_trap()
{
    LOGD("do_init_trap");
    csr_set(sie, SIE_SEIE | SIE_STIE | SIE_SSIE);
    // set up to take exceptions and traps while in the kernel.
    csr_write(stvec, (uint64)kernelvec);
    // enable supervisor-mode interrupts.
    csr_set(sstatus, SSTATUS_SIE);
}