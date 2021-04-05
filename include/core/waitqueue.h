#pragma once

#include <list.h>
#include <spinlock.h>
#include <task.h>

typedef struct wait_queue_entry wait_queue_entry_t;
int autoremove_wake_function(struct wait_queue_entry *wq_entry);

typedef int (*wait_queue_func_t)(struct wait_queue_entry *wq_entry);

struct wait_queue_entry
{
    void *priv;
    wait_queue_func_t func;
    struct list_head entry;
};

typedef struct wait_queue_head
{
    spinlock_t lock;
    struct list_head head;
} wait_queue_head_t;

void init_waitqueue_head(struct wait_queue_head *wq_head);
void init_wait_entry(struct wait_queue_entry *wq_entry);
void prepare_to_wait_queue(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry);
void add_wait_queue(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry);
void remove_wait_queue(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry);
void wake_up(struct wait_queue_head *wq_head);

#define wait_event(wq_head, condition)                 \
    do                                                 \
    {                                                  \
        if (condition)                                 \
            break;                                     \
                                                       \
        struct wait_queue_entry wq_entry;              \
        init_wait_entry(&wq_entry);                    \
        for (;;)                                       \
        {                                              \
            prepare_to_wait_queue(wq_head, &wq_entry); \
            if (condition)                             \
                break;                                 \
            task_resume(current);                      \
        }                                              \
        remove_wait_queue(wq_head, &wq_entry);         \
    } while (0)