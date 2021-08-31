#include "types.h"
#include "io.h"

#include <stddef.h>

volatile vaddr_t virt = 0x10000000;

void serial_lowlevel_putc(u8_t c)
{
    while ((read8(virt + 0x05) & (0x1 << 6)) == 0)
        ;
    write8(virt + 0x00, c);
}

size_t serial_lowlevel_puts(const char *buf, size_t size)
{
    for (int i = 0; buf[i] && (i < size); i++)
    {
        serial_lowlevel_putc(buf[i]);
    }
    return size;
}