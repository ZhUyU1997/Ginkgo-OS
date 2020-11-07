#include "types.h"
#include "print.h"

#include <stddef.h>

volatile virtual_addr_t virt = 0x10000000;

void print_u8(u8_t c)
{
    while ((read8(virt + 0x05) & (0x1 << 6)) == 0)
        ;
    write8(virt + 0x00, c);
}
