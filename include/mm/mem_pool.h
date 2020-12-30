#pragma once

#include <mm/buddy.h>

#include <types.h>
#include <list.h>

typedef struct
{
    struct list_head page;
    struct list_head free_block;
    size_t size;
} mem_pool_t;

typedef struct
{
    struct list_head list;
    mem_pool_t* pool;
    size_t free_count;
} mem_pool_header_t;

typedef struct
{
    struct list_head list;
} mem_pool_block_t;

enum
{
    MEM_POOL_PAGE_SIZE_LOG2 = 12,
    MEM_POOL_MIN_SIZE_LOG2 = 4,
    MEM_POOL_MIN_SIZE = (1 << MEM_POOL_MIN_SIZE_LOG2),
    MEM_POOL_ALIGN_SIZE_LOG2 = 4,
    MEM_POOL_ALIGN_SIZE = (2 * sizeof(size_t)),
    MEM_POOL_S_INDEX_COUNT_LOG2 = 2,
    MEM_POOL_S_INDEX_COUNT = (1 << MEM_POOL_S_INDEX_COUNT_LOG2),
    MEM_POOL_F_INDEX_COUNT = (MEM_POOL_PAGE_SIZE_LOG2 - MEM_POOL_MIN_SIZE_LOG2),
    MEM_POOL_MAX_SIZE = ((1 << MEM_POOL_PAGE_SIZE_LOG2) - (1 << (MEM_POOL_PAGE_SIZE_LOG2 - MEM_POOL_S_INDEX_COUNT_LOG2 - 1))),
};

typedef struct
{
    buddy_system_t* system;
    mem_pool_t pool[MEM_POOL_F_INDEX_COUNT][MEM_POOL_S_INDEX_COUNT];
} mem_pool_control_t;

int mem_pool_create(mem_pool_control_t* control, buddy_system_t* system);
void* mem_pool_alloc(mem_pool_control_t* control, size_t size);
void mem_pool_free(mem_pool_control_t* control, void* ptr);