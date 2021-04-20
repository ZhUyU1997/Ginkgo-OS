#include <core/test.h>
#include <core/syscall.h>
#include <log.h>

TEST(sys_nanosleep)
{
    s64_t clock1 = sys_clock_get_monotonic().tv64;
    sc_status_t s = sys_nanosleep(sys_deadline_after(1000000));
    s64_t clock2 = sys_clock_get_monotonic().tv64;

    ASSERT_TRUE(s == SS_OK);
    ASSERT_TRUE((clock1 + 1000000) <= clock2);
}