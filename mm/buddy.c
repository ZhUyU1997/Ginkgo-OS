#include "buddy-common.h"

#include <mm/buddy.h>
#include <printv.h>
#include <list.h>


static void buddy_add_page(buddy_system_t* system, page_t* page)
{
    assert(system && page);

    int order = page->order;
    page->u32_flags = 0;
    page->flags.is_free = 1;

    list_add_tail(&page->list, &system->page_buddy[order]);
    system->bitmap |= 1UL << order;
    system->count[order]++;
}

static void buddy_del_page(buddy_system_t* system, page_t* page)
{
    assert(system && page);

    int order = page->order;
    list_del(&page->list);
    page->u32_flags = 0;
    page->flags.is_free = 0;

    if (list_empty(&system->page_buddy[order]))
        system->bitmap &= ~(1UL << order);
    system->count[order]--;
}

static void* page_to_ptr(buddy_system_t* system, page_t* page)
{
    ptrdiff_t diff = page - system->map;
    return &arrary_cast(system->page_start, PAGE_SIZE)[diff];
}

static page_t* page_from_ptr(buddy_system_t* system, void* ptr)
{
    assert(0 == ((size_t)ptr % PAGE_SIZE));
    ptrdiff_t diff = arrary_cast(ptr, PAGE_SIZE) - arrary_cast(system->page_start, PAGE_SIZE);
    return &system->map[diff];
}

static size_t page_to_pfn(buddy_system_t* system, page_t* page)
{
    assert(system);

    ptrdiff_t diff = page - system->map;
    return diff;
}

static page_t* page_from_pfn(buddy_system_t* system, size_t pfn)
{
    assert(system);
    return &system->map[pfn];
}

static int pfn_valid(buddy_system_t* system, size_t pfn)
{
    assert(system);

    return pfn <= system->max_pfn;
}

static int page_free(page_t* page)
{
    return page->flags.is_free;
}

static buddy_system_t* buddy_new(void* mem)
{
    assert(mem);
    buddy_system_t* system = mem;
    for (int order = 0; order <= MAX_ORDER; order++)
    {
        init_list_head(&system->page_buddy[order]);
        system->count[order] = 0UL;
    }
    system->bitmap = 0UL;
    system->page_start = NULL;

    return system;
}

static void buddy_debug(buddy_system_t* system)
{
#ifdef BUDDY_DEBUG
    assert(system);

    for (int order = MAX_ORDER - 1; order >= 0; order--)
    {
        printv($(system->count[order]));
    }
    printv("\n");
#endif
}

buddy_system_t* buddy_create(void* mem, size_t size)
{
    assert(mem && size);
    LOGD("mem:" $(mem) " size:"$(size));
    if (!(mem && size))
        return NULL;

    // Alloc buddy_system_t
    buddy_system_t* system = buddy_new(mem);
    mem += sizeof(buddy_system_t);
    size -= sizeof(buddy_system_t);

    size_t page_count = size >> PAGE_SHIFT;

    // Alloc page map
    size_t map_size = align_up(page_count * sizeof(page_t), PAGE_SIZE);
    page_count -= map_size / PAGE_SIZE;
    mem += map_size;
    size -= map_size;

    // Alloc page
    size_t mem_diff = (type_cast(ptrdiff_t, align_ptr(mem, PAGE_SIZE)) - type_cast(ptrdiff_t, mem));
    mem += mem_diff;
    size -= mem_diff;
    page_count = size >> PAGE_SHIFT;

    system->page_start = mem;
    system->max_pfn = page_count - 1;

    buddy_assert(((size_t)mem & PAGE_MASK) == 0, "must align to PAGE_SIZE");

    for (size_t i = 0; i < page_count; i++)
    {
        system->map[i].order = INVALID_ORDER;
    }

    int order = INVALID_ORDER;

    for (size_t i = 0, count = page_count; count; i += 1UL << order, count -= 1UL << order)
    {
        order = buddy_fls_sizet(count);
        assert(order >= 0);

        if (order > MAX_ORDER)
            order = MAX_ORDER;

        page_t* p = &system->map[i];
        init_list_head(&p->list);
        p->order = order;
        buddy_add_page(system, p);
    }

    return system;
}

static page_t* buddy_locate_free(buddy_system_t* system, int order)
{
    assert(system && (order >= 0));
    assert(list_empty(&system->page_buddy[order]) == (system->count[order] == 0));

    if (list_empty(&system->page_buddy[order]))
    {
        size_t bitmap = system->bitmap & (~0UL << order);
        if (!bitmap)
        {
            LOGE("Cannot find free page!");
            return NULL;
        }

        order = buddy_ffs_sizet(bitmap);
        assert(order >= 0);
    }

    return list_last_entry(&system->page_buddy[order], page_t, list);
}

static void page_split(buddy_system_t* system, page_t* page, int order)
{
    assert(system && page && (order >= 0));

    for (int i = page->order; i > order; i--)
    {
        int new = i - 1;
        page_t* buddy = &page[1UL << new];
        buddy->order = new;
        buddy_add_page(system, buddy);
    }
    page->order = order;
}

static page_t* page_merge(buddy_system_t* system, page_t* page)
{
    assert(system && page);

    int order = page->order;
    size_t pfn = page_to_pfn(system, page);

    for (; order <= MAX_ORDER;)
    {
        size_t buddy_pfn = pfn ^ (1UL << order);
        page_t* buddy = page + (buddy_pfn - pfn);

        if (!pfn_valid(system, buddy_pfn))
            break;
        if (buddy->order != order)
            break;
        if (!page_free(buddy))
            break;

        buddy_del_page(system, buddy);

        size_t private_pfn = buddy_pfn | pfn;
        page_t* private = page + (private_pfn - pfn);
        private->order = INVALID_ORDER;

        size_t combined_pfn = buddy_pfn & pfn;
        page = page + (combined_pfn - pfn);
        pfn = combined_pfn;
        order++;
    }

    page->order = order;
    return page;
}

static void* page_prepare_used(buddy_system_t* system, page_t* page, int order)
{
    assert(system && page && (order >= 0));

    buddy_del_page(system, page);

    if (page->order > order)
    {
        page_split(system, page, order);
    }

    return page_to_ptr(system, page);
}

void* buddy_alloc_page(buddy_system_t* system, size_t count)
{
    assert(system && count);

    if (count == 0UL)
    {
        LOGE("count == 0");
        return NULL;
    }

    int order = buddy_fls_sizet(count + ((1UL << buddy_fls_sizet(count)) - 1));

    page_t* page = buddy_locate_free(system, order);

    if (!page)
    {
        LOGE("Failed to call buddy_locate_free");
        return NULL;
    }
    return page_prepare_used(system, page, order);
}

void buddy_free_page(buddy_system_t* system, void* ptr)
{
    assert(system && ptr);

    if (ptr)
    {
        page_t* page = page_from_ptr(system, ptr);

        if (page_free(page))
        {
            LOGE("Double free!");
            return;
        }
        page = page_merge(system, page);
        buddy_add_page(system, page);
    }
}