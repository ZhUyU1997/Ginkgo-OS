/*
 * libc/stdio/fwrite.c
 */

#include <stdio.h>

size_t fwrite(const void *buf, size_t size, size_t count, FILE *f)
{
	size_t request = size * count;
	size_t written;

	if (request == 0)
		return 0;

	written = __stdio_write(f, buf, request);
	return written == request ? count : written / size;
}
