#include <core/device.h>
#include <framebuffer/framebuffer.h>
#include <task.h>
#include <riscv.h>
#include <vm.h>
#include <log.h>

void *sys_create()
{
    LOGI();
    framebuffer_t *fb = search_device_with_class(framebuffer_t, "virtio-framebuffer");
    struct surface_t *surface = fb->create(fb);
    pagetable_map(current->pagetable, 0x30000000UL, surface->pixels, surface->height * surface->stride, PTE_R | PTE_W | PTE_U);
    local_flush_tlb_all();

    return 0x30000000UL;
}

void sys_present()
{
    framebuffer_t *fb = search_device_with_class(framebuffer_t, "virtio-framebuffer");
    fb->present(fb, fb->surface);
}