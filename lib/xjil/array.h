#include <stddef.h>

typedef struct
{
    void **arr;
    size_t size, capacity;
} array_t;

array_t *array_new(size_t capacity);
void array_destory(array_t *arr);
void array_copy(array_t *dst, array_t *src);
void array_clear(array_t *arr);
void array_push(array_t *arr, void *ele);
void *array_get(array_t *arr, size_t index);
size_t array_size(array_t *arr);