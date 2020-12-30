#include "types.h"

#include <stdarg.h>

extern void print(const char *buf);
extern void print_char(u8_t c);

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
        print_char(buf[i]);
}

static void printptr(uint64 x)
{
    int i;
    print_char('0');
    print_char('x');
    for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
        print_char(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

// Print to the console. only understands %d, %x, %p, %s.
void printf(char *fmt, ...)
{
    va_list ap;
    int i, c;
    char *s;

    if (fmt == 0)
    {
        print("null fmt");
        while (1)
            ;
    }

    va_start(ap, fmt);
    for (i = 0; (c = fmt[i] & 0xff) != 0; i++)
    {
        if (c != '%')
        {
            print_char(c);
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
                print_char(*s);
            break;
        case '%':
            print_char('%');
            break;
        default:
            // Print unknown % sequence to draw attention.
            print_char('%');
            print_char(c);
            break;
        }
    }
}
