#include <task.h>
#include <spinlock.h>
#include <log.h>
#include <core/sched.h>
#include <core/timer.h>

LIST_HEAD(ready);
static spinlock_t sched_lock;

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

void switch_mm(thread_t *thread)
{
    pagetable_active(thread->vmspace->pgtbl);
}

void scheduler_enqueue_task(thread_t *thread)
{
    list_add_tail(&thread->ready_queue_node, &ready);
}

void scheduler_dequeue_task(thread_t *thread)
{
    list_del_init(&thread->ready_queue_node);
}

void schedule()
{
    spin_lock(&sched_lock);

    thread_t *current = thread_self();

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

    current = thread_self_set(thread);

    current->status = TASK_STATUS_RUNNING;

    spin_unlock(&sched_lock);

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

static int sched_timer_function(struct timer_t *timer, void *data)
{
    LOGI();
    thread_need_sched(thread_self());
	timer_forward_now(timer, ms_to_ktime(1000));
    return 1;
}

void do_sched_init()
{
    spin_lock_init(&sched_lock);
    timer_t *timer = new (timer_t);
    timer_init(timer, sched_timer_function, NULL);
    timer_start_now(timer, ms_to_ktime(1000));
}