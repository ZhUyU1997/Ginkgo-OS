#include <core/test.h>
#include <core/syscall.h>
#include <log.h>

TEST(sys_clock_get_monotonic)
{
    s64_t clock = sys_clock_get_monotonic().tv64;
    ASSERT_TRUE(clock > 0);
}