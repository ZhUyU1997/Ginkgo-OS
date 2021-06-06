#include <syscall.h>
#include "types.h"

typedef enum
{
    VMO_ANONYM = 0,     /* lazy allocation */
    VMO_DATA = 1,       /* immediate allocation */
    VMO_FILE = 2,       /* file backed */
    VMO_SHM = 3,        /* shared memory */
    VMO_USER_PAGER = 4, /* support user pager */
    VMO_DEVICE = 5,     /* memory mapped device registers */
} vmo_type_t;

void *memset(void *dst, int c, uint32 n)
{
    char *cdst = (char *)dst;
    int i;
    for (i = 0; i < n; i++)
    {
        cdst[i] = c;
    }
    return dst;
}

int main()
{

    usys_putstring("main");
    int slot = usys_vmo_create(1024, VMO_DATA);
    char buf[1024] = "hello vmo";
    usys_vmo_write(slot, 0, buf, 1024);
    usys_vmo_read(slot, 0, buf, 1024);
    usys_putstring(buf);

    while (1)
        ;
    return 0;
}