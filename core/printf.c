#include "types.h"

#include <stdarg.h>

extern void serial_lowlevel_puts(const char *buf);
extern void serial_lowlevel_putc(u8_t c);

static char digits[] = "0123456789abcdef";

static void printint(int xx, int base, int sign)
{
    char buf[16];
    int i;
    uint32 x;

    if (sign && (sign = xx < 0))
        x = -xx;
    else
        x = xx;

    i = 0;
    do
    {
        buf[i++] = digits[x % base];
    } while ((x /= base) != 0);

    if (sign)
        buf[i++] = '-';

    while (--i >= 0)
        serial_lowlevel_putc(buf[i]);
}

static void printptr(uint64 x)
{
    int i;
    serial_lowlevel_putc('0');
    serial_lowlevel_putc('x');
    for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
        serial_lowlevel_putc(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

// Print to the console. only understands %d, %x, %p, %s.
void printf(char *fmt, ...)
{
    va_list ap;
    int i, c;
    char *s;

    if (fmt == 0)
    {
        serial_lowlevel_puts("null fmt");
        while (1)
            ;
    }

    va_start(ap, fmt);
    for (i = 0; (c = fmt[i] & 0xff) != 0; i++)
    {
        if (c != '%')
        {
            serial_lowlevel_putc(c);
            continue;
        }
        c = fmt[++i] & 0xff;
        if (c == 0)
            break;
        switch (c)
        {
        case 'd':
            printint(va_arg(ap, int), 10, 1);
            break;
        case 'x':
            printint(va_arg(ap, int), 16, 1);
            break;
        case 'p':
            printptr(va_arg(ap, uint64));
            break;
        case 's':
            if ((s = va_arg(ap, char *)) == 0)
                s = "(null)";
            for (; *s; s++)
                serial_lowlevel_putc(*s);
            break;
        case '%':
            serial_lowlevel_putc('%');
            break;
        default:
            // Print unknown % sequence to draw attention.
            serial_lowlevel_putc('%');
            serial_lowlevel_putc(c);
            break;
        }
    }
}
