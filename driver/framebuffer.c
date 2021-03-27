#include <framebuffer/framebuffer.h>
#include <malloc.h>

class_impl(framebuffer_t, device_t){};

struct surface_t *surface_create(int width, int height, int pixlen)
{
    struct surface_t *surface = malloc(sizeof(struct surface_t));
    void *pixels = malloc(width * height * pixlen);
    surface->width = width;
    surface->height = height;
    surface->pixlen = pixels;
    surface->stride = width * pixlen;
    surface->pixels = pixels;
    return surface;
}

void surface_destory(struct surface_t *surface)
{
    free(surface->pixels);
    *surface = (struct surface_t){};
    free(surface);
}
