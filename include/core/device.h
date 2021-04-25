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

void do_init_device();

device_t *search_device(const char *name);

#define search_device_with_class(type, name) ((type *)class_cast(type##_class_type, (char *)(search_device(name))))
