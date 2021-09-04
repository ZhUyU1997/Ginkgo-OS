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
#include <core/futex.h>

void main()
{
    LOGI("main");
    do_trap_init();
    do_mem_init();
    do_class_init();

    do_futex_init();
    do_kvm_init();
    do_task_init();

    do_device_init();
    do_vfs_init();

#ifdef UNIT_TEST
    thread_resume(kthread_create(do_all_test, NULL));
#endif
    do_sched_init();

    local_irq_enable(); // do_user_init may trigger interrupt (virtio block)
    do_user_init();

    // Not support kernel preemption, must be scheduled manually
    while (1)
        schedule();
}