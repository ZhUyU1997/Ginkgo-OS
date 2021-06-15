#include <core/class.h>
#include <core/device.h>
#include <core/syscall.h>
#include <core/test.h>
#include <core/timer.h>
#include <core/sched.h>

#include <vm.h>
#include <task.h>
#include <kalloc.h>
#include <trap.h>
#include <vfs/vfs.h>
#include <log.h>
#include <types.h>

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
    do_sched_init();

    launch_user_init("/test");
    local_irq_enable();

    // Not support kernel preemption, must be scheduled manually
    while (1)
        schedule();
}