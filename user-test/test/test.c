#include <syscall.h>
#include <string.h>
#include <vfs.h>
#include "ipc.h"

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
    usys_nanosleep(usys_deadline_after(100000000)); //100ms
    printf("test_nanosleep:end\n");
}

void test_futex()
{
    //TODO: write UT
    int i = 1;
    printf("test_futex:start\n");
    // usys_futex_wait(&i, 1);
    // usys_futex_wake(&i);
    printf("test_futex:end\n");
}

void test_vfs()
{
    do_vfs_init();
    printf("test vfs\n");
    int fd = vfs_open("/test.txt", O_RDONLY, S_IRWXU);
    printf("fd:%d\n", fd);

    char buf[50] = {};
    size_t read_size = vfs_read(fd, buf, 50);
    printf("read_size:%d buf:%s\n", read_size, buf);
    printf("test vfs end\n");
}

int main(int argc, char **argv)
{
    printf("argc %d\n", argc);
    printf("argv %p\n", argv);
    printf("argv[0] %s\n", argv[0]);
    printf("argv[1] %s\n", argv[1]);

    printf("main\n");

    test_vmo();
    test_vmo_map();
    test_nanosleep();
    test_futex();
    test_vfs();
    return 0;
}