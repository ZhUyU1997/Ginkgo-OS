#include "syscall.h"

int main()
{
    while (1)
    {
        for (volatile int i = 0; i < 10000000; i++);
            putstring("2342");
    }
    return 0;
}