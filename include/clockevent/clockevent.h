#pragma once

#include <types.h>
#include <core/time.h>
#include <core/device.h>
#include <core/class.h>

class(clockevent_t, device_t)
{
    u32_t mult;
    u32_t shift;
    u64_t min_delta_ns;
    u64_t max_delta_ns;
    void *data;
    void (*handler)(struct clockevent_t * ce, void *data);

    bool_t (*next)(struct clockevent_t * ce, u64_t evt);
};

/*
 * clockevent_hz2mult - calculates mult from hz and shift
 * @hz: Clockevent frequency in Hz
 * @shift_constant:	Clockevent shift factor
 *
 * Helper functions that converts a hz counter
 * frequency to a timsource multiplier, given the
 * clockevent shift value
 */
static inline u32_t clockevent_hz2mult(u32_t hz, u32_t shift)
{
    /*
	 * hz = cyc/(Billion ns)
	 * mult/2^shift  = ns/cyc
	 * mult = ns/cyc * 2^shift
	 * mult = 1Billion/hz * 2^shift
	 * mult = 1000000000 * 2^shift / hz
	 * mult = (1000000000<<shift) / hz
	 */
    u64_t tmp = ((u64_t)1000000000) << shift;
    tmp += hz / 2;
    tmp = tmp / hz;
    return (u32_t)tmp;
}

/*
 * clockevent_khz2mult - calculates mult from khz and shift
 * @khz: Clockevent frequency in KHz
 * @shift_constant: Clockevent shift factor
 *
 * Helper functions that converts a khz counter frequency to a timsource
 * multiplier, given the clockevent shift value
 */
static inline u32_t clockevent_khz2mult(u32_t khz, u32_t shift)
{
    /*
	 * khz = cyc/(Million ns)
	 * mult/2^shift  = ns/cyc
	 * mult = ns/cyc * 2^shift
	 * mult = 1Million/khz * 2^shift
	 * mult = 1000000 * 2^shift / khz
	 * mult = (1000000<<shift) / khz
	 */
    u64_t tmp = ((u64_t)1000000) << shift;
    tmp += khz / 2;
    tmp = tmp / khz;
    return (u32_t)tmp;
}

/*
 * clockevent_calc_mult_shift - calculate mult/shift factors from frequency and minsec
 * @ce:		pointer to clockevent
 * @freq:	frequency to convert to
 * @minsec:	guaranteed runtime conversion range in seconds
 *
 * The function evaluates the shift/mult pair for the scaled math
 * operations of clockevents.
 */
static inline void clockevent_calc_mult_shift(struct clockevent_t *ce, u32_t freq, u32_t minsec)
{
    u64_t tmp;
    u32_t sft, sftacc = 32;

    /*
	 * Calculate the shift factor which is limiting the conversion range
	 */
    tmp = ((u64_t)minsec * 1000000000ULL) >> 32;
    while (tmp)
    {
        tmp >>= 1;
        sftacc--;
    }

    /*
	 * Find the conversion shift/mult pair which has the best
	 * accuracy and fits the minsec conversion range:
	 */
    for (sft = 32; sft > 0; sft--)
    {
        tmp = (u64_t)freq << sft;
        tmp += 1000000000ULL / 2;
        tmp = tmp / 1000000000ULL;
        if ((tmp >> sftacc) == 0)
            break;
    }
    ce->mult = tmp;
    ce->shift = sft;
}

/*
 * clockevent_delta2ns - Convert a latch value (device ticks) to nanoseconds
 * @ce:	pointer to clockevent device
 * @latch:	value to convert
 *
 * Math helper, returns latch value converted to nanoseconds (bound checked)
 */
static inline u64_t clockevent_delta2ns(struct clockevent_t *ce, u64_t latch)
{
    u64_t clc = (u64_t)latch << ce->shift;
    u64_t rnd;

    if (!ce->mult)
        ce->mult = 1;
    rnd = (u64_t)ce->mult - 1;

    if ((clc >> ce->shift) != (u64_t)latch)
        clc = ~0ULL;

    if ((~0ULL - clc > rnd) && (ce->mult <= (1ULL << ce->shift)))
        clc += rnd;

    clc = clc / ce->mult;
    return clc > 1000 ? clc : 1000;
}

void register_default_clockevent(struct clockevent_t *ce);
void handle_clockevent();
bool_t clockevent_set_event_handler(struct clockevent_t *ce, void (*handler)(struct clockevent_t *, void *), void *data);
bool_t clockevent_set_event_next(struct clockevent_t *ce, ktime_t now, ktime_t expires);
