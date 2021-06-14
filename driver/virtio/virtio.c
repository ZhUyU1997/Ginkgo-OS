#include "virtio-internal.h"

#include <virtio/virtio.h>

#include <types.h>
#include <riscv.h>
#include <memlayout.h>
#include <io.h>
#include <log.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

//https://github.com/sgmarz/osblog/blob/master/risc_v/src/virtio.rs
//https://github.com/wblixin6017/zircon/tree/master/system/dev/bus/virtio
//https://github.com/yiannigiannaris/xv6-riscv-window/tree/master/kernel

struct virtio_mmio_desc_t mmio_device_data[MMIO_VIRTIO_NUM] = {0};

bool_t virtio_desc_new(struct virtio_queue_t *queue, struct vring_desc *desc, __virtio16 flags)
{
    if (flags & VRING_DESC_F_NEXT)
        desc->next = (queue->idx + 1) % VIRTIO_RING_SIZE;
    else
        desc->next = 0;

    desc->flags |= flags;

    if (queue->queue.desc[queue->idx].flags != 0)
    {
        return FALSE;
    }

    queue->queue.desc[queue->idx] = *desc;
    queue->idx = (queue->idx + 1) % VIRTIO_RING_SIZE;
    return TRUE;
}

void virtio_desc_del(struct virtio_queue_t *queue, int idx)
{
    struct vring_desc *desc = queue->queue.desc;

    while (1)
    {
        //TODO
        // dev->free_desc(dev, idx);
        desc[idx] = (struct vring_desc){};

        if (desc[idx].flags & VRING_DESC_F_NEXT)
            idx = desc[idx].next;
        else
            break;
    }
}

bool_t virtio_send_command_1(struct virtio_queue_t *queue, struct vring_desc *response)
{
    uint32 head = virtio_desc_get_index(queue);
    if (!virtio_desc_new(queue, response, VRING_DESC_F_WRITE))
        return FALSE;

    virtio_avail_new(queue, head);
    return TRUE;
}

bool_t virtio_send_command_2(struct virtio_queue_t *queue, struct vring_desc *request, struct vring_desc *response)
{
    uint32 head = virtio_desc_get_index(queue);

    if (!virtio_desc_new(queue, request, VRING_DESC_F_NEXT))
        return FALSE;
    if (!virtio_desc_new(queue, response, VRING_DESC_F_WRITE))
        return FALSE;
    virtio_avail_new(queue, head);
    return TRUE;
}

bool_t virtio_send_command_3(struct virtio_queue_t *queue, struct vring_desc *request1, struct vring_desc *request2, struct vring_desc *response)
{
    uint32 head = virtio_desc_get_index(queue);

    if (!virtio_desc_new(queue, request1, VRING_DESC_F_NEXT))
        return FALSE;
    if (!virtio_desc_new(queue, request2, VRING_DESC_F_NEXT))
        return FALSE;
    if (!virtio_desc_new(queue, response, VRING_DESC_F_WRITE))
        return FALSE;
    virtio_avail_new(queue, head);
    return TRUE;
}

void virtio_avail_new(struct virtio_queue_t *queue, int idx)
{
    vring_avail_t *avail = queue->queue.avail;

    avail->ring[avail->idx % VIRTIO_RING_SIZE] = idx;
    __sync_synchronize();
    avail->idx += 1;
    __sync_synchronize();
}

int virtio_desc_get_index(struct virtio_queue_t *queue)
{
    return queue->idx;
}

void virtio_mmio_notify(struct virtio_mmio_desc_t *desc)
{
    write32(desc->addr + VIRTIO_MMIO_QUEUE_NOTIFY, 0);
}

int virtio_mmio_setup(struct virtio_mmio_desc_t *desc, uint32 (*get_features)(uint32 features))
{
    addr_t addr = desc->addr;

    uint32 status = 0;

    // [Driver] Device Initialization
    // 1. Reset the device (write 0 into status)
    write32(addr + VIRTIO_MMIO_STATUS, 0);
    // 2. Set ACKNOWLEDGE status bit
    status |= VIRTIO_CONFIG_S_ACKNOWLEDGE;
    write32(addr + VIRTIO_MMIO_STATUS, status);
    // 3. Set the DRIVER status bit
    status |= VIRTIO_CONFIG_S_DRIVER;
    write32(addr + VIRTIO_MMIO_STATUS, status);
    // 4. Read device feature bits, write subset of feature
    // bits understood by OS and driver to the device.
    uint32 features = read32(addr + VIRTIO_MMIO_DEVICE_FEATURES);
    LOGD("feature:", $(features));
    write32(addr + VIRTIO_MMIO_DRIVER_FEATURES, get_features(features));
    // 5. Set the FEATURES_OK status bit
    status |= VIRTIO_CONFIG_S_FEATURES_OK;
    write32(addr + VIRTIO_MMIO_STATUS, status);
    // 6. Re-read status to ensure FEATURES_OK is still set.
    // Otherwise, it doesn't support our features.
    uint32 status_ok = read32(addr + VIRTIO_MMIO_STATUS);
    // If the status field no longer has features_ok set,
    // that means that the device couldn't accept
    // the features that we request. Therefore, this is
    // considered a "failed" state.
    if (!(status_ok & VIRTIO_CONFIG_S_FEATURES_OK))
    {
        LOGI("features fail...");
        write32(addr + VIRTIO_MMIO_STATUS, VIRTIO_CONFIG_S_FAILED);
        return 0;
    }

    // 8. Set the DRIVER_OK status bit. Device is now "live"
    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    write32(addr + VIRTIO_MMIO_STATUS, status);

    write32(addr + VIRTIO_MMIO_GUEST_PAGE_SIZE, PAGE_SIZE);

    return 1;
}

