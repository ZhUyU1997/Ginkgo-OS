#include <core/sys.h>
#include <core/time.h>
#include <core/errno.h>
#include <clocksource/clocksource.h>

sc_status_t sys_nanosleep(ktime_t deadline)
{
    return SS_OK;
}

sc_status_t sys_clock_get(clock_id_t type, ktime_t *time)
{
    *time = ktime_get();
    return SS_OK;
}

ktime_t sys_clock_get_monotonic()
{
    return ktime_get();
}

kticks_t sys_ticks_get()
{
    return kticks_get();
}

kticks_t sys_ticks_per_second()
{
    return kticks_ticks_per_second();
}

ktime_t sys_deadline_after(kduration_t nanoseconds)
{
    return ktime_add_ns(ktime_get(), nanoseconds);
}