#include "types.h"
#include "io.h"

#include <stddef.h>

volatile vaddr_t virt = 0x10000000;

void print_char(u8_t c)
{
    while ((read8(virt + 0x05) & (0x1 << 6)) == 0)
        ;
    write8(virt + 0x00, c);
}

void print(const char *buf)
{
    for (int i = 0; buf[i]; i++)
    {
        print_char(buf[i]);
    }
}