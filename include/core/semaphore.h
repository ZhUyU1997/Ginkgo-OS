#pragma once

#include <types.h>
#include <list.h>
#include <atomic.h>
#include <spinlock.h>

struct semaphore_t
{
	atomic_t atomic;
	struct list_head mwait;
	spinlock_t lock;
};

void semaphore_init(struct semaphore_t *m, unsigned int count);
void semaphore_down(struct semaphore_t *m);
void semaphore_up(struct semaphore_t *m);
