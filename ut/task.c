#include <core/test.h>
#include <core/timer.h>
#include <log.h>
#include <task.h>

static volatile bool_t isCalled = FALSE;

static void test()
{
    isCalled = TRUE;
}

TEST(task_create)
{
    task_resume(task_create("init", test));
    schedule();
    while (isCalled == FALSE);
    ASSERT_TRUE(isCalled);
}