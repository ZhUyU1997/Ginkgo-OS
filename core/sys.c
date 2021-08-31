
#include <types.h>

extern void serial_lowlevel_putc(u8_t c);
extern size_t serial_lowlevel_puts(const char *buf);

void sys_console_putc(char c)
{
    serial_lowlevel_putc(c);
}

size_t sys_console_puts(const char *buf)
{
    return serial_lowlevel_puts(buf);
}