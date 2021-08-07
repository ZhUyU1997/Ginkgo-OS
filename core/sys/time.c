#include <core/sys.h>
#include <core/time.h>
#include <core/errno.h>
#include <clocksource/clocksource.h>
#include <task.h>

static int sleep_callback(struct timer_t *timer, void *data)
{
    thread_t *thread = (thread_t *)data;
    thread_resume(thread);
    return 0;
}

status_t sys_nanosleep(ktime_t deadline)
{
    timer_t *timer = &thread_self()->sleep_timer;
    timer_init(timer, sleep_callback, thread_self());
    timer_start(timer, deadline, ms_to_ktime(0));
    thread_suspend(thread_self());
    return SS_OK;
}

status_t sys_clock_get(clockid_t type, ktime_t *time)
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