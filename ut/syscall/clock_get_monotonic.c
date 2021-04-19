#include <core/test.h>
#include <core/syscall.h>
#include <log.h>

TEST(sys_clock_get_monotonic)
{
    u64_t clock = sys_clock_get_monotonic().tv64;
}