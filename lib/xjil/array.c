#include <types.h>

#include "array.h"

typedef array_t T;
typedef void *E;

#include "array-template.h"

array_t *array_new(size_t capacity)
{
    return _array_new(capacity);
}

void array_destory(array_t *arr)
{
    _array_destory(arr);
}

void array_copy(array_t *dst, array_t *src)
{
    _array_copy(dst, src);
}

void array_clear(array_t *arr)
{
    _array_clear(arr);
}

void array_push(array_t *arr, E ele)
{
    _array_push(arr, ele);
}

void *array_get(array_t *arr, size_t index)
{
    return _array_get(arr, index);
}

size_t array_size(array_t *arr)
{
    return _array_size(arr);
}