#include <core/test.h>
#include <core/syscall.h>
#include <core/sys.h>
#include <log.h>

TEST(sys_clock_get)
{
    ktime_t time;
    sc_status_t s = sys_clock_get(CLOCK_MONOTONIC, &time);
    ASSERT_TRUE(s == SS_OK);
    ASSERT_TRUE(time.tv64 > 0);
}