#include <core/sys.h>
#include <core/time.h>
#include <core/errno.h>
#include <core/timer.h>
#include <core/class.h>
#include <malloc.h>
#include <task.h>

status_t sys_timer_create(uint32_t options, clock_id_t clock_id, u64_t *out_slot)
{
    if (clock_id != CLOCK_MONOTONIC)
        return SS_ERR_INVALID_ARGS;

    timer_t *timer = new (timer_t);
    *out_slot = slot_alloc_install(process_self(), timer);
    return SS_OK;
}

status_t sys_timer_set(u64_t slot, ktime_t deadline, kduration_t slack)
{
    return SS_OK;
}

status_t sys_timer_cancel(u64_t slot)
{
    return SS_OK;
}