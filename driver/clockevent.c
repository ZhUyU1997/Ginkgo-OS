#include <clockevent/clockevent.h>
#include <malloc.h>
#include <core/timer.h>
#include <compiler.h>
#include <log.h>


/*
 * Dummy clockevent, 1us - 1MHZ
 */
static void __ce_dummy_handler(struct clockevent_t *ce, void *data)
{
}

static bool_t __ce_dummy_next(struct clockevent_t *ce, u64_t evt)
{
    return TRUE;
}

class_impl(clockevent_t, device_t){
    .mult = 4294967,
    .shift = 32,
    .min_delta_ns = 1000,
    .max_delta_ns = 4294967591000,
    .data = NULL,
    .handler = __ce_dummy_handler,
    .next = __ce_dummy_next,
};

static struct clockevent_t *__clockevent = NULL;

void register_default_clockevent(struct clockevent_t *ce)
{
    __clockevent = ce;
    timer_bind_clockevent(__clockevent);
}

void handle_clockevent()
{
    if (__clockevent)
    {
        __clockevent->handler(__clockevent, __clockevent->data);
    }
}

bool_t clockevent_set_event_handler(struct clockevent_t *ce, void (*handler)(struct clockevent_t *, void *), void *data)
{
    if (!ce)
        return FALSE;
    ce->data = data;
    ce->handler = handler ? handler : __ce_dummy_handler;
    return TRUE;
}

bool_t clockevent_set_event_next(struct clockevent_t *ce, ktime_t now, ktime_t expires)
{
    u64_t delta;

    if (!ce)
        return FALSE;

    if (ktime_before(expires, now))
        return FALSE;

    delta = ktime_to_ns(ktime_sub(expires, now));
    if (delta > ce->max_delta_ns)
        delta = ce->max_delta_ns;
    if (delta < ce->min_delta_ns)
        delta = ce->min_delta_ns;
    return ce->next(ce, ((u64_t)delta * ce->mult) >> ce->shift);
}
