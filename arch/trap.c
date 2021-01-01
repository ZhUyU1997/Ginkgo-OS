#include <csr.h>
#include <types.h>

extern void kernelvec();

void do_init_trap()
{
    // set up to take exceptions and traps while in the kernel.
    csr_write(stvec, (uint64)kernelvec);
}