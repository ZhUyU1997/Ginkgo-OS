#include <clocksource/clocksource.h>
#include <core/device.h>
#include <smp.h>
#include <io.h>
#include <log.h>

#include <malloc.h>

#define CLINT_MSIP(cpu) (0x0000 + ((cpu)*4))
#define CLINT_MTIMECMP(cpu) (0x4000 + ((cpu)*8))
#define CLINT_MTIME (0xbff8)

class(clint_clocksource_t, clocksource_t)
{
    virtual_addr_t virt;
    int cpu;
};

class_impl(clint_clocksource_t, clocksource_t){};

static u64_t cs_clint_timer_read(struct clocksource_t *this)
{
    clint_clocksource_t *cs = dynamic_cast(clint_clocksource_t)(this);
    return (u64_t)read64(cs->virt + CLINT_MTIME);
}

static int cs_clint_timer_probe(device_t *this, xjil_value_t *value)
{
    clint_clocksource_t *ccs = dynamic_cast(clint_clocksource_t)(this);
    clocksource_t *cs = dynamic_cast(clocksource_t)(this);
    virtual_addr_t virt = xjil_read_u64(value, "addr", -1);
    u64_t rate = xjil_read_u64(value, "rate", -1);

    ccs->virt = virt;
    ccs->cpu = smp_processor_id();

    clocksource_calc_mult_shift(&cs->mult, &cs->shift, rate, 1000000000ULL, 10);

    cs->mask = CLOCKSOURCE_MASK(64);
    cs->read = cs_clint_timer_read;
    cs->ticks_per_second = rate;
    register_default_clocksource(cs);
    return 0;
}

constructor(clint_clocksource_t)
{
    dynamic_cast(device_t)(this)->name = "clint-clocksource";
    dynamic_cast(device_t)(this)->probe = cs_clint_timer_probe;
    dynamic_cast(clocksource_t)(this)->read = cs_clint_timer_read;
    dynamic_cast(clocksource_t)(this)->mask = CLOCKSOURCE_MASK(64);
}