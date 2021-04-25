#include <core/sys.h>
#include <core/time.h>
#include <core/errno.h>
#include <core/timer.h>
#include <malloc.h>

status_t sys_timer_create(uint32_t options, clock_id_t clock_id, handle_t *out)
{
    if (clock_id != CLOCK_MONOTONIC)
        return SS_ERR_INVALID_ARGS;

    struct timer_t *timer = malloc(sizeof(struct timer_t));
    *out = timer;
    return SS_OK;
}

status_t sys_timer_set(handle_t handle, ktime_t deadline, kduration_t slack)
{
    return SS_OK;
}

status_t sys_timer_cancel(handle_t handle)
{
    return SS_OK;
}