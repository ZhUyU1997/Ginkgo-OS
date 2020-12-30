#pragma once

#include <types.h>
#include <list.h>

#define MAX_ORDER 20
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12
#define SHIFT_TO_MASK(s) ((1UL << s) - 1)
#define PAGE_MASK SHIFT_TO_MASK(PAGE_SHIFT)
#define ORDER_MASK (SHIFT_TO_MASK((MAX_ORDER + PAGE_SHIFT)) - PAGE_MASK)
#define INVALID_ORDER (-1)

typedef struct
{
    u32_t is_free : 1;
} page_flags_t;

typedef struct
{
    struct list_head list;
    union
    {
        page_flags_t flags;
        u32_t u32_flags;
    };

    u32_t order;
} page_t;

typedef struct
{
    struct list_head page_buddy[MAX_ORDER + 1];
    size_t count[MAX_ORDER + 1];
    size_t bitmap;
    void* page_start;
    size_t max_pfn;
    page_t map[0];
} buddy_system_t;

buddy_system_t* buddy_create(void* mem, size_t size);
void* buddy_alloc_page(buddy_system_t* system, size_t count);
void buddy_free_page(buddy_system_t* system, void* ptr);
