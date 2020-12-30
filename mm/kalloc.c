#include "buddy-common.h"

#include <riscv.h>
#include <memlayout.h>
#include <list.h>
#include <types.h>

#include <mm/buddy.h>
#include <mm/mem_pool.h>


static buddy_system_t* system = NULL;
static mem_pool_control_t* control = &(mem_pool_control_t) {};

extern char end[];

void* alloc_page(size_t count)
{
    return buddy_alloc_page(system, count);
}

void free_page(void* ptr)
{
    buddy_free_page(system, ptr);
}

void* malloc(size_t size)
{
    if (size > MEM_POOL_MAX_SIZE)
        return alloc_page(align_up(size, PAGE_SIZE) >> PAGE_SHIFT);
    return mem_pool_alloc(control, size);
}

void free(void* ptr)
{
    if (((size_t)ptr & (PAGE_SIZE - 1)) == 0)
        free_page(ptr);
    else
        mem_pool_free(control, ptr);
}

void kinit()
{
    system = buddy_create(end, (size_t)PHYSTOP - (size_t)end);
    mem_pool_create(control, system);
}