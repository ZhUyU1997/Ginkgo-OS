#include <core/sys.h>
#include <core/time.h>
#include <core/errno.h>

sc_status_t timer_create(uint32_t options, clock_id_t clock_id, handle_t *out)
{
    return SS_OK;
}

sc_status_t timer_set(handle_t handle, ktime_t deadline, kduration_t slack)
{
    return SS_OK;
}

sc_status_t timer_cancel(handle_t handle)
{
    return SS_OK;
}
