#include <core/test.h>
#include <core/timer.h>
#include <log.h>

static struct timer_t timer;
static volatile bool_t isCalled = FALSE;
static int function(struct timer_t *timer, void *data)
{
    isCalled = TRUE;
    return 0;
}

TEST(test_timer_1ms)
{
    isCalled = FALSE;
    timer_init(&timer, function, NULL);
    timer_start_now(&timer, ms_to_ktime(1));
    while(isCalled == FALSE);
    ASSERT_TRUE(isCalled);
}

TEST(test_timer_100ms)
{
    isCalled = FALSE;
    timer_init(&timer, function, NULL);
    timer_start_now(&timer, ms_to_ktime(100));
    while(isCalled == FALSE);
    ASSERT_TRUE(isCalled);
}