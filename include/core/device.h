#pragma once
/*
Refer to:
the virtio spec:
    https://docs.oasis-open.org/virtio/virtio/v1.1/virtio-v1.1.pdf
Linux:
    /usr/include/linux/virtio*.h
Qemu:
    /include/standard-headers/linux/virtio*.h
 */

#pragma once

#include <types.h>
#include <xjil.h>
#include <core/class.h>

class(device_t)
{
    char *name;
    int (*probe)(device_t * this, xjil_value_t * value);
    void (*remove)(device_t * this);
    void (*suspend)(device_t * this);
    void (*resume)(device_t * this);
};

#define register_driver(type_name)            \
    extern type_index type_name##_class_type; \
    static type_index *__driver_type_index    \
        __attribute__((__used__, __section__(".driver.type.index"))) = &type_name##_class_type

void do_init_device();