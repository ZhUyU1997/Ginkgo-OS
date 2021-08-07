#include <task.h>
#include <vm.h>
#include <log.h>
#include <kalloc.h>
#include <malloc.h>
#include <list.h>
#include <spinlock.h>
#include <core/sched.h>

class_impl(thread_t){};

constructor(thread_t)
{
    init_list_head(&this->ready_queue_node);
    init_list_head(&this->mlist);
    init_list_head(&this->node);
}

destructor(thread_t)
{
    timer_cancel(&this->sleep_timer);
}

int nextpid = 1;

extern pagetable_t kernel_pagetable;

void task_mapstacks(thread_t *thread)
{
    map_range_in_pgtbl(thread->vmspace->pgtbl, thread->kstack, thread->kstack, thread->sz, PTE_R | PTE_W);
}

static void task_helper()
{
    thread_t *current = thread_self();

    task_func_t func = (task_func_t)current->context.s0;
    func();
    thread_destroy(current);
}

thread_t *kthread_create(u64_t pc, u64_t arg)
{
    thread_t *thread = new (thread_t);

#define KERNEL_STACK_SIZE PAGE_SIZE
    thread->kstack = (vaddr_t)alloc_page(KERNEL_STACK_SIZE / PAGE_SIZE);
    thread->sz = KERNEL_STACK_SIZE;
    thread->vmspace = new (vmspace_t);

    thread->thread_info.kernel_sp = thread->kstack + KERNEL_STACK_SIZE;

    thread->context.ra = (unsigned long)task_helper;
    thread->context.s0 = (unsigned long)pc;
    thread->context.sp = thread->thread_info.kernel_sp;

    LOGI("context.sp:" $(thread->context.sp));

    task_mapstacks(thread);
    thread->status = TASK_STATUS_SUSPEND;
    return thread;
}

thread_t *thread_create(process_t *process, void *stack, void *pc, void *arg)
{
    thread_t *thread = new (thread_t);

#define KERNEL_STACK_SIZE PAGE_SIZE
    thread->kstack = (vaddr_t)alloc_page(KERNEL_STACK_SIZE / PAGE_SIZE);
    thread->sz = KERNEL_STACK_SIZE;
    thread->vmspace = dynamic_cast(vmspace_t)(slot_get(process, VMSPACE_OBJ_ID));
    thread->process = process;
    thread->thread_info.kernel_sp = thread->kstack + KERNEL_STACK_SIZE;

    struct pt_regs *regs = (struct pt_regs *)(thread->thread_info.kernel_sp - sizeof(struct pt_regs));
    regs->sp = (uintptr_t)stack;
    regs->a0 = (uintptr_t)arg;
    regs->sepc = (uintptr_t)pc;
    regs->sstatus = csr_read(sstatus);
    regs->sstatus &= ~(SSTATUS_SPP);
    regs->sstatus |= SSTATUS_SPIE;

    void ret_from_exception();

    thread->context.ra = (unsigned long)ret_from_exception;
    thread->context.sp = (unsigned long)regs;

    LOGI("context.sp:" $(thread->context.sp));

    task_mapstacks(thread);
    thread->status = TASK_STATUS_SUSPEND;
    return thread;
}

int sys_thread_create(u64_t process_slot, void *stack, void *pc, void *arg)
{
    process_t *process = dynamic_cast(process_t)(slot_get(process_self(), process_slot));
    thread_t *thread = thread_create(process, stack, pc, arg);
    int slot = slot_alloc_install(process_self(), thread);
    return slot;
}

void sys_thread_exit()
{
    thread_t *thread = thread_self();

    if (thread)
        thread_destroy(thread);
}

void thread_exit(thread_t *thread)
{
    thread_destroy(thread);
}

void thread_destroy(thread_t *thread)
{
    if (thread)
    {
        //TODO: free pagetable, task_t

        thread->status = TASK_STATUS_EXIT;
        schedule();
    }
    PANIC();
}

void thread_resume(thread_t *thread)
{
    if (thread && (thread->status == TASK_STATUS_SUSPEND))
    {
        thread->status = TASK_STATUS_READY;
        scheduler_enqueue_task(thread);
    }
}

void thread_suspend(thread_t *thread)
{
    thread_t *next;

    if (thread)
    {
        if (thread->status == TASK_STATUS_READY)
        {
            thread->status = TASK_STATUS_SUSPEND;
            scheduler_dequeue_task(thread);
        }
        else if (thread->status == TASK_STATUS_RUNNING)
        {
            thread->status = TASK_STATUS_SUSPEND;
            schedule();
        }
    }
}


void sys_yield()
{
    thread_suspend(thread_self());
}

void thread_need_sched(thread_t *thread)
{
    thread->thread_info.flags |= NEED_RESCHED;
}

void thread_clear_sched_flag(thread_t *thread)
{
    thread->thread_info.flags &= ~NEED_RESCHED;
}

// should be kept at the end of the file to let the compiler check for incorrect usage
static thread_t *current = NULL;

thread_t *thread_self()
{
    return current;
}

thread_t *thread_self_set(thread_t *thread)
{
    return current = thread;
}

void do_task_init()
{
    thread_t *current = new (thread_t);
    current->vmspace = new (vmspace_t);
    current->name = "idle";
    current->status = TASK_STATUS_RUNNING;
    thread_self_set(current);

    asm volatile("mv tp, %0" ::"r"(current));
}