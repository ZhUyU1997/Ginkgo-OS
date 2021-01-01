#include "buddy-common.h"

#include <mm/mem_pool.h>
#include <mm/buddy.h>
#include <list.h>
#include <types.h>


static_assert((sizeof(mem_pool_header_t)& (2 * sizeof(size_t) - 1)) == 0, "pointer address must align to 2 * sizeof(size_t)");
static_assert((1UL << MEM_POOL_PAGE_SIZE_LOG2) == PAGE_SIZE, "");
static_assert((1UL << MEM_POOL_ALIGN_SIZE_LOG2) == MEM_POOL_ALIGN_SIZE, "");

static int mem_pool_alloc_page(mem_pool_t* pool, buddy_system_t* system)
{
    assert(pool && system);

    void* ptr = buddy_alloc_page(system, 1);

    if (ptr == NULL) {
        LOGE("Failed to call buddy_alloc_page!");
        return -1;
    }

    mem_pool_header_t* header = (mem_pool_header_t*)ptr;

    init_list_head(&header->list);
    header->free_count = (PAGE_SIZE - sizeof(mem_pool_header_t)) / pool->size;
    header->pool = pool;

    void* block_start = type_cast(char*, header) + sizeof(mem_pool_header_t);

    for (int k = 0; k < header->free_count; k++)
    {
        struct list_head* list = type_cast(struct list_head*, ((char*)block_start) + pool->size * k);
        list_add_tail(list, &pool->free_block);
    }

    return 0;
}

static void mem_pool_free_page(mem_pool_header_t* header, buddy_system_t* system)
{
    assert(header && system);

    if (header->free_count != 0)
        return;

    list_del(&header->list);
    buddy_free_page(system, type_cast(void*, header));
}

int mem_pool_create(mem_pool_control_t* control, buddy_system_t* system)
{
    assert(control && system);

    if (!(control && system))
        return -1;

    control->system = system;

    for (int i = 0; i < MEM_POOL_F_INDEX_COUNT; i++)
    {
        int size_log2 = (i + MEM_POOL_MIN_SIZE_LOG2);
        int shift = min(size_log2 - MEM_POOL_ALIGN_SIZE_LOG2, MEM_POOL_S_INDEX_COUNT_LOG2);
        int s_index_count = 1 << shift;
        size_t size = 1UL << size_log2;
        size_t inc_size = 1UL << (size_log2 - shift);

        for (int j = 0; j < s_index_count; j++)
        {
            mem_pool_t* pool = &control->pool[i][j];
            init_list_head(&pool->page);
            init_list_head(&pool->free_block);

            pool->size = size + inc_size * j;
            assert(0 == (pool->size & (MEM_POOL_ALIGN_SIZE - 1)));
        }
    }

    return 0;
}

static mem_pool_block_t* mem_pool_locate_free(mem_pool_control_t* control, size_t size)
{
    assert(control && size);

    int size_log2 = buddy_fls_sizet(size);
    int shift = min(size_log2 - MEM_POOL_ALIGN_SIZE_LOG2, MEM_POOL_S_INDEX_COUNT_LOG2);
    int f_index = size_log2 - MEM_POOL_MIN_SIZE_LOG2;
    int s_index = (size >> (size_log2 - shift)) & ((1UL << shift) - 1);

    LOGD("f_index:" $(f_index) " s_index:" $(s_index));

    mem_pool_t* pool = &control->pool[f_index][s_index];

    if (list_empty(&pool->free_block))
    {
        int ret = mem_pool_alloc_page(pool, control->system);

        if (ret)
            return NULL;
    }

    mem_pool_block_t* block = list_last_entry(&pool->free_block, mem_pool_block_t, list);
    return block;
}

static mem_pool_header_t* header_from_block(mem_pool_block_t* block)
{
    assert(((size_t)block & (PAGE_SIZE - 1)) != 0);
    return type_cast(mem_pool_header_t*, align_down((size_t)block, PAGE_SIZE));
}

static void mem_pool_add_block(mem_pool_block_t* block)
{
    assert(block);
    mem_pool_header_t* header = header_from_block(block);
    mem_pool_t* pool = header->pool;
    assert(pool);
    list_add_tail(&block->list, &pool->free_block);
    header->free_count++;
    assert(header->free_count < (PAGE_SIZE / MEM_POOL_ALIGN_SIZE));
}

static void mem_pool_del_block(mem_pool_block_t* block)
{
    assert(block);
    list_del(&block->list);
    mem_pool_header_t* header = header_from_block(block);
    header->free_count--;
    assert(header->free_count < (PAGE_SIZE / MEM_POOL_ALIGN_SIZE));
}

static void* mem_pool_prepare_used(mem_pool_control_t* control, mem_pool_block_t* block)
{
    assert(control && block);
    mem_pool_del_block(block);
    return block;
}

static void mem_pool_recycle(mem_pool_control_t* control, mem_pool_block_t* block)
{
    assert(control && block);
    mem_pool_add_block(block);

    mem_pool_header_t* header = header_from_block(block);
    if (header->free_count == 0)
        mem_pool_free_page(header, control->system);
}

static size_t adjust_request_size(size_t size)
{
    if (size < MEM_POOL_MIN_SIZE)
        size = MEM_POOL_MIN_SIZE;

    int size_log2 = buddy_fls_sizet(size);
    int shift = min(size_log2 - MEM_POOL_ALIGN_SIZE_LOG2, MEM_POOL_S_INDEX_COUNT_LOG2);

    size = align_up(size, 1UL << (size_log2 - shift));

    return size;
}

void* mem_pool_alloc(mem_pool_control_t* control, size_t size)
{
    assert(control);

    if (size == 0UL || size > MEM_POOL_MAX_SIZE)
        return NULL;

    const size_t adjust = adjust_request_size(size);
    mem_pool_block_t* block = mem_pool_locate_free(control, adjust);
    if (!block)
        return NULL;

    assert(((size_t)block & (MEM_POOL_ALIGN_SIZE - 1)) == 0);

    return mem_pool_prepare_used(control, block);
}

void mem_pool_free(mem_pool_control_t* control, void* ptr)
{
    assert(control && ptr);

    if (ptr == NULL)
        return;

    mem_pool_recycle(control, type_cast(mem_pool_block_t*, ptr));
}