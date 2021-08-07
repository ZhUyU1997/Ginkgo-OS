#include <syscall.h>
#include <string.h>

#include "types.h"
#include "print.h"

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
    printf("slot:%d\n", slot);
    char buf[1024] = "hello vmo\n";
    usys_vmo_write(slot, 0, (u64_t)buf, 1024);
    usys_vmo_map(0, slot, (u64_t)0x100000, VM_READ | VM_WRITE, 0);

    u64_t *addr = (u64_t *)0x100000;
    printf((const char *)addr);
}

void test_nanosleep()
{
    printf("test_nanosleep:start\n");
    usys_nanosleep(usys_deadline_after(1000000000));
    printf("test_nanosleep:end\n");
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
    test_nanosleep();
    return 0;
}