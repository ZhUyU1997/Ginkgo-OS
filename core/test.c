#include <core/test.h>
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

        void *data = (ut->setup != NULL) ? ut->setup(ut) : NULL;
        ut->run(ut, data);
        if (ut->clean != NULL)
        {
            ut->clean(ut, data);
        }

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
    while (1)
        ;
}