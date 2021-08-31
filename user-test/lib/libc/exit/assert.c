/*
 * libc/exit/assert.c
 */

#include <stdio.h>
#include <assert.h>

void __assert_fail(const char * expr, const char * file, int line, const char * func)
{
	// fprintf(stderr, "Assertion failed: %s (%s: %s: %d)\r\n", expr, file, func, line);
	// TODO
	printf("Assertion failed: %s (%s: %s: %d)\r\n", expr, file, func, line);
}
