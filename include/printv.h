#pragma once

#include <printf.h>
#include <macro/for-each.h>

#define PRINT_FUNC(fmt, type) \
    type:                     \
    print_##fmt

#define PRINT_TEMPLATE(fmt, type)              \
    static inline void print_##fmt(type value) \
    {                                          \
        printf("%" #fmt, value);               \
    }
#define PRINT_TEMPLATE2(fmt, type) \
    static inline void print_##fmt(type value)

#define PRINT_0(value) _Generic(      \
    (value),                          \
    PRINT_FUNC(s, char *),            \
    PRINT_FUNC(d, int),               \
    PRINT_FUNC(u, unsigned int),      \
    PRINT_FUNC(p, long int),          \
    PRINT_FUNC(x, unsigned long int), \
    PRINT_FUNC(p, default))(value);

// #define PRINT_0(x)  x;
#define PRINT_1(x)  
#define __PRINTV(x) CAT(PRINT_,IS_EMPTY(x))(x)
#define _PRINTV_0(...) ({ FOR_EACH(__PRINTV, ##__VA_ARGS__) })
#define _PRINTV_1(...)

#define _PRINTV(...)                      \
    CAT(_PRINTV_, IS_EMPTY(__VA_ARGS__)) \
    (__VA_ARGS__)
#define printv(...)  _PRINTV(__VA_ARGS__) 
#define $(x) , x,

PRINT_TEMPLATE(p, const void *);
PRINT_TEMPLATE(s, const char *);
PRINT_TEMPLATE(d, const int);
PRINT_TEMPLATE(u, const unsigned int);
PRINT_TEMPLATE2(x, const unsigned long int)
{
    printf("%p", (const void *)value);
}