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
#include <virtio.h>
#include <malloc.h>

void hello()
{
    LOGI("hello");
    // char* p = malloc(1024);;
    // virtio_disk_rw(p, 0, 1024, 0);
    // p[1024 - 1]  ='\0';
    // LOGI("Disk Content:"$(p));
    //     virtio_disk_rw(p, 0, 1024, 0);
    // p[1024 - 1]  ='\0';
    // LOGI("Disk Content:"$(p));
    while (1);
}

void main()
{
    LOGI("main");
    do_init_trap();
    do_init_mem();
    do_init_class();

    kvminit();
    kvminithart();
    plicinit();      // set up interrupt controller
    plicinithart();  // ask PLIC for device interrupts
    uart_init();
    virtio_init();
    virtio_disk_init();
    task_init();
    task_create("test0", hello);

    while (1)
        schedule();
}