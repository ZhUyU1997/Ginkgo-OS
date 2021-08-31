/*
 * libc/stdio/rename.c
 */

#include <stdio.h>
#include <vfs.h>

int rename(const char * old, const char * new)
{
	return vfs_rename(old, new);
}
