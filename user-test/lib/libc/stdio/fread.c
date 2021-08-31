/*
 * libc/stdio/fread.c
 */

#include <stdio.h>

size_t fread(void *buf, size_t size, size_t count, FILE *f)
{
	size_t bytes_requested = size * count;
	size_t bytes_read;

	if (bytes_requested == 0)
		return 0;

	bytes_read = __stdio_read(f, buf, size);
	return bytes_read == bytes_requested ? count : bytes_read / size;
}
