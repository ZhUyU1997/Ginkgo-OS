#pragma once

#include <list.h>
#include <types.h>
#include <core/device.h>
#include <core/class.h>

class(block_t, device_t)
{
    /* The size of block */
    u64_t blksz;

    /* The total count of block */
    u64_t blkcnt;

    /* Read block device, return the block counts of reading */
    u64_t (*read)(block_t * blk, u8_t * buf, u64_t blkno, u64_t blkcnt);

    /* Write block device, return the block counts of writing */
    u64_t (*write)(block_t * blk, u8_t * buf, u64_t blkno, u64_t blkcnt);

    /* Sync cache to block device */
    void (*sync)(block_t * blk);
};

static inline u64_t block_size(block_t *blk)
{
    return (blk->blksz);
}

static inline u64_t block_count(block_t *blk)
{
    return (blk->blkcnt);
}

static inline u64_t block_capacity(block_t *blk)
{
    return (blk->blksz * blk->blkcnt);
}

static inline u64_t block_offset(block_t *blk, u64_t blkno)
{
    return (blk->blksz * blkno);
}

static inline u64_t block_available_count(block_t *blk, u64_t blkno, u64_t blkcnt)
{
    u64_t count = 0;

    if (blk->blkcnt > blkno)
    {
        count = blk->blkcnt - blkno;
        if (count > blkcnt)
            count = blkcnt;
    }
    return count;
}

static inline u64_t block_available_length(block_t *blk, u64_t blkno, u64_t blkcnt)
{
    return (block_size(blk) * block_available_count(blk, blkno, blkcnt));
}

u64_t block_read(block_t *blk, u8_t *buf, u64_t offset, u64_t count);
u64_t block_write(block_t *blk, u8_t *buf, u64_t offset, u64_t count);
void block_sync(block_t *blk);