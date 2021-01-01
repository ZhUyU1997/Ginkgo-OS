#pragma once

#include <macro/base.h>
#include <macro/eval.h>

#define _FOR_EACH(macro, x, ...)           \
    CAT(_FOR_EACH_, IS_EMPTY(__VA_ARGS__)) \
    (macro, x, __VA_ARGS__)
#define _FOR_EACH_0(macro, x, ...) macro(x) DEFER(_FOR_EACH_I)()(macro, __VA_ARGS__)
#define _FOR_EACH_1(macro, x, ...) macro(x)
#define _FOR_EACH_I() _FOR_EACH
#define FOR_EACH(macro, ...) EVAL(_FOR_EACH(macro, __VA_ARGS__))