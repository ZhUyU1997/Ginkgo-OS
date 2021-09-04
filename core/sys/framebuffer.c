#include <framebuffer/framebuffer.h>
#include <task.h>
#include <log.h>

static framebuffer_t *fb = NULL;
static struct surface_t *surface = NULL;
u64_t sys_framebuffer_create(u64_t addr)
{
    fb = search_device_with_class(framebuffer_t, "virtio-framebuffer");
    surface = fb->create(fb);
    LOGD("surface->pixels:"$(surface->pixels));
    map_range_in_pgtbl(thread_self()->vmspace->pgtbl, 0x40000000UL, (paddr_t)surface->pixels, PGROUNDUP(surface->height * surface->stride), PTE_R | PTE_W | PTE_X | PTE_U);
    return 0x40000000UL;
}

void sys_framebuffer_get_info(uint32_t *width,
                              uint32_t *height,
                              uint32_t *stride)
{
    *width = surface->width;
    *height = surface->height;
    *stride = surface->stride;
}

void sys_framebuffer_present()
{
    fb->present(fb, surface);
}