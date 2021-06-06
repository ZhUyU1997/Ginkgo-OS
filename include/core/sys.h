#pragma once

#include <types.h>

typedef s32_t status_t; // for syscall
typedef u32_t handle_t;

typedef u32_t signals_t;
typedef u32_t koid_t;
typedef struct wait_item_t
{
    handle_t handle;
    signals_t waitfor;
    signals_t pending;
} wait_item_t;