#include <core/test.h>
#include <core/syscall.h>
#include <log.h>

TEST(sys_deadline_after)
{
    s64_t clock1 = sys_clock_get_monotonic().tv64;
    s64_t clock2 = sys_deadline_after(1000000).tv64;
    ASSERT_TRUE((clock1 + 1000000) <= clock2);
}