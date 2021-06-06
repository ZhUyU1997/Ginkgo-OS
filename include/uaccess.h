#pragma once

#include <string.h>

int copy_from_user(char *kernel_buf, char *user_buf, size_t size)
{
	/* validate user_buf */
	memcpy(kernel_buf, user_buf, size);
	return 0;
}

int copy_to_user(char *user_buf, char *kernel_buf, size_t size)
{
	/* validate user_buf */
	memcpy(user_buf, kernel_buf, size);
	return 0;
}