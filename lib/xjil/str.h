#include <stddef.h>

typedef struct
{
    char *arr;
    size_t size, capacity;
} string_t;

string_t *string_new(size_t capacity);
void string_destory(string_t *arr);
void string_copy(string_t *dst, string_t *src);
void string_clear(string_t *arr);
void string_push(string_t *arr, char ele);
char string_get(string_t *arr, size_t index);
size_t string_size(string_t *arr);