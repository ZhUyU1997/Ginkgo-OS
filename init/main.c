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
#include <vfs/vfs.h>
#include <ctype.h>
#include <csr.h>
#include <core/syscall.h>
#include <core/test.h>
#include <core/timer.h>

void main()
{
    LOGI("main");
    do_init_trap();
    do_init_mem();
    do_init_class();

    do_kvm_init();
    do_task_init();

    do_init_device();
    do_init_vfs();

#ifdef UNIT_TEST
    thread_resume(kthread_create(do_all_test, NULL));
#endif

    launch_user_init("/test");
    local_irq_enable();
    while (1)
        schedule();
}