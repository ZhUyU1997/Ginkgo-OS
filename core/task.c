#include "task.h"
#include "vm.h"
#include "printv.h"
#include "kalloc.h"

int nextpid = 1;

task_t *idle = &(task_t){0};
static struct list_head *ready = &(struct list_head){0};
extern pagetable_t kernel_pagetable;
task_t *current = NULL;

void schedule()
{
    if (!list_empty(ready))
    {
        struct list_head *list = ready->next;
        list_del(list);
        list_add_tail(&current->list, ready);

        task_t *task = container_of(list, task_t, list);
        task_t *old = current;
        current = task;
        printv("swtch:"$(old->name)"-->"$(current->name)"\n");
        swtch(&old->context, &current->context);
    }
    else
    {
        printv("empty task list\n");
    }
}

void task_mapstacks(task_t *task)
{
    kvmmap(task->pagetable, task->kstack, task->kstack, PGSIZE, PTE_R | PTE_W);
}

void task_init()
{
    init_list_head(ready);
    idle->pagetable = kernel_pagetable;
    idle->name = "idle";
    current = idle;
}

void task_create(const char *name, task_func_t func)
{
    task_t *task = (task_t *)alloc_page(1);
    init_list_head(&task->list);
    task->kstack = (virtual_addr_t)task;
    task->context.ra = (register_t)func;
    task->context.sp = task->kstack + PGSIZE;
    task->pagetable = kernel_pagetable;
    task->name = name;

    task_mapstacks(task);
    list_add_tail(&task->list, ready);
}
