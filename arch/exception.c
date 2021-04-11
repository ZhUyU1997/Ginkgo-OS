#include <types.h>
#include <riscv.h>
#include <memlayout.h>
#include <kalloc.h>
#include <vm.h>
#include <printv.h>
#include <task.h>
#include <log.h>
#include <interrupt/interrupt.h>

extern void handle_exception();
extern void timer_vector();

void timertrap()
{
    // LOGI("timertrap");
    // each CPU has a separate source of timer interrupts.
    int id = csr_read(mhartid);
    // ask the CLINT for a timer interrupt.
    int interval = 10000000; // cycles; about 1/10th second in qemu.
    *(uint64 *)CLINT_MTIMECMP(id) = *(uint64 *)CLINT_MTIME + interval;
}

#define NCPU 1 // maximum number of CPUs

struct
{
    uint64 a1;
    uint64 a2;
    uint64 a3;
    uint64 mtimecmp;
    uint64 interval;
} timer_scratch[NCPU][5];

void do_timer_init()
{
    // each CPU has a separate source of timer interrupts.
    int id = csr_read(mhartid);

    // ask the CLINT for a timer interrupt.
    int interval = 10000000;
    *(uint64 *)CLINT_MTIMECMP(id) = *(uint64 *)CLINT_MTIME + interval;

    timer_scratch[id]->interval = interval; // cycles; about 1/10th second in qemu.
    timer_scratch[id]->mtimecmp = CLINT_MTIMECMP(id);

    csr_write(mscratch, (uint64)&timer_scratch[id]);
    // set the machine-mode trap handler.
    csr_write(mtvec, (uint64)timer_vector);
    // enable machine-mode timer interrupts.
    csr_set(mie, MIE_MTIE);
}

void riscv64_handle_exception(struct pt_regs *regs)
{
    uint64 sepc = regs->sepc;
    uint64 sstatus = regs->sstatus;
    uint64 scause = regs->scause;

    unsigned long stval = csr_read(stval);

#if 0
    LOGI("scause:" $(scause) " sepc: " $(sepc) " stval: " $(stval));

    if ((sstatus & SSTATUS_SPP) == 0)
    {
        LOGI("from user mode");
    }
    else
    {
        LOGI("from supervisor mode");
    }
#endif

    if (intr_get() != 0)
    {
        PANIC("interrupts enabled");
    }

    uint64 cause = scause & ~(1UL << 63);
    int interrupt = scause >> 63;

    if (interrupt)
    {
        switch (cause)
        {
        case 1:
            // software interrupt from a machine-mode timer interrupt,
            // forwarded by timervec
            // acknowledge the software interrupt by clearing
            // the SSIP bit in sip.
            csr_clear(sip, 2);
            break;
        case 9:
            interrupt_handle_exception();
            break;
        default:
            break;
        }
    }
    else
    {
        switch (cause)
        {
        case 8:
            regs->sepc += 4;
            LOGI("syscall");
            break;

        default:
        {
            unsigned long spec = csr_read(sepc);
            unsigned long stval = csr_read(stval);
            PANIC(
                "scause:" $(scause) "\n",
                "sepc: " $(spec) "\n",
                "stval: " $(stval) "\n");
            break;
        }
        }
    }
}

void do_init_trap()
{
    LOGD("do_init_trap");
    csr_set(sie, SIE_SEIE | SIE_STIE | SIE_SSIE);
    // set up to take exceptions and traps while in the kernel.
    csr_write(stvec, (uint64)handle_exception);
    // permit Supervisor User Memory access
    // csr_set(sstatus, 1 << 18);

    // disable supervisor-mode interrupts.
    csr_clear(sstatus, SSTATUS_SIE);
}