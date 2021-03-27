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