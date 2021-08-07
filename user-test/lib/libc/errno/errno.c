/*
 * libc/errno/errno.c
 */

#include <errno.h>

static volatile int _errno = 0;

volatile int * __task_errno_location(void)
{
	return &(_errno);
}