int virtio_mmio_queue_set(struct virtio_mmio_desc_t *desc, struct virtio_queue_t *queue, int sel)
{
    addr_t addr = desc->addr;

    // 7. Perform device-specific setup.
    // Set the queue num. We have to make sure that the
    // queue size is valid because the device can only take
    // a certain size.
    LOGI("SEL:" $(sel));

    write32(addr + VIRTIO_MMIO_QUEUE_SEL, sel);

    uint32 max = read32(addr + VIRTIO_MMIO_QUEUE_NUM_MAX);
    LOGI("QUEUE_NUM_MAX:" $(max));
    write32(addr + VIRTIO_MMIO_QUEUE_NUM, VIRTIO_RING_SIZE);

    if (VIRTIO_RING_SIZE > max)
    {
        LOGI("queue size fail...");
        write32(addr + VIRTIO_MMIO_STATUS, VIRTIO_CONFIG_S_FAILED);
        return 0;
    }

    uint32 num_pages = (vring_size(VIRTIO_RING_SIZE, PAGE_SIZE) + PAGE_SIZE - 1) >> PAGE_SHIFT;
    LOGI("num_pages:" $(num_pages));

    // QueuePFN is a physical page number, however it
    // appears for QEMU we have to write the entire memory
    // address. This is a physical memory address where we
    // (the OS) and the block device have in common for
    // making and receiving requests.

    void *queue_ptr = calloc(1, num_pages * PAGE_SIZE);
    LOGD("queue_ptr:" $(queue_ptr));
    uint32 queue_pfn = (addr_t)queue_ptr >> PAGE_SHIFT;
    write32(addr + VIRTIO_MMIO_QUEUE_PFN, queue_pfn);

    vring_init(&queue->queue, VIRTIO_RING_SIZE, queue_ptr, PAGE_SIZE);

    return 1;
}
void virtio_mmio_interrupt_ack(struct virtio_mmio_desc_t *desc)
{
    addr_t addr = desc->addr;
    write32(addr + VIRTIO_MMIO_INTERRUPT_ACK, read32(addr + VIRTIO_MMIO_INTERRUPT_STATUS) & 0x3);
}

void *virtio_mmio_get_config(struct virtio_mmio_desc_t *desc)
{
    addr_t addr = desc->addr;
    return (void *)(addr + VIRTIO_MMIO_CONFIG);
}

struct virtio_mmio_desc_t *virtio_mmio_search_device(uint32 _device_id, int virtio_mmio_bus)
{
    addr_t addr = MMIO_VIRTIO_START + virtio_mmio_bus * MMIO_VIRTIO_STRIDE;
    uint32 magic_value = read32(addr + VIRTIO_MMIO_MAGIC_VALUE);
    uint32 version = read32(addr + VIRTIO_MMIO_VERSION);
    uint32 device_id = read32(addr + VIRTIO_MMIO_DEVICE_ID);
    uint32 vendor_id = read32(addr + VIRTIO_MMIO_VENDOR_ID);

    // 0x74_72_69_76 is "virt" in little endian, so in reality
    // it is triv. All VirtIO devices have this attached to the
    // MagicValue register (offset 0x000)
    if (magic_value != 0x74726976)
    {
        LOGI("not virtio.");
    }
    else if (device_id == _device_id)
    {
        if (virtio_mmio_bus < MMIO_VIRTIO_NUM)
        {
            struct virtio_mmio_desc_t *data = &mmio_device_data[virtio_mmio_bus];
            data->virtio_mmio_bus = virtio_mmio_bus;
            data->device_id = device_id;
            data->addr = addr;
            return data;
        }
    }
    return NULL;
}

void virtio_device_irq_handler(struct virtio_queue_t *data, void (*free_desc)(struct vring_desc *desc, void *data), void *priv)
{
    assert(data && free_desc);

    volatile vring_t *queue = &data->queue;

    while (data->ack_used_idx != queue->used->idx)
    {
        int idx = queue->used->ring[data->ack_used_idx % VIRTIO_RING_SIZE].id;

        struct vring_desc *desc = data->queue.desc;

        while (1)
        {
            uint16_t flags = desc[idx].flags;
            uint16_t next = desc[idx].next;

            // In case of sending command during callback
            struct vring_desc temp = desc[idx];

            desc[idx] = (struct vring_desc){};
            free_desc(&temp, priv);

            if (!(flags & VRING_DESC_F_NEXT))
                break;
            idx = next;
        }

        __sync_synchronize();
        data->ack_used_idx += 1;
        __sync_synchronize();
    }
}