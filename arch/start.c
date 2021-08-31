#include <types.h>
#include <riscv.h>
#include <memlayout.h>
#include <kalloc.h>
#include <vm.h>
#include <printv.h>
#include <task.h>
#include <log.h>
#include <plic.h>
#include <uart.h>
#include <trap.h>

// entry.S needs one stack per CPU.
__attribute__((aligned(16))) char stack0[4096 * CONFIG_CPU];

extern unsigned long __bss_start[];
extern unsigned long __bss_end[];

static void clear_bss()
{
    for (int i = 0; i < __bss_end - __bss_start; i++)
    {
        __bss_start[i] = 0;
    }
}

void start()
{
    clear_bss();

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

    // delegate all interrupts and exceptions to supervisor mode.
    csr_write(medeleg, 0xffff);
    csr_write(mideleg, 0xffff);

    // enable machine-mode timer interrupts.
    do_timer_init();

    // enable machine-mode interrupts.
    csr_set(mstatus, MSTATUS_MIE);

    // disable paging for now.
    csr_write(satp, 0);

    // keep each CPU's hartid in its tp register, for cpuid().
    int id = csr_read(mhartid);
    register_write(tp, id);
    // switch to supervisor mode and jump to main().
    asm volatile("mret");
}