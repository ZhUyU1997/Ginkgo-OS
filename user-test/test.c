#include "syscall.h"

int main()
{

    putstring("create");
    unsigned int *addr = create();

    for (int i = 2; i < 640 * 480; i++)
    {
        addr[i] = i;
    }

    for (int i = 2; i < 640 * 480; i++)
    {
        addr[i] = 0xff000000 | (addr[i] + addr[i - 1]);
    }

    present();
    putstring("present");
    while (1)
        ;
    return 0;
}