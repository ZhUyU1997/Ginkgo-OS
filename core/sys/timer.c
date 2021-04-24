#include <core/sys.h>
#include <core/time.h>
#include <core/errno.h>

status_t sys_timer_create(uint32_t options, clock_id_t clock_id, handle_t *out)
{
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