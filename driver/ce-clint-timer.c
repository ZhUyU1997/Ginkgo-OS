#include <clockevent/clockevent.h>
#include <core/timer.h>
#include <csr.h>
#include <smp.h>
#include <io.h>
#include <log.h>
#include <riscv.h>

#define CLINT_MSIP(cpu) (0x0000 + ((cpu)*4))
#define CLINT_MTIMECMP(cpu) (0x4000 + ((cpu)*8))
#define CLINT_MTIME (0xbff8)

class(clint_clockevent_t, clockevent_t)
{
    virtual_addr_t virt;
    int cpu;
};

class_impl(clint_clockevent_t, clockevent_t){};

static void ce_clint_timer_interrupt(void *data)
{
    struct clockevent_t *ce = (struct clockevent_t *)data;
    clint_clockevent_t *cct = dynamic_cast(clint_clockevent_t)(ce);
    ce->handler(ce, ce->data);
}

static bool_t ce_clint_timer_next(struct clockevent_t *this, u64_t evt)
{
    clint_clockevent_t *cct = dynamic_cast(clint_clockevent_t)(this);
    u64_t last = read64(cct->virt + CLINT_MTIME) + evt;
    write64(cct->virt + CLINT_MTIMECMP(cct->cpu), last);

    return TRUE;
}

static int ce_clint_timer_probe(device_t *this, xjil_value_t *value)
{
    clint_clockevent_t *cct = dynamic_cast(clint_clockevent_t)(this);
    clockevent_t *ce = dynamic_cast(clockevent_t)(this);

    virtual_addr_t virt = xjil_read_u64(value, "addr", -1);
    u64_t rate = xjil_read_u64(value, "rate", -1);

    cct->virt = virt;
    cct->cpu = smp_processor_id();

    clockevent_calc_mult_shift(ce, rate, 10);
    ce->min_delta_ns = clockevent_delta2ns(ce, 0x1);
    ce->max_delta_ns = clockevent_delta2ns(ce, 0xffffffffffffffffULL);
    ce->next = ce_clint_timer_next;
    write64(cct->virt + CLINT_MTIMECMP(cct->cpu), 0xffffffff);

    register_default_clockevent(ce);
    return 0;
}

constructor(clint_clockevent_t)
{
    dynamic_cast(device_t)(this)->name = "ce-clint-timer";
    dynamic_cast(device_t)(this)->probe = ce_clint_timer_probe;
}