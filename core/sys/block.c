#include <types.h>
#include <block/block.h>

static block_t *user_bdev = NULL;

u64_t sys_block_read(u8_t *buf, u64_t offset, u64_t count)
{
    return block_read(user_bdev, buf, offset, count);
}

u64_t sys_block_write(u8_t *buf, u64_t offset, u64_t count)
{
    return block_write(user_bdev, buf, offset, count);
}

u64_t sys_block_capacity()
{
    return block_capacity(user_bdev);
}

u64_t sys_block_size()
{
    return block_size(user_bdev);
}

u64_t sys_block_count()
{
    return block_count(user_bdev);
}

void do_user_block_init(const char *dev)
{
    user_bdev = search_device_with_class(block_t, dev);
}