#pragma once

#include <core/time.h>
#include <core/class.h>

#define CLOCKSOURCE_MASK(bits) (u64_t)((bits) < 64 ? ((1ULL << (bits)) - 1) : -1)

class(clocksource_t, device_t)
{
    char *name;
    u64_t mask;
    u32_t mult;
    u32_t shift;
    kticks_t ticks_per_second;
    u64_t (*read)(struct clocksource_t * cs);
};

/*
 * clocksource_hz2mult - calculates mult from hz and shift
 * @hz: Clocksource frequency in Hz
 * @shift_constant:	Clocksource shift factor
 *
 * Helper functions that converts a hz counter
 * frequency to a timsource multiplier, given the
 * clocksource shift value
 */
static inline u32_t clocksource_hz2mult(u32_t hz, u32_t shift)
{
    /*
	 * hz = cyc/(Billion ns)
	 * mult/2^shift  = ns/cyc
	 * mult = ns/cyc * 2^shift
	 * mult = 1Billion/hz * 2^shift
	 * mult = 1000000000 * 2^shift / hz
	 * mult = (1000000000<<shift) / hz
	 */
    u64_t tmp = ((u64_t)1000000000ULL) << shift;
    tmp += hz / 2;
    tmp = tmp / hz;
    return (u32_t)tmp;
}

/*
 * clocksource_khz2mult - calculates mult from khz and shift
 * @khz: Clocksource frequency in KHz
 * @shift_constant: Clocksource shift factor
 *
 * Helper functions that converts a khz counter frequency to a timsource
 * multiplier, given the clocksource shift value
 */
static inline u32_t clocksource_khz2mult(u32_t khz, u32_t shift)
{
    /*
	 * khz = cyc/(Million ns)
	 * mult/2^shift  = ns/cyc
	 * mult = ns/cyc * 2^shift
	 * mult = 1Million/khz * 2^shift
	 * mult = 1000000 * 2^shift / khz
	 * mult = (1000000<<shift) / khz
	 */
    u64_t tmp = ((u64_t)1000000ULL) << shift;
    tmp += khz / 2;
    tmp = tmp / khz;
    return (u32_t)tmp;
}

/*
 * clocksource_calc_mult_shift - calculate mult/shift factors for scaled math of clocksource
 * @mult:	pointer to mult variable
 * @shift:	pointer to shift variable
 * @from:	frequency to convert from
 * @to:		frequency to convert to
 * @maxsec:	guaranteed runtime conversion range in seconds
 *
 * The function evaluates the shift/mult pair for the scaled math
 * operations of clocksources and clockevents.
 *
 * @to and @from are frequency values in HZ. For clock sources @to is
 * NSEC_PER_SEC == 1GHz and @from is the counter frequency. For clock
 * event @to is the counter frequency and @from is NSEC_PER_SEC.
 *
 * The @maxsec conversion range argument controls the time frame in
 * seconds which must be covered by the runtime conversion with the
 * calculated mult and shift factors. This guarantees that no 64bit
 * overflow happens when the input value of the conversion is
 * multiplied with the calculated mult factor. Larger ranges may
 * reduce the conversion accuracy by chosing smaller mult and shift
 * factors.
 */
static inline void clocksource_calc_mult_shift(u32_t *mult, u32_t *shift, u32_t from, u32_t to, u32_t maxsec)
{
    u64_t tmp;
    u32_t sft, sftacc = 32;

    /*
     * Firstly, try to calculate to / from
     * TODO: Check it in arm platform
     */

    u32_t q = to / from;

    if (q * from == to)
    {
        *mult = q;
        *shift = 0;
        return;
    }

    /*
	 * Calculate the shift factor which is limiting the conversion range
	 */
    tmp = ((u64_t)maxsec * from) >> 32;
    while (tmp)
    {
        tmp >>= 1;
        sftacc--;
    }

    /*
	 * Find the conversion shift/mult pair which has the best
	 * accuracy and fits the maxsec conversion range:
	 */
    for (sft = 32; sft > 0; sft--)
    {
        tmp = (u64_t)to << sft;
        tmp += from / 2;
        tmp = tmp / from;
        if ((tmp >> sftacc) == 0)
            break;
    }
    *mult = tmp;
    *shift = sft;
}

static inline u64_t clocksource_deferment(struct clocksource_t *cs)
{
    return ((u64_t)cs->mask * cs->mult) >> cs->shift;
}

static inline u64_t clocksource_cycle(struct clocksource_t *cs)
{
    return cs->read(cs) & cs->mask;
}

static inline u64_t clocksource_delta(struct clocksource_t *cs, u64_t last, u64_t now)
{
    if (last < now)
        return (now - last) & cs->mask;
    return (cs->mask + 1 - last + now) & cs->mask;
}

static inline u64_t clocksource_delta2ns(struct clocksource_t *cs, u64_t delta)
{
    return (delta * cs->mult) >> cs->shift;
}

static inline ktime_t clocksource_keeper_read(struct clocksource_t *cs)
{
    // TODO: refer to xboot
    u64_t now, delta, offset;
    now = clocksource_cycle(cs);
    offset = clocksource_delta2ns(cs, now);
    return ns_to_ktime(offset);
}

void register_default_clocksource(clocksource_t *cs);
ktime_t ktime_get(void);
kticks_t kticks_get(void);
kticks_t kticks_ticks_per_second(void);