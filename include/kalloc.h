#pragma once

#include <types.h>

void free_page(void* pa);
void* alloc_page(size_t count);
void kinit();