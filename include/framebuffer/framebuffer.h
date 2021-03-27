#pragma once

#include <list.h>
#include <types.h>
#include <core/device.h>
#include <core/class.h>

struct surface_t
{
    int width;
    int height;
    int stride;
    int pixlen;
    void *pixels;
};

class(framebuffer_t, device_t)
{
    /* The width and height in pixel */
    int width, height;

    /* Create a surface */
    struct surface_t *(*create)(struct framebuffer_t * fb);

    /* Destroy a surface */
    void (*destroy)(struct framebuffer_t * fb, struct surface_t * s);

    /* Present a surface */
    void (*present)(struct framebuffer_t * fb, struct surface_t * s);
};

struct surface_t *surface_create(int width, int height, int pixlen);
void surface_destory(struct surface_t *surface);