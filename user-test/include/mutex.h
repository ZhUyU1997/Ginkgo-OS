#pragma once

#include <types.h>
#include <list.h>
#include <atomic.h>

struct mutex_t
{
    atomic_t atomic;
};

static inline void mutex_init(struct mutex_t *m)
{
    atomic_set(&m->atomic, 1);
}
static inline void mutex_lock(struct mutex_t *m)
{
}
static inline void mutex_unlock(struct mutex_t *m)
{
}

struct mutex_guard_t
{
    struct mutex_t *m
};

#define mutex_guard(name, mutex) struct mutex_guard_t __attribute__((cleanup(mutex_auto_unlock), unused)) name = {.m = mutex}

static void mutex_auto_unlock(struct mutex_guard_t *guard)
{
    mutex_unlock(guard->m);
}