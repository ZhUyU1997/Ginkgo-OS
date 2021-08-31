#include <types.h>
#include <riscv.h>
#include <memlayout.h>
#include <kalloc.h>
#include <vm.h>
#include <printv.h>
#include <task.h>
#include <log.h>
#include <interrupt/interrupt.h>
#include <clockevent/clockevent.h>

extern void handle_exception();
extern void timer_vector();

struct
{
    uint64 a1;
    uint64 a2;
    uint64 a3;
    uint64 mtimecmp;
    uint64 interval;
} timer_scratch[CONFIG_CPU];

void do_timer_init()
{
    // each CPU has a separate source of timer interrupts.
    int id = csr_read(mhartid);

    // ask the CLINT for a timer interrupt.
    *(uint64 *)CLINT_MTIMECMP(id) = 0xffffffff;
    timer_scratch[id].interval = 0xffffffff; // cycles; about 1/10th second in qemu.
    timer_scratch[id].mtimecmp = CLINT_MTIMECMP(id);

    csr_write(mscratch, (uint64)&timer_scratch[id]);
    csr_write(mtvec, (uint64)timer_vector); // set the machine-mode trap handler.
    csr_set(mie, MIE_MTIE);                 // enable machine-mode timer interrupts.
}

static const char * get_scause_info(uint64 scause)
{
    if (scause == 0)
        return ("Instruction address misaligned");
    else if (scause == 1)
        return ("Instruction access fault");
    else if (scause == 2)
        return("Illegal instruction");
    else if (scause == 3)
        return("Breakpoint");
    else if (scause == 4)
        return("Load address misaligned");
    else if (scause == 5)
        return("Load access fault");
    else if (scause == 6)
        return("Store/AMO address misaligned");
    else if (scause == 7)
        return("Store/AMO access fault");
    else if (scause == 8)
        return("Environment call from U-mode");
    else if (scause == 9)
        return("Environment call from S-mode");
    else if (scause == 10 || scause == 11)
        return("Reserved for future standard use");
    else if (scause == 12)
        return("Instruction page fault");
    else if (scause == 13)
        return("Load page fault");
    else if (scause == 14)
        return("Reserved for future standard use");
    else if (scause == 15)
        return("Store/AMO page fault");
    else if ((scause >= 16) && (scause <= 23))
        return("Reserved for future standard use");
    else if ((scause >= 24) && (scause <= 31))
        return("Reserved for custom use");
    else if ((scause >= 32) && (scause <= 47))
        return("Reserved for future standard use");
    else if ((scause >= 48) && (scause <= 63))
        return("Reserved for custom use");
    else if (scause >= 64)
        return("Reserved for future standard use");
    return "";
}

void do_IRQ(struct pt_regs *regs)
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
            handle_clockevent();
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

        default:
        {
            unsigned long spec = csr_read(sepc);
            unsigned long stval = csr_read(stval);

            printv("scause:" $(scause) "\n");
            printv("sepc: " $(spec) "\n");
            printv("stval: " $(stval) "\n");
            printv("Exception Description: " $(get_scause_info(scause)) "\n");
            while (1)
                ;
            break;
        }
        }
    }
}

void do_trap_init()
{
    LOGD("do_trap_init");
    csr_set(sie, SIE_SEIE | SIE_STIE | SIE_SSIE);
    // set up to take exceptions and traps while in the kernel.
    csr_write(stvec, (uint64)handle_exception);
    // permit Supervisor User Memory access
    csr_set(sstatus, 1 << 18);

    // disable supervisor-mode interrupts.
    csr_clear(sstatus, SSTATUS_SIE);
}