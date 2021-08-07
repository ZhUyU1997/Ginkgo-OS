#pragma once

#include <core/sys.h>
#include <core/time.h>
#include <core/errno.h>
#include <clocksource/clocksource.h>

status_t sys_nanosleep(ktime_t deadline);
status_t sys_clock_get(clockid_t type, ktime_t *time);
ktime_t sys_clock_get_monotonic();
kticks_t sys_ticks_get();
kticks_t sys_ticks_per_second();
ktime_t sys_deadline_after(kduration_t nanoseconds);