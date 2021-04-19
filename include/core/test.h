#include <core/class.h>
#include <types.h>

class(unit_test)
{
    bool_t result;
    void *(*setup)(unit_test * this);
    void (*clean)(unit_test * this, void *data);
    void (*run)(unit_test * this, void *data);
};

#define TEST_SETUP(name) void *__test_##name##_setup(unit_test *this)
#define TEST_CLEAN(name) void __test_##name##_clean(unit_test *this, void *data)

#define TEST(name)                                                    \
    TEST_SETUP(name)                                                  \
    __attribute__((weak));                                            \
    TEST_CLEAN(name)                                                  \
    __attribute__((weak));                                            \
    void __test_##name##_run(unit_test *this, void *data);            \
    class(name##_unit_test, unit_test){};                             \
    class_impl(name##_unit_test, unit_test){};                        \
    constructor(name##_unit_test)                                     \
    {                                                                 \
        dynamic_cast(unit_test)(this)->setup = __test_##name##_setup; \
        dynamic_cast(unit_test)(this)->clean = __test_##name##_clean; \
        dynamic_cast(unit_test)(this)->run = __test_##name##_run;     \
    }                                                                 \
    void __test_##name##_run(unit_test *this, void *data)

void do_all_test();