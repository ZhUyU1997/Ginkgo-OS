#pragma once

#include <asm-offsets.h>
#include <thread_info.h>
#include <compiler.h>
#include <csr.h>
#include <log.h>

typedef struct thread_t thread_t;

extern void __fstate_save(thread_t *save_to);
extern void __fstate_restore(thread_t *restore_from);

static inline void __fstate_clean(struct pt_regs *regs)
{
    regs->sstatus = (regs->sstatus & ~SR_FS) | SR_FS_CLEAN;
}

static inline void fstate_off(thread_t *task,
                              struct pt_regs *regs)
{
    regs->sstatus = (regs->sstatus & ~SR_FS) | SR_FS_OFF;
}

static inline void fstate_save(thread_t *task,
                               struct pt_regs *regs)
{
    if ((regs->sstatus & SR_FS) == SR_FS_DIRTY)
    {
        __fstate_save(task);
        __fstate_clean(regs);
    }
}

static inline void fstate_restore(thread_t *task,
                                  struct pt_regs *regs)
{
    if ((regs->sstatus & SR_FS) != SR_FS_OFF)
    {
        __fstate_restore(task);
        __fstate_clean(regs);
    }
}

static inline void __switch_to_aux(thread_t *prev,
                                   thread_t *next)
{
    struct pt_regs *regs;

    regs = task_pt_regs(prev);
    if (unlikely(regs->sstatus & SR_SD))
        fstate_save(prev, regs);
    fstate_restore(next, task_pt_regs(next));
}

thread_t *__switch_to(thread_t *, thread_t *);

#define switch_to(prev, next, last)              \
    do                                           \
    {                                            \
        thread_t *__prev = (prev);               \
        thread_t *__next = (next);               \
        __switch_to_aux(__prev, __next);         \
        ((last) = __switch_to(__prev, __next)); \
    } while (0)
