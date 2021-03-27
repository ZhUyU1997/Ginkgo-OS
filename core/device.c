#include <core/device.h>
#include <core/class.h>
#include <log.h>
#include <hmap.h>
#include <xjil.h>

class_impl(device_t){};

extern type_index *__driver_type_index_table_start[];
extern type_index *__driver_type_index_table_end[];
extern unsigned char __dtree_start;
extern unsigned char __dtree_end;

static struct hmap_t *map = NULL;

void do_init_device()
{
    // device_t *dev = dynamic_cast(device_t)(new (plic_irqchip_t));
    // dev->probe(dev, NULL);
    map = hmap_alloc(0);
    for (type_index **i = __driver_type_index_table_start; i < __driver_type_index_table_end; i++)
    {
        type_index index = **i;
        hmap_add(map, __class_name(index), index);
    }

    xjil_value_t *v = xjil_new();
    xjil_parse(v, &__dtree_start, &__dtree_end - &__dtree_start);

    if (xjil_is_array(v))
    {
        size_t size = xjil_get_array_size(v);
        for (int i = 0; i < size; i++)
        {
            xjil_value_t *value = xjil_get_array_element(v, i);
            const char *tag = xjil_value_get_tag(value);
            if (tag)
            {
                LOGI("Init ["$(tag)"]");
                type_index index = hmap_search(map, tag);
                device_t *dev = dynamic_cast(device_t)(new_class_object(index));
                int ret = dev->probe(dev, value);
                
                if(ret != 0)
                    delete(dev);
            }
        }
    }
    xjil_del(v);
}
