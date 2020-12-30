#pragma once

#include <types.h>
#include <assert.h>
#include <printv.h>

#define type_cast(t, exp) ((t)(exp))
#define arrary_cast(p, size) (((char(*)[size])(p)))

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define min(x, y) ({typeof(x) _x = (x);typeof(y) _y = (y);(void)(&_x == &_y);_x < _y ? _x : _y; })

#define BUDDY_DEBUG
#ifdef BUDDY_DEBUG
#define LOGI(...) printv("[" $((char *)__func__) ":" $(__LINE__) "] "__VA_ARGS__, "\n")
#define LOGD(...) printv("[" $((char *)__func__) ":" $(__LINE__) "] "__VA_ARGS__, "\n")
#define LOGW(...) printv("[" $((char *)__func__) ":" $(__LINE__) "] "__VA_ARGS__, "\n")
#define LOGE(...) printv("[" $((char *)__func__) ":" $(__LINE__) "] "__VA_ARGS__, "\n")
#define LOGF(...) printv("[" $((char *)__func__) ":" $(__LINE__) "] "__VA_ARGS__, "\n")

#else

#define LOGI(...)
#define LOGD(...)
#define LOGW(...) printv("[" $((char *)__func__) ":" $(__LINE__) "] "__VA_ARGS__, "\n")
#define LOGE(...) printv("[" $((char *)__func__) ":" $(__LINE__) "] "__VA_ARGS__, "\n")
#define LOGF(...) printv("[" $((char *)__func__) ":" $(__LINE__) "] "__VA_ARGS__, "\n")

#undef assert
#define assert(...)
#endif

#define buddy_assert(x, msg) assert((x) && (msg))

/*
** Detect whether or not we are building for a 32- or 64-bit (LP/LLP)
** architecture. There is no reliable portable method at compile-time.
*/
#if defined(__alpha__) || defined(__ia64__) || defined(__x86_64__) || defined(_WIN64) || defined(__LP64__) || defined(__LLP64__)
#define CONFIG_64BIT
#endif


#if 0
static inline int buddy_ffs(unsigned int word)
{
    return __builtin_ffs(word) - 1;
}

static inline int buddy_fls(unsigned int word)
{
    const int bit = word ? 32 - __builtin_clz(word) : 0;
    return bit - 1;
}

#else

#include <ffs.h>
#include <fls.h>

static inline int buddy_ffs(unsigned int word)
{
    return ffs(word) - 1;
}

static inline int buddy_fls(unsigned int word)
{
    return fls(word) - 1;
}

#endif
/* Possibly 64-bit version of buddy_fls. */
#if defined(CONFIG_64BIT)
static int buddy_fls_sizet(size_t size)
{
    int high = (int)(size >> 32);
    int bits = 0;
    if (unlikely(high))
    {
        bits = 32 + buddy_fls(high);
    }
    else
    {
        bits = buddy_fls((int)size & 0xffffffff);
    }
    return bits;
}

static int buddy_ffs_sizet(size_t size)
{
    int low = (int)(size);
    int bits = 0;
    if (low)
    {
        bits = buddy_ffs(low);
    }
    else
    {
        bits = 32 + buddy_ffs((int)(size >> 32));
    }
    return bits;
}
#else
#define buddy_fls_sizet buddy_fls
#define buddy_ffs_sizet buddy_ffs
#endif

static size_t align_up(size_t x, size_t align)
{
    buddy_assert(0 == (align & (align - 1)), "must align to a power of two");
    return (x + (align - 1)) & ~(align - 1);
}

static size_t align_down(size_t x, size_t align)
{
    buddy_assert(0 == (align & (align - 1)), "must align to a power of two");
    return x - (x & (align - 1));
}

static void* align_ptr(const void* ptr, size_t align)
{
    assert(ptr && align);

    const ptrdiff_t aligned =
        (type_cast(ptrdiff_t, ptr) + (align - 1)) & ~(align - 1);
    buddy_assert(0 == (align & (align - 1)), "must align to a power of two");
    return type_cast(void*, aligned);
}