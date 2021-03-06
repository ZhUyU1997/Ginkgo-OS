#pragma once

#include <task.h>

void scheduler_enqueue_task(thread_t *thread);
void scheduler_dequeue_task(thread_t *thread);

void switch_mm(thread_t *thread);

void schedule();
void schedule_to(thread_t *thread);
void do_sched_init();