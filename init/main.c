#include <types.h>
#include <riscv.h>
#include <memlayout.h>
#include <kalloc.h>
#include <vm.h>
#include <task.h>
#include <log.h>
#include <core/class.h>

extern void kernelvec();
// set up to take exceptions and traps while in the kernel.
void trapinithart(void)
{
    csr_write(stvec, (uint64)kernelvec);
}

void hello()
{
    LOGI("hello");
    while (1);
}

void main()
{
    LOGI("main");
    kinit();
    trapinithart();
    kvminit();
    kvminithart();

    do_init_class();
    task_init();
    task_create("test0", hello);

    while (1)
        schedule();
}