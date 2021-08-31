#include <stddef.h>
#include <vfs.h>
#include <syscall.h>
#include <types.h>
#include <stdio.h>
#include <ipc.h>

u64_t fs_dispatch(struct fs_request *fr);

static void ipc_handler(ipc_msg_t *ipc_msg)
{
    printf("ipc_msg %x\n", ipc_msg);

    struct fs_request *fr = ipc_get_msg_data(ipc_msg);
    s64_t ret = fs_dispatch(fr);
    ipc_return(ret);
}

u64_t block_read(struct block_t *blk, u8_t *buf, u64_t offset, u64_t count)
{
    return usys_block_read(buf, offset, count);
}
u64_t block_write(struct block_t *blk, u8_t *buf, u64_t offset, u64_t count)
{
    return usys_block_write(buf, offset, count);
}

void block_sync(struct block_t *blk)
{
}

extern struct filesystem_t filesystem_cpio;

struct block_t block_dev = {
    .name = "virtio",
};

static int exit_flag = 0;
int main(int argc, char **argv)
{
    block_dev.blksz = usys_block_size();
    block_dev.blkcnt = usys_block_count();

    vfs_mount(&block_dev, &filesystem_cpio, "/", MOUNT_RO);

    ipc_register_named_server("vfs", ipc_handler);

    while (exit_flag != 1)
        usys_yield();

    return 0;
}