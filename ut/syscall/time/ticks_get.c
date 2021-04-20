#include <core/test.h>
#include <core/syscall.h>
#include <log.h>

TEST(sys_ticks_get)
{
    kticks_t ticks = sys_ticks_get();
    ASSERT_TRUE(ticks > 0);
}