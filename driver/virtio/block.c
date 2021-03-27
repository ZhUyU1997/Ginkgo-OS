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
    struct virtio_device_data *data;
};

class_impl(virtio_block_t, block_t){};

u64_t virtio_block_read(block_t *blk, u8_t *buf, u64_t blkno, u64_t blkcnt)
{
    virtio_block_t *vblk = dynamic_cast(virtio_block_t)(blk);
    struct virtio_device_data *data = vblk->data;
    struct virtio_blk_outhdr *buf0 = calloc(1, sizeof(struct virtio_blk_outhdr));

    buf0->type = VIRTIO_BLK_T_IN;
    buf0->ioprio = 0;
    buf0->sector = blkno;

    uint32 head = virtio_desc_get_index(data);
    uint8 *status = malloc(sizeof(uint8));

    virtio_desc_new3(data, VRING_DESC(buf0), VRING_DESC_LEN_FLAG(buf, blkcnt * block_size(blk), VRING_DESC_F_WRITE), VRING_DESC(status));
    virtio_avail_new(data, head);

    virtio_mmio_notify(data);
    while (*status == 0xff)
        ;
    return blkcnt;
}

u64_t virtio_block_write(block_t *blk, u8_t *buf, u64_t blkno, u64_t blkcnt)
{
    virtio_block_t *vblk = dynamic_cast(virtio_block_t)(blk);
    struct virtio_device_data *data = vblk->data;
    struct virtio_blk_outhdr *buf0 = calloc(1, sizeof(struct virtio_blk_outhdr));

    buf0->type = VIRTIO_BLK_T_OUT;
    buf0->ioprio = 0;
    buf0->sector = blkno;

    uint32 head = virtio_desc_get_index(data);
    uint8 *status = malloc(sizeof(uint8));

    virtio_desc_new3(data, VRING_DESC(buf0), VRING_DESC_LEN(buf, blkcnt * block_size(blk)), VRING_DESC(status));
    virtio_avail_new(data, head);

    virtio_mmio_notify(data);
    while (*status == 0xff)
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
    uint32 device_id = xjil_read_int(value, "device_id", -1);
    int virtio_mmio_bus = xjil_read_int(value, "virtio-mmio-bus", -1);

    if (device_id == -1)
    {
        LOGE("device_id == -1");
        return -1;
    }

    struct virtio_device_data *data = virtio_mmio_search_device(device_id, virtio_mmio_bus);
    
    if (!data)
    {
        return -1;
    }

    vblk->data = data;
    virtio_device_setup(vblk->data, virtio_block_get_features);
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