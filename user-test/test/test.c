#include <syscall.h>
#include "types.h"
#include "print.h"

typedef enum
{
    VMO_ANONYM = 0,     /* lazy allocation */
    VMO_DATA = 1,       /* immediate allocation */
    VMO_FILE = 2,       /* file backed */
    VMO_SHM = 3,        /* shared memory */
    VMO_USER_PAGER = 4, /* support user pager */
    VMO_DEVICE = 5,     /* memory mapped device registers */
} vmo_type_t;

#define VM_READ  (1 << 0)
#define VM_WRITE (1 << 1)
#define VM_EXEC  (1 << 2)

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

void test_vmo()
{
    int slot = usys_vmo_create(1024, VMO_DATA);
    char buf[1024] = "hello vmo\n";
    usys_vmo_write(slot, 0, (u64_t)buf, 1024);
    usys_vmo_read(slot, 0, (u64_t)buf, 1024);
    printf(buf);
}

void test_vmo_map()
{
    int slot = usys_vmo_create(1024, VMO_DATA);
    char buf[1024] = "hello vmo\n";
    usys_vmo_write(slot, 0, (u64_t)buf, 1024);
    usys_vmo_map(0, slot, (u64_t)0x100000, VM_READ | VM_WRITE, 0);

    u64_t *addr = (u64_t *)0x100000;
    printf((const char *)addr);
}

int main(int argc, char **argv)
{
    printf("%d\n",argc);
    printf("%p\n",argv);
    printf("%s\n",argv[0]);
    printf("%s\n",argv[1]);

    printf("main\n");

    test_vmo();
    test_vmo_map();
    return 0;
}