#pragma once

#include <task.h>

void scheduler_enqueue_task(thread_t *thread);
void scheduler_dequeue_task(thread_t *thread);

void switch_to(thread_t *, thread_t *);
void switch_mm(thread_t *thread);
void load_context(thread_t *);

void schedule();
void do_sched_init();