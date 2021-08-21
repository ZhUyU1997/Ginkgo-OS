#include <core/device.h>
#include <core/class.h>
#include <log.h>
#include <hmap.h>
#include <xjil.h>
#include <string.h>

class_impl(device_t){};

extern unsigned char __dtree_start;
extern unsigned char __dtree_end;

static struct hmap_t *map = NULL;
static struct hmap_t *device_map = NULL;

static void register_device(type_index index)
{
    struct list_head *child = get_class_child(index);
    struct class_table_info *info;

    list_for_each_entry(info, child, list)
    {
        LOGI("Register [" $(__class_name(info->type)) "]");
        hmap_add(map, __class_name(info->type), &info->type);
    }

    list_for_each_entry(info, child, list)
    {
        register_device(info->type);
    }
}

void do_device_init()
{
    map = hmap_alloc(0);
    device_map = hmap_alloc(0);

    register_device(class_type(device_t));

    xjil_value_t *v = xjil_new();
    xjil_parse(v, (const char *)&__dtree_start, &__dtree_end - &__dtree_start);

    if (xjil_is_array(v))
    {
        size_t size = xjil_get_array_size(v);
        for (int i = 0; i < size; i++)
        {
            xjil_value_t *value = xjil_get_array_element(v, i);
            const char *tag = xjil_value_get_tag(value);
            if (tag)
            {
                type_index *index = hmap_search(map, tag);

                if(index == NULL)
                {
                    PANIC("Unable to find "$(tag));
                }

                LOGI("Init [" $(tag) "] type-index [" $(*index) "]");

                void *obj = new_class_object(*index);

                if (obj == NULL)
                {
                    PANIC("Failed to new " $(tag));
                }

                device_t *dev = dynamic_cast(device_t)(obj);
                int ret = dev->probe(dev, value);

                if (ret != 0)
                    delete_class_object(dev);

                const char *name = xjil_read_string(value, "name", NULL);

                if (name)
                {
                    if (search_device(name) != NULL)
                    {
                        PANIC($(name) " existed");
                    }
                    hmap_add(device_map, strdup(name), dev);
                }
            }
        }
    }
    xjil_del(v);
}

device_t *search_device(const char *name)
{
    device_t *dev = hmap_search(device_map, name);
    return dev;
}