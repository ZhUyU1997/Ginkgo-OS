#include "types.h"
#include "riscv.h"
#include "memlayout.h"
#include "kalloc.h"
#include "vm.h"
#include "printv.h"
#include "task.h"
#include "log.h"

#include <stddef.h>

#define NCPU 1 // maximum number of CPUs

// entry.S needs one stack per CPU.
__attribute__((aligned(16))) char stack0[4096 * NCPU];

extern void timervec();

void timerinit()
{
    // each CPU has a separate source of timer interrupts.
    int id = csr_read(mhartid);

    // ask the CLINT for a timer interrupt.
    int interval = 10000000; // cycles; about 1/10th second in qemu.
    *(uint64 *)CLINT_MTIMECMP(id) = *(uint64 *)CLINT_MTIME + interval;

    // prepare information in scratch[] for timervec.
    // scratch[0..2] : space for timervec to save registers.
    // scratch[3] : address of CLINT MTIMECMP register.
    // scratch[4] : desired interval (in cycles) between timer interrupts.
    // uint64 *scratch = &timer_scratch[id][0];
    // scratch[3] = CLINT_MTIMECMP(id);
    // scratch[4] = interval;
    // csr_write(mscratch, (uint64)scratch);

    // set the machine-mode trap handler.
    csr_write(mtvec, (uint64)timervec);

    // enable machine-mode interrupts.
    csr_set(mstatus, MSTATUS_MIE);

    // enable machine-mode timer interrupts.
    csr_set(mie, MIE_MTIE);
}

// check if it's an external interrupt or software interrupt,
// and handle it.
// returns 2 if timer interrupt,
// 1 if other device,
// 0 if not recognized.
int devintr()
{
    uint64 scause = csr_read(scause);

    // LOGI("devintr");

    if ((scause & 0x8000000000000000L) &&
        (scause & 0xff) == 9)
    {
        // this is a supervisor external interrupt, via PLIC.

        // irq indicates which device interrupted.
        LOGI("PLIC");
        return 1;
    }
    else if (scause == 0x8000000000000001L)
    {
        // software interrupt from a machine-mode timer interrupt,
        // forwarded by timervec in kernelvec.S.

        // LOGI("timer");

        // acknowledge the software interrupt by clearing
        // the SSIP bit in sip.
        csr_clear(sip, 2);

        return 2;
    }
    else
    {
        return 0;
    }
}

void timertrap()
{
    // LOGI("timertrap");
    // each CPU has a separate source of timer interrupts.
    int id = csr_read(mhartid);

    // ask the CLINT for a timer interrupt.
    int interval = 10000000; // cycles; about 1/10th second in qemu.
    *(uint64 *)CLINT_MTIMECMP(id) = *(uint64 *)CLINT_MTIME + interval;
}
void kerneltrap()
{
    // LOGI("kerneltrap");
    uint64 sepc = csr_read(sepc);
    uint64 sstatus = csr_read(sstatus);
    uint64 scause = csr_read(scause);

    if ((sstatus & SSTATUS_SPP) == 0)
    {
        PANIC("kerneltrap: not from supervisor mode");
    }

    if (intr_get() != 0)
    {
        PANIC("kerneltrap: interrupts enabled");
    }

    if ((devintr()) == 0)
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

void start()
{
    LOGI("Hello RISC-V!");

    // set M Previous Privilege mode to Supervisor, for mret.
    unsigned long x = csr_read(mstatus);
    x &= ~MSTATUS_MPP_MASK;
    x |= MSTATUS_MPP_S;
    csr_write(mstatus, x);

    // set M Exception Program Counter to main, for mret.
    // requires gcc -mcmodel=medany
    void main();
    csr_write(mepc, (uint64)main);

    // disable paging for now.
    csr_write(satp, 0);

    // delegate all interrupts and exceptions to supervisor mode.
    csr_write(medeleg, 0xffff);
    csr_write(mideleg, 0xffff);
    csr_set(sie, SIE_SEIE | SIE_STIE | SIE_SSIE);

    // ask for clock interrupts.
    // timerinit();
    // enable machine-mode interrupts.
    csr_set(sstatus, SSTATUS_SIE);
    // enable machine-mode timer interrupts.
    timerinit();
    // keep each CPU's hartid in its tp register, for cpuid().
    int id = csr_read(mhartid);
    register_write(tp, id);
    // switch to supervisor mode and jump to main().
    asm volatile("mret");
}