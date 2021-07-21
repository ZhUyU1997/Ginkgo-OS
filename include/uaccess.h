#pragma once

#include <string.h>

static inline int copy_from_user(char *kernel_buf, char *user_buf, size_t size)
{
	/* validate user_buf */
	memcpy(kernel_buf, user_buf, size);
	return 0;
}

static inline int copy_to_user(char *user_buf, char *kernel_buf, size_t size)
{
	/* validate user_buf */
	memcpy(user_buf, kernel_buf, size);
	return 0;
}

#define assign_struct_from_user(kernel, user) copy_from_user((char *)(kernel), (char *)(user), sizeof(*(kernel)))
#define assign_struct_to_user(user, kernel) copy_to_user((char *)(user), (char *)(kernel), sizeof(*(kernel)))
// #define get_var_from_user(user) ({typeof(*user) _t, copy_from_user((char *)(&_t), (char *)(user), sizeof(typeof(*user))); _t})
// #define set_var_to_user(user, value) copy_to_user((char *)(user), (char *)(&value), sizeof(value))