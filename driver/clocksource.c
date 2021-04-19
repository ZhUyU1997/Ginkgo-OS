#include <clocksource/clocksource.h>
#include <csr.h>
#include <spinlock.h>
#include <types.h>

/*
 * Dummy clocksource, 10us - 100KHZ
 */
static u64_t __cs_dummy_read(struct clocksource_t *cs)
{
    static volatile u64_t __cs_dummy_cycle = 0;
    return __cs_dummy_cycle++;
}

class_impl(clocksource_t, device_t){
    .name = "cs-dummy",
    .mask = CLOCKSOURCE_MASK(64),
    .mult = 2621440000,
    .shift = 18,
    .read = __cs_dummy_read,
};

static clocksource_t *__clocksource = NULL;

void register_default_clocksource(clocksource_t *cs)
{
    __clocksource = cs;
}

ktime_t clocksource_ktime_get(struct clocksource_t *cs)
{
    if (cs)
        return clocksource_keeper_read(cs);
    return ns_to_ktime(0);
}

ktime_t ktime_get(void)
{
    return clocksource_ktime_get(__clocksource);
}
