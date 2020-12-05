#include "types.h"
#include "riscv.h"
#include "memlayout.h"
#include "kalloc.h"
#include "vm.h"
#include "print.h"
#include "task.h"

#include <stddef.h>

#define NCPU 1 // maximum number of CPUs

// entry.S needs one stack per CPU.
__attribute__((aligned(16))) char stack0[4096 * NCPU];


extern void timervec();

void timerinit()
{
    // each CPU has a separate source of timer interrupts.
    int id = r_mhartid();

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
    // w_mscratch((uint64)scratch);

    // set the machine-mode trap handler.
    w_mtvec((uint64)timervec);

    // enable machine-mode interrupts.
    w_mstatus(r_mstatus() | MSTATUS_MIE);

    // enable machine-mode timer interrupts.
    w_mie(r_mie() | MIE_MTIE);
}

// check if it's an external interrupt or software interrupt,
// and handle it.
// returns 2 if timer interrupt,
// 1 if other device,
// 0 if not recognized.
int devintr()
{
    uint64 scause = r_scause();
    print("devintr\r\n");
    if ((scause & 0x8000000000000000L) &&
        (scause & 0xff) == 9)
    {
        // this is a supervisor external interrupt, via PLIC.

        // irq indicates which device interrupted.
        print("PLIC");
        return 1;
    }
    else if (scause == 0x8000000000000001L)
    {
        // software interrupt from a machine-mode timer interrupt,
        // forwarded by timervec in kernelvec.S.

        print("timer");

        // acknowledge the software interrupt by clearing
        // the SSIP bit in sip.
        w_sip(r_sip() & ~2);

        return 2;
    }
    else
    {
        return 0;
    }
}

void timertrap()
{
    print("timertrap\r\n");
    // each CPU has a separate source of timer interrupts.
    int id = r_mhartid();

    // ask the CLINT for a timer interrupt.
    int interval = 10000000; // cycles; about 1/10th second in qemu.
    *(uint64 *)CLINT_MTIMECMP(id) = *(uint64 *)CLINT_MTIME + interval;
}
void kerneltrap()
{
    print("kerneltrap\r\n");
    uint64 sepc = r_sepc();
    uint64 sstatus = r_sstatus();
    uint64 scause = r_scause();

    if ((sstatus & SSTATUS_SPP) == 0)
    {
        print("kerneltrap: not from supervisor mode");
        while (1)
            ;
    }

    if (intr_get() != 0)
    {
        print("kerneltrap: interrupts enabled");
        while (1)
            ;
    }

    if ((devintr()) == 0)
    {
        print("scause :");
        printx_u64(scause);
        print("\n");
        print("sepc :");
        printx_u64(r_sepc());
        print("\n");
        print("stval :");
        printx_u64(r_stval());
        print("\n");
        while (1)
            ;
    }
    // the yield() may have caused some traps to occur,
    // so restore trap registers for use by kernelvec.S's sepc instruction.
    w_sepc(sepc);
    w_sstatus(sstatus);
}



void start()
{
    print("Hello RISC-V!\r\n");

    // set M Previous Privilege mode to Supervisor, for mret.
    unsigned long x = r_mstatus();
    x &= ~MSTATUS_MPP_MASK;
    x |= MSTATUS_MPP_S;
    w_mstatus(x);

    // set M Exception Program Counter to main, for mret.
    // requires gcc -mcmodel=medany
    void main();
    w_mepc((uint64)main);

    // disable paging for now.
    w_satp(0);

    // delegate all interrupts and exceptions to supervisor mode.
    w_medeleg(0xffff);
    w_mideleg(0xffff);
    w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);

    // ask for clock interrupts.
    // timerinit();
    // enable machine-mode interrupts.
    w_sstatus(r_sstatus() | SSTATUS_SIE);
    // enable machine-mode timer interrupts.

    // keep each CPU's hartid in its tp register, for cpuid().
    int id = r_mhartid();
    w_tp(id);
    // switch to supervisor mode and jump to main().
    asm volatile("mret");
}