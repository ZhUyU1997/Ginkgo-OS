#include <block/block.h>
#include <string.h>
#include <malloc.h>
#include <log.h>

class_impl(block_t, device_t){};

u64_t block_read(block_t *blk, u8_t *buf, u64_t offset, u64_t count)
{
    u64_t blkno, blksz, blkcnt, capacity;
    u64_t len, tmp;
    u64_t ret = 0;
    u8_t *p;

    if (!blk || !buf || !count)
        return 0;

    blksz = block_size(blk);
    blkcnt = block_count(blk);
    if (!blksz || !blkcnt)
        return 0;

    capacity = block_capacity(blk);
    if (offset >= capacity)
        return 0;

    tmp = capacity - offset;
    if (count > tmp)
        count = tmp;

    p = malloc(blksz);
    if (!p)
        return 0;

    blkno = offset / blksz;
    tmp = offset % blksz;
    if (tmp > 0)
    {
        len = blksz - tmp;
        if (count < len)
            len = count;

        if (blk->read(blk, p, blkno, 1) != 1)
        {
            free(p);
            return ret;
        }

        memcpy((void *)buf, (const void *)(&p[tmp]), len);
        buf += len;
        count -= len;
        ret += len;
        blkno += 1;
    }

    tmp = count / blksz;
    if (tmp > 0)
    {
        len = tmp * blksz;

        if (blk->read(blk, buf, blkno, tmp) != tmp)
        {
            free(p);
            return ret;
        }

        buf += len;
        count -= len;
        ret += len;
        blkno += tmp;
    }

    if (count > 0)
    {
        len = count;

        if (blk->read(blk, p, blkno, 1) != 1)
        {
            free(p);
            return ret;
        }

        memcpy((void *)buf, (const void *)(&p[0]), len);
        ret += len;
    }

    free(p);
    return ret;
}

u64_t block_write(block_t *blk, u8_t *buf, u64_t offset, u64_t count)
{
    u64_t blkno, blksz, blkcnt, capacity;
    u64_t len, tmp;
    u64_t ret = 0;
    u8_t *p;

    if (!blk || !buf || !count)
        return 0;

    blksz = block_size(blk);
    blkcnt = block_count(blk);
    if (!blksz || !blkcnt)
        return 0;

    capacity = block_capacity(blk);
    if (offset >= capacity)
        return 0;

    tmp = capacity - offset;
    if (count > tmp)
        count = tmp;

    p = malloc(blksz);
    if (!p)
        return 0;

    blkno = offset / blksz;
    tmp = offset % blksz;
    if (tmp > 0)
    {
        len = blksz - tmp;
        if (count < len)
            len = count;

        if (blk->read(blk, p, blkno, 1) != 1)
        {
            free(p);
            return ret;
        }

        memcpy((void *)(&p[tmp]), (const void *)buf, len);

        if (blk->write(blk, p, blkno, 1) != 1)
        {
            free(p);
            return ret;
        }

        buf += len;
        count -= len;
        ret += len;
        blkno += 1;
    }

    tmp = count / blksz;
    if (tmp > 0)
    {
        len = tmp * blksz;

        if (blk->write(blk, buf, blkno, tmp) != tmp)
        {
            free(p);
            return ret;
        }

        buf += len;
        count -= len;
        ret += len;
        blkno += tmp;
    }

    if (count > 0)
    {
        len = count;

        if (blk->read(blk, p, blkno, 1) != 1)
        {
            free(p);
            return ret;
        }

        memcpy((void *)(&p[0]), (const void *)buf, len);

        if (blk->write(blk, p, blkno, 1) != 1)
        {
            free(p);
            return ret;
        }

        ret += len;
    }

    free(p);
    return ret;
}

void block_sync(block_t *blk)
{
    if (blk && blk->sync)
        blk->sync(blk);
}