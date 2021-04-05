#include <virtio/virtio.h>

#include <types.h>
#include <riscv.h>
#include <memlayout.h>
#include <io.h>
#include <log.h>
#include <string.h>
#include <malloc.h>
#include <core/class.h>
#include <block/block.h>
#include <interrupt/interrupt.h>
#include <core/semaphore.h>
#include <core/waitqueue.h>

class(virtio_block_t, block_t)
{
    struct virtio_device_data *data;
    struct wait_queue_head wq_head;
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

    uint8 status = 1;
    virtio_send_command_3(data, VRING_DESC(buf0), VRING_DESC_LEN_FLAG(buf, blkcnt * block_size(blk), VRING_DESC_F_WRITE), VRING_DESC(&status));
    virtio_mmio_notify(data);
    wait_event(&vblk->wq_head, (!status));
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

    uint8 status = 1;
    virtio_send_command_3(data, VRING_DESC(buf0), VRING_DESC_LEN(buf, blkcnt * block_size(blk)), VRING_DESC(&status));
    virtio_mmio_notify(data);
    wait_event(&vblk->wq_head, (!status));
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

static void free_desc(struct vring_desc *desc, virtio_block_t *vblk)
{
    if (desc->flags == VRING_DESC_F_WRITE)
    {
        u8_t *status = desc->addr;

        if (*status == 0)
        {
            wake_up(&vblk->wq_head);
        }
    }
    else
    {
        if (desc->len == sizeof(struct virtio_blk_outhdr))
        {
            free(desc->addr);
        }
    }
}

static void irq_handler(virtio_block_t *vblk)
{
    struct virtio_device_data *data = vblk->data;
    virtio_device_interrupt_ack(data);
    virtio_device_irq_handler(data, free_desc, vblk);
}

int virtio_block_probe(device_t *this, xjil_value_t *value)
{
    virtio_block_t *vblk = dynamic_cast(virtio_block_t)(this);
    uint32 device_id = xjil_read_int(value, "device_id", -1);
    int virtio_mmio_bus = xjil_read_int(value, "virtio-mmio-bus", -1);
    int irq = xjil_read_int(value, "interrupt", -1);

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

    struct virtio_blk_config *config = (struct virtio_blk_config *)virtio_device_get_configuration_layout(vblk->data);
    LOGD("capacity:" $(config->capacity));
    LOGD("blk_size:" $(config->blk_size));

    dynamic_cast(block_t)(this)->blksz = config->blk_size;
    dynamic_cast(block_t)(this)->blkcnt = config->capacity;

    init_waitqueue_head(&vblk->wq_head);
    request_irq(irq, irq_handler, vblk);

    return 0;
}

constructor(virtio_block_t)
{
    dynamic_cast(device_t)(this)->name = "virtio-block";
    dynamic_cast(device_t)(this)->probe = virtio_block_probe;

    dynamic_cast(block_t)(this)->write = virtio_block_write;
    dynamic_cast(block_t)(this)->read = virtio_block_read;
    dynamic_cast(block_t)(this)->sync = virtio_block_sync;
}