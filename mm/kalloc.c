#include "buddy-common.h"

#include <riscv.h>
#include <memlayout.h>
#include <list.h>
#include <types.h>
#include <string.h>

#include <mm/buddy.h>
#include <mm/mem_pool.h>

static buddy_system_t *system = NULL;
static mem_pool_control_t *control = &(mem_pool_control_t){};

extern char __end[];

void *alloc_page(size_t count)
{
    return buddy_alloc_page(system, count);
}

void free_page(void *ptr)
{
    buddy_free_page(system, ptr);
}

void *malloc(size_t size)
{
    if (size > MEM_POOL_MAX_SIZE)
        return alloc_page(align_up(size, PAGE_SIZE) >> PAGE_SHIFT);
    return mem_pool_alloc(control, size);
}

void *calloc(size_t nmemb, size_t size)
{
    void *ptr = malloc(nmemb * size);

    if (ptr != NULL)
    {
        memset(ptr, 0, nmemb * size);
    }

    return ptr;
}

void free(void *ptr)
{
    if (((size_t)ptr & (PAGE_SIZE - 1)) == 0)
        free_page(ptr);
    else
        mem_pool_free(control, ptr);
}

void *realloc(void *old, size_t size)
{
    void *new = malloc(size);

    if (new)
    {
        memcpy(new, old, size);
        free(old);
    }

    return new;
}

void do_mem_init()
{
    system = buddy_create(__end, (size_t)PHYSTOP - (size_t)__end);
    mem_pool_create(control, system);
}