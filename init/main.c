#include <types.h>
#include <riscv.h>
#include <memlayout.h>
#include <kalloc.h>
#include <vm.h>
#include <task.h>
#include <log.h>
#include <trap.h>
#include <core/class.h>
#include <plic.h>
#include <uart.h>
#include <malloc.h>
#include <core/device.h>
#include <hmap.h>

void hello()
{
    LOGI("hello");
    while (1)
        ;
}

void main()
{
    LOGI("main");
    do_init_trap();
    do_init_mem();
    do_init_class();

    kvminit();
    kvminithart();

    do_init_device();
    task_init();
    task_create("test0", hello);

    while (1)
        schedule();
}