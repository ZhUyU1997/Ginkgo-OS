#include "types.h"
#include "riscv.h"
#include "memlayout.h"
#include "kalloc.h"
#include "vm.h"
#include "print.h"
#include "task.h"

extern void kernelvec();
// set up to take exceptions and traps while in the kernel.
void trapinithart(void)
{
    csr_write(stvec, (uint64)kernelvec);
}

void hello()
{
    while (1)
    {
        schedule();
    }
}

void main()
{
    print("main\r\n");
    kinit();
    trapinithart();
    kvminit();
    kvminithart();
    task_init();
    task_create("test0", hello);

    while (1)
        schedule();
}