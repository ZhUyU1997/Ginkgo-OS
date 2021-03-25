#include <stddef.h>
#include "str.h"

typedef string_t T;
typedef char E;

#include "array-template.h"

string_t *string_new(size_t capacity)
{
    return _array_new(capacity);
}

void string_destory(string_t *arr)
{
    _array_destory(arr);
}

void string_copy(string_t *dst, string_t *src)
{
    _array_copy(dst, src);
}

void string_clear(string_t *arr)
{
    _array_clear(arr);
}

void string_push(string_t *arr, E ele)
{
    _array_push(arr, ele);
}

E string_get(string_t *arr, size_t index)
{
    return _array_get(arr, index);
}

size_t string_size(string_t *arr)
{
    return _array_size(arr);
}