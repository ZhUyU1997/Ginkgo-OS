#include <core/test.h>
#include <core/time.h>
#include <log.h>

static void *setup(unit_test *this)
{
    return NULL;
}

static void clean(unit_test *this, void *data)
{
}

static void run(unit_test *this, void *data)
{
}

class_impl(unit_test){
    .result = TRUE,
    .setup = setup,
    .clean = clean,
    .run = run,
};

static void print_time(ktime_t t)
{
    s64_t div = 1;
    const char *unit = "ns";
    if (ktime_to_ns(t) > 1000L)
    {
        div = 1000L;
        unit = "us";
    }
    if (ktime_to_ns(t) > 1000000L)
    {
        div = 1000000L;
        unit = "ms";
    }

    if (ktime_to_ns(t) > 1000000000L)
    {
        div = 1000000000L;
        unit = "s";
    }
    printv(" " $((int)(ktime_to_ns(t) / div)) $(unit));
}
static void do_unit_test(type_index index, int order)
{
    struct list_head *child = get_class_child(index);
    struct class_table_info *info;

    list_for_each_entry(info, child, list)
    {
        printv("Test [" $(order) "]");

        void *obj = new_class_object(info->type);

        if (obj == NULL)
        {
            printv("Failed to new " $(__class_name(info->type)) "\n");
            continue;
        }

        unit_test *ut = dynamic_cast(unit_test)(obj);
        printv("[" $(ut->ut_name) "]");

        ktime_t t1 = ktime_get();
        {
            void *data = (ut->setup != NULL) ? ut->setup(ut) : NULL;
            ut->run(ut, data);
            if (ut->clean != NULL)
            {
                ut->clean(ut, data);
            }
        }

        ktime_t t2 = ktime_get();

        print_time(ktime_sub(t2, t1));
        if (ut->result)
        {
            printv(" \e[32m[PASS]\e[0m\n");
        }
        else
        {
            printv(" \e[31m[FAIL]\e[0m\n");
        }

        order++;
        if (ut != 0)
            delete_class_object(ut);
    }

    list_for_each_entry(info, child, list)
    {
        do_unit_test(info->type, order);
    }
}

void do_all_test()
{
    printv("\n\n##################### Unit Test #####################\n");

    do_unit_test(class_type(unit_test), 1);
    printv("#####################    END    #####################\n");

    while (1)
        ;
}