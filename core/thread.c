#include <task.h>
#include <vm.h>
#include <log.h>
#include <kalloc.h>
#include <malloc.h>
#include <list.h>
#include <spinlock.h>

LIST_HEAD(ready);

spinlock_t sche_lock;

thread_t *current = NULL;

class_impl(thread_t){};

constructor(thread_t)
{
    init_list_head(&this->ready_queue_node);
    init_list_head(&this->mlist);
    init_list_head(&this->node);
}

int nextpid = 1;

extern pagetable_t kernel_pagetable;

static inline struct thread_t *scheduler_next_ready_task()
{
    if (list_empty(&ready))
    {
        PANIC("empty thread list");
    }

    struct list_head *list = ready.next;
    thread_t *thread = container_of(list, thread_t, ready_queue_node);

    return thread;
}

static inline void scheduler_enqueue_task(thread_t *thread)
{
    list_add_tail(&thread->ready_queue_node, &ready);
}

static inline void scheduler_dequeue_task(thread_t *thread)
{
    list_del_init(&thread->ready_queue_node);
}

void task_mapstacks(thread_t *thread)
{
    map_range_in_pgtbl(thread->vmspace->pgtbl, thread->kstack, thread->kstack, thread->sz, PTE_R | PTE_W);
}

void do_task_init()
{
    spin_lock_init(&sche_lock);

    current = new (thread_t);
    current->vmspace = new (vmspace_t);
    current->name = "idle";
    current->status = TASK_STATUS_RUNNING;

    asm volatile("mv tp, %0" ::"r"(current));
}

static void task_helper()
{
    task_func_t func = (task_func_t)current->context.s0;
    func();
    thread_destroy(current);
}

thread_t *kthread_create(u64_t pc, u64_t arg)
{
    thread_t *thread = new (thread_t);

#define KERNEL_STACK_SIZE PAGE_SIZE
    thread->kstack = (virtual_addr_t)alloc_page(KERNEL_STACK_SIZE / PAGE_SIZE);
    thread->sz = KERNEL_STACK_SIZE;
    thread->vmspace = new (vmspace_t);

    thread->thread_info.kernel_sp = thread->kstack + KERNEL_STACK_SIZE;

    thread->context.ra = (register_t)task_helper;
    thread->context.s0 = (register_t)pc;
    thread->context.sp = thread->thread_info.kernel_sp;

    LOGI("context.sp:" $(thread->context.sp));

    task_mapstacks(thread);
    thread->status = TASK_STATUS_SUSPEND;
    return thread;
}

thread_t *thread_create(process_t *process, u64_t stack, u64_t pc, u64_t arg)
{
    thread_t *thread = new (thread_t);

#define KERNEL_STACK_SIZE PAGE_SIZE
    thread->kstack = (virtual_addr_t)alloc_page(KERNEL_STACK_SIZE / PAGE_SIZE);
    thread->sz = KERNEL_STACK_SIZE;
    thread->vmspace = dynamic_cast(vmspace_t)(slot_get(process, VMSPACE_OBJ_ID));
    thread->process = process;
    thread->thread_info.kernel_sp = thread->kstack + KERNEL_STACK_SIZE;

    struct pt_regs *regs = thread->thread_info.kernel_sp - sizeof(struct pt_regs);
    regs->sp = stack;
    regs->a0 = arg;
    regs->sepc = pc;
    regs->sstatus = csr_read(sstatus);
    regs->sstatus &= ~(SSTATUS_SPP);
    regs->sstatus |= SSTATUS_SPIE;

    void ret_from_exception();

    thread->context.ra = (register_t)ret_from_exception;
    thread->context.sp = regs;

    LOGI("context.sp:" $(thread->context.sp));

    task_mapstacks(thread);
    thread->status = TASK_STATUS_SUSPEND;
    return thread;
}

int sys_thread_create(u64_t process_slot, u64_t stack, u64_t pc, u64_t arg)
{
    process_t *process = dynamic_cast(process_t)(slot_get(current_process, process_slot));
    thread_t *thread = thread_create(process, stack, pc, arg);
    int slot = slot_alloc_install(current_process, thread);
    return slot;
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

static void switch_mm(thread_t *thread)
{
    pagetable_active(thread->vmspace->pgtbl);
}

void schedule()
{
    spin_lock(&sche_lock);

    if (current->status == TASK_STATUS_SUSPEND || current->status == TASK_STATUS_EXIT)
    {
        init_list_head(&current->ready_queue_node);
    }
    else
    {
        scheduler_enqueue_task(current);
    }

    thread_t *thread = scheduler_next_ready_task();
    scheduler_dequeue_task(thread);

    thread_t *old = current;
    current = thread;

    current->status = TASK_STATUS_RUNNING;

    spin_unlock(&sche_lock);

    switch_mm(current);

    if (old->status == TASK_STATUS_EXIT)
    {
        delete (old);
        load_context(current);
    }
    else
    {
        switch_to(old, current);
    }
}