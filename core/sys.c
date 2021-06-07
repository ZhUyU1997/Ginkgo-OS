
#include <types.h>

extern void print_char(u8_t c);
void sys_putc(char c)
{
    print_char(c);
}