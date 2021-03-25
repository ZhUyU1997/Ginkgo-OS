#include <virtio/virtio.h>

#include <types.h>
#include <riscv.h>
#include <memlayout.h>
#include <io.h>
#include <log.h>
#include <string.h>
#include <malloc.h>
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

class_impl(block_t, device_t){};

static inline u64_t block_size(block_t *blk)
{
    return (blk->blksz);
}

class(virtio_block_t, block_t)
{
    struct virtio_device_data dev;
    uint8 status[VIRTIO_RING_SIZE];
};

class_impl(virtio_block_t, block_t){};

u64_t virtio_block_read(block_t *blk, u8_t *buf, u64_t blkno, u64_t blkcnt)
{
    virtio_block_t *vblk = dynamic_cast(virtio_block_t)(blk);
    struct virtio_device_data *dev = &vblk->dev;
    struct virtio_blk_outhdr *buf0 = calloc(1, sizeof(struct virtio_blk_outhdr));

    buf0->type = VIRTIO_BLK_T_IN;
    buf0->ioprio = 0;
    buf0->sector = blkno;

    uint32 head = dev->idx;

    vblk->status[head] = 0;

    struct vring_desc descs[3] = {
        {
            .addr = (uint64)buf0,
            .len = sizeof(struct virtio_blk_outhdr),
        },
        {
            .addr = (uint64)buf,
            .len = blkcnt * block_size(blk),
            .flags = VRING_DESC_F_WRITE,
        },
        {
            .addr = (uint64)&vblk->status[head],
            .len = sizeof(uint8),
        },
    };

    virtio_desc_new3(dev, descs + 0, descs + 1, descs + 1);
    virtio_avail_new(dev, head);

    write32(dev->addr + VIRTIO_MMIO_QUEUE_NOTIFY, 0);
    while (vblk->status[head] == 0xff)
        ;
    return blkcnt;
}

u64_t virtio_block_write(block_t *blk, u8_t *buf, u64_t blkno, u64_t blkcnt)
{
    virtio_block_t *vblk = dynamic_cast(virtio_block_t)(blk);
    struct virtio_device_data *dev = &vblk->dev;
    struct virtio_blk_outhdr *buf0 = calloc(1, sizeof(struct virtio_blk_outhdr));

    buf0->type = VIRTIO_BLK_T_OUT;
    buf0->ioprio = 0;
    buf0->sector = blkno;

    uint32 head = dev->idx;

    vblk->status[head] = 0;

    struct vring_desc descs[3] = {
        {
            .addr = (uint64)buf0,
            .len = sizeof(struct virtio_blk_outhdr),
        },
        {
            .addr = (uint64)buf,
            .len = blkcnt * block_size(blk),
        },
        {
            .addr = (uint64)&vblk->status[head],
            .len = sizeof(uint8),
        },
    };

    virtio_desc_new3(dev, descs + 0, descs + 1, descs + 1);
    virtio_avail_new(dev, head);

    write32(dev->addr + VIRTIO_MMIO_QUEUE_NOTIFY, 0);
    while (vblk->status[head] == 0xff)
        ;
    return blkcnt;
}

void virtio_block_sync(block_t *blk)
{
}

static uint32 virtio_block_get_features(uint32 features)
{
    features &= ~(1 << VIRTIO_BLK_F_RO);
    features &= ~(1 << VIRTIO_BLK_F_SCSI);
    features &= ~(1 << VIRTIO_BLK_F_CONFIG_WCE);
    features &= ~(1 << VIRTIO_BLK_F_MQ);
    features &= ~(1 << VIRTIO_F_ANY_LAYOUT);
    features &= ~(1 << VIRTIO_RING_F_EVENT_IDX);
    features &= ~(1 << VIRTIO_RING_F_INDIRECT_DESC);
    return features;
}

int virtio_block_probe(device_t *this, xjil_value_t *value)
{
    virtio_block_t *vblk = dynamic_cast(virtio_block_t)(this);
    vblk->dev.device_id = xjil_read_int(value, "device_id", -1);
    vblk->dev.virtio_mmio_bus = xjil_read_int(value, "virtio-mmio-bus", -1);

    if (vblk->dev.device_id == -1)
    {
        LOGE("device_id == -1");
        return -1;
    }

    if (virtio_mmio_search_device(&vblk->dev))
    {
        return -1;
    }

    virtio_device_setup(&vblk->dev, virtio_block_get_features);
    return 0;
}

constructor(virtio_block_t)
{
    dynamic_cast(device_t)(this)->name = "virtio-block";
    dynamic_cast(device_t)(this)->probe = virtio_block_probe;

    dynamic_cast(block_t)(this)->write = virtio_block_write;
    dynamic_cast(block_t)(this)->read = virtio_block_read;
    dynamic_cast(block_t)(this)->sync = virtio_block_sync;
    dynamic_cast(block_t)(this)->blksz = 512;
}

register_driver(virtio_block_t);