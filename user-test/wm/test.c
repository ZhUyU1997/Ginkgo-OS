#include <stddef.h>
#include <syscall.h>
#include <types.h>
#include <stdio.h>
#include <malloc.h>
#include <ipc.h>
#include <gato/render.h>

// typedef int wid_t;
// wid_t wm_window_create(int x, int y, unsigned int width, unsigned int height);
// void wm_window_destory(wid_t id);
// void wm_window_config_set(wid_t id);
// void wm_window_config_get(wid_t id);

static void draw_logo(surface_t *base)
{
    surface_clear(base, (color_t){255, 255, 255, 255}, 0, 0, base->width, base->height);
    draw_svg(base, "M135.422 298.027C135.372 301.487 140.134 305.639 140.134 305.639C140.134 305.639 146.534 313.927 144.173 299.284C141.812 284.641 166.115 211.848 194.532 191.225C222.949 170.603 335.232 153.712 318.381 121.817C301.53 89.921 276.275 66.787 264.472 68.125C252.669 69.4631 220.275 114.654 220.275 114.654C220.275 114.654 247.815 56.6292 234.902 43.9376C221.99 31.246 153.982 38.3774 150.676 58.2741C147.37 78.1708 183.995 174.457 171.431 209.06C158.867 243.664 135.473 294.566 135.422 298.027Z",
             331, 338, 331, 338, (640 - 331) / 2, (480 - 338) / 2, RGB(0x8EAB7D));
    draw_svg(base, "M179.86 325.388C179.079 329.342 172.812 332.909 172.812 332.909C172.812 332.909 163.734 340.788 169.885 324.682C176.036 308.576 166.783 219.702 140.364 189.262C113.946 158.823 -6.06383 112.144 20.2748 79.9217C46.6135 47.6994 80.1209 27.5129 92.8416 31.9212C105.562 36.3295 130.434 95.732 130.434 95.732C130.434 95.732 114.031 22.8935 131.372 11.5886C148.713 0.283645 222.15 25.0256 220.991 48.5024C219.832 71.9791 156.066 172.732 161.582 215.227C167.098 257.722 180.641 321.433 179.86 325.388Z",
             331, 338, 331, 338, (640 - 331) / 2, (480 - 338) / 2, ARGB(0xC16EE028));
}

static int exit_flag = 0;
int main(int argc, char **argv)
{
    u64_t addr = usys_framebuffer_create(0);
    printf("usys_framebuffer_create %p\n", addr);

    uint32_t width, height, stride;
    usys_framebuffer_get_info(&width, &height, &stride);
    printf("[%d %d %d]\n", width, height, stride);

    surface_t *base = &(surface_t){0};
    surface_wrap(base, (color_t *)addr, width, height);
    draw_logo(base);
    usys_framebuffer_present();

    while (exit_flag != 1)
        usys_yield();

    return 0;
}