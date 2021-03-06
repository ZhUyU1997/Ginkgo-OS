#pragma once

#include <types.h>
#include <list.h>
#include <atomic.h>
#include <spinlock.h>

struct mutex_t
{
	atomic_t atomic;
	struct list_head mwait;
	spinlock_t lock;
};

void mutex_init(struct mutex_t *m);
void mutex_lock(struct mutex_t *m);
void mutex_unlock(struct mutex_t *m);
