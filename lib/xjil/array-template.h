#include <string.h>
#include <assert.h>
#include <malloc.h>

static T *_array_new(size_t capacity)
{
    T *arr = malloc(sizeof(T));
    assert(arr);
    arr->arr = malloc(sizeof(E) * capacity);
    arr->capacity = capacity;
    return arr;
}

static void _array_destory(T *arr)
{
    assert(arr);

    free(arr->arr);
    arr->arr = NULL;
    arr->capacity = arr->size = 0;
    free(arr);
}

static void _array_resize(T *arr, size_t capacity)
{
    assert(arr);
    arr->arr = realloc(arr->arr, sizeof(E) * capacity);
    arr->capacity = capacity;

    if (capacity < arr->size)
        arr->size = capacity;
}

static void _array_copy(T *dst, T *src)
{
    assert(dst && src);
    _array_resize(dst, src->size);
    memcpy(dst->arr, src->arr, sizeof(E) * src->size);
    dst->size = src->size;
}

static void _array_clear(T *arr)
{
    assert(arr);
    arr->size = 0;
}

static void _array_push(T *arr, E ele)
{
    assert(arr);

    if (arr->size >= arr->capacity)
    {
        _array_resize(arr, arr->capacity * 2);
    }

    arr->arr[arr->size++] = ele;
}

static E _array_get(T *arr, size_t index)
{
    assert(arr && (index < arr->size));
    return arr->arr[index];
}

static size_t _array_size(T *arr)
{
    assert(arr);
    return arr->size;
}