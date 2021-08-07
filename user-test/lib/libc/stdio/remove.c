/*
 * libc/stdio/remove.c
 */

#include <stdio.h>
#include <vfs/vfs.h>

int remove(const char * path)
{
	return vfs_unlink(path);
}
