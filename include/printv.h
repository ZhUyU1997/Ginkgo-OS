#pragma once

#include "printf.h"

#define PRIMITIVE_CAT(x, y) x##y
#define CAT(x, y) PRIMITIVE_CAT(x, y)

#define __IS_EMPTY(a, b, size, ...) size
#define _IS_EMPTY(...) __IS_EMPTY(__VA_OPT__(,), 0, 1)
#define IS_EMPTY(...) _IS_EMPTY(__VA_ARGS__)
 
#define EMPTY()
#define DEFER(id) id EMPTY()

#define EVAL(...) EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL1(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL2(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL3(...) EVAL4(EVAL4(EVAL4(__VA_ARGS__)))
#define EVAL4(...) EVAL5(EVAL5(EVAL5(__VA_ARGS__)))
#define EVAL5(...) __VA_ARGS__

#define _FOR_EACH(macro, x, ...)           \
    CAT(_FOR_EACH_, IS_EMPTY(__VA_ARGS__)) \
    (macro, x, __VA_ARGS__)
#define _FOR_EACH_0(macro, x, ...) macro(x) DEFER(_FOR_EACH_I)()(macro, __VA_ARGS__)
#define _FOR_EACH_1(macro, x, ...) macro(x)
#define _FOR_EACH_I() _FOR_EACH
#define FOR_EACH(macro, ...) EVAL(_FOR_EACH(macro, __VA_ARGS__))

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

#define panic(...) ({printv(__VA_ARGS__);while(1);})