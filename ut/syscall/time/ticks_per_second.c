#include <core/test.h>
#include <core/syscall.h>
#include <log.h>

TEST(sys_ticks_per_second)
{
    kticks_t ticks = sys_ticks_per_second();
    ASSERT_TRUE(ticks > 0);
}