#include <task.h>
#include <vm.h>
#include <log.h>
#include <kalloc.h>
#include <malloc.h>
#include <list.h>
#include <spinlock.h>

LIST_HEAD(ready);

spinlock_t sche_lock;

task_t *idle = &(task_t){0};
task_t *current = NULL;

int nextpid = 1;

extern pagetable_t kernel_pagetable;

static inline struct task_t *scheduler_next_ready_task()
{
    if (list_empty(&ready))
    {
        PANIC("empty task list");
    }

    struct list_head *list = ready.next;
    task_t *task = container_of(list, task_t, list);

    return task;
}

static inline void scheduler_enqueue_task(task_t *task)
{
    list_add_tail(&task->list, &ready);
}

static inline void scheduler_dequeue_task(task_t *task)
{
    list_del_init(&task->list);
}

static void switch_mm(task_t *task)
{
    pagetable_active(task->pagetable);
}

void schedule()
{
    spin_lock(&sche_lock);

    if (current->status == TASK_STATUS_SUSPEND)
    {
        init_list_head(&current->list);
    }
    else
    {
        scheduler_enqueue_task(current);
    }

    task_t *task = scheduler_next_ready_task();
    scheduler_dequeue_task(task);

    task_t *old = current;
    current = task;

    current->status = TASK_STATUS_RUNNING;

    spin_unlock(&sche_lock);

    switch_mm(current);
    switch_context(&old->context, &current->context);
}

void task_mapstacks(task_t *task)
{
    pagetable_map(task->pagetable, task->kstack, task->kstack, PGSIZE, PTE_R | PTE_W);
}

void do_task_init()
{
    spin_lock_init(&sche_lock);
    init_list_head(&idle->list);
    init_list_head(&idle->mlist);

    idle->pagetable = kernel_pagetable;
    idle->name = "idle";
    idle->status = TASK_STATUS_RUNNING;
    current = idle;

    asm volatile("mv tp, %0" ::"r"(current));
}

task_t *task_create(const char *name, task_func_t func)
{
    LOGD("Create task: " $(name));
    task_t *task = (task_t *)alloc_page(1);
    init_list_head(&task->list);
    init_list_head(&task->mlist);

    task->kstack = (virtual_addr_t)task;

    task->pagetable = pagetable_create();
    map_kernel_page(task->pagetable);

    task->name = name;

    task->thread_info.kernel_sp = task->kstack + PGSIZE;
    task->context.ra = (register_t)func;
    task->context.sp = task->thread_info.kernel_sp - sizeof(struct pt_regs);

    LOGI("context.sp:" $(task->context.sp));

    task_mapstacks(task);
    task->status = TASK_STATUS_SUSPEND;
    return task;
}

void task_resume(task_t *task)
{
    if (task && (task->status == TASK_STATUS_SUSPEND))
    {
        task->status = TASK_STATUS_READY;
        scheduler_enqueue_task(task);
    }
}

void task_suspend(task_t *task)
{
    task_t *next;

    if (task)
    {
        if (task->status == TASK_STATUS_READY)
        {
            task->status = TASK_STATUS_SUSPEND;
            scheduler_dequeue_task(task);
        }
        else if (task->status == TASK_STATUS_RUNNING)
        {
            task->status = TASK_STATUS_SUSPEND;
            LOGI();
            schedule();
        }
    }
}