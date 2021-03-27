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

struct virtio_device_data mmio_device_data[MMIO_VIRTIO_NUM] = {0};

void virtio_desc_new(struct virtio_device_data *data, struct vring_desc *desc, __virtio16 flags)
{
    if (flags == VRING_DESC_F_NEXT)
        desc->next = (data->idx + 1) % VIRTIO_RING_SIZE;
    else
        desc->next = 0;

    desc->flags |= flags;
    data->queue.desc[data->idx] = *desc;
    data->idx = (data->idx + 1) % VIRTIO_RING_SIZE;
}

void virtio_desc_del(struct virtio_device_data *data, int idx)
{
    struct vring_desc *desc = data->queue.desc;

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

void virtio_send_command_2(struct virtio_device_data *data, struct vring_desc *request, struct vring_desc *response)
{
    uint32 head = virtio_desc_get_index(data);

    virtio_desc_new(data, request, VRING_DESC_F_NEXT);
    virtio_desc_new(data, response, VRING_DESC_F_WRITE);
    virtio_avail_new(data, head);
}

void virtio_send_command_3(struct virtio_device_data *data, struct vring_desc *request1, struct vring_desc *request2, struct vring_desc *response)
{
    uint32 head = virtio_desc_get_index(data);

    virtio_desc_new(data, request1, VRING_DESC_F_NEXT);
    virtio_desc_new(data, request2, VRING_DESC_F_NEXT);
    virtio_desc_new(data, response, VRING_DESC_F_WRITE);
    virtio_avail_new(data, head);
}

void virtio_avail_new(struct virtio_device_data *data, int idx)
{
    vring_avail_t *avail = data->queue.avail;

    avail->ring[avail->idx % VIRTIO_RING_SIZE] = idx;
    __sync_synchronize();
    avail->idx += 1;
    __sync_synchronize();
}

int virtio_desc_get_index(struct virtio_device_data *data)
{
    return data->idx;
}

void virtio_mmio_notify(struct virtio_device_data *data)
{
    write32(data->addr + VIRTIO_MMIO_QUEUE_NOTIFY, 0);
}

int virtio_device_setup(struct virtio_device_data *data, uint32 (*get_features)(uint32 features))
{
    addr_t addr = data->addr;

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

    // 7. Perform device-specific setup.
    // Set the queue num. We have to make sure that the
    // queue size is valid because the device can only take
    // a certain size.
    uint32 queue_num_max = read32(addr + VIRTIO_MMIO_QUEUE_NUM_MAX);
    LOGI("QUEUE_NUM_MAX:" $(queue_num_max));
    write32(addr + VIRTIO_MMIO_QUEUE_NUM, VIRTIO_RING_SIZE);

    if (VIRTIO_RING_SIZE > queue_num_max)
    {
        LOGI("queue size fail...");
        write32(addr + VIRTIO_MMIO_STATUS, VIRTIO_CONFIG_S_FAILED);
        return 0;
    }
    // First, if the block device array is empty, create it!
    // We add 4095 to round this up and then do an integer
    // divide to truncate the decimal. We don't add 4096,
    // because if it is exactly 4096 bytes, we would get two
    // pages, not one.

    uint32 num_pages = (vring_size(VIRTIO_RING_SIZE, PAGE_SIZE) + PAGE_SIZE - 1) >> PAGE_SHIFT;
    LOGI("num_pages:" $(num_pages));
    // We allocate a page for each device. This will the the
    // descriptor where we can communicate with the block
    // device. We will still use an MMIO register (in
    // particular, QueueNotify) to actually tell the device
    // we put something in memory. We also have to be
    // careful with memory ordering. We don't want to
    // issue a notify before all memory writes have
    // finished. We will look at that later, but we need
    // what is called a memory "fence" or barrier.
    write32(addr + VIRTIO_MMIO_QUEUE_SEL, 0);
    // TODO: Set up queue #1 (cursorq)

    // Alignment is very important here. This is the memory address
    // alignment between the available and used rings. If this is wrong,
    // then we and the device will refer to different memory addresses
    // and hence get the wrong data in the used ring.
    // ptr.add(MmioOffsets::QueueAlign.scale32()).write_volatile(2);
    write32(addr + VIRTIO_MMIO_GUEST_PAGE_SIZE, PAGE_SIZE);

    // QueuePFN is a physical page number, however it
    // appears for QEMU we have to write the entire memory
    // address. This is a physical memory address where we
    // (the OS) and the block device have in common for
    // making and receiving requests.

    void *queue_ptr = calloc(1, num_pages * PAGE_SIZE);
    LOGD("queue_ptr:" $(queue_ptr));
    uint32 queue_pfn = (addr_t)queue_ptr >> PAGE_SHIFT;
    write32(addr + VIRTIO_MMIO_QUEUE_PFN, queue_pfn);

    // 8. Set the DRIVER_OK status bit. Device is now "live"
    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    write32(addr + VIRTIO_MMIO_STATUS, status);
    vring_init(&data->queue, VIRTIO_RING_SIZE, queue_ptr, PAGE_SIZE);
    return 1;
}

void virtio_device_interrupt_ack(struct virtio_device_data *data)
{
    addr_t addr = MMIO_VIRTIO_START + MMIO_VIRTIO_STRIDE * data->virtio_mmio_bus;
    write32(addr + VIRTIO_MMIO_INTERRUPT_ACK, read32(addr + VIRTIO_MMIO_INTERRUPT_STATUS) & 0x3);
}

struct virtio_device_data *virtio_mmio_search_device(uint32 _device_id, int virtio_mmio_bus)
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
            struct virtio_device_data *data = &mmio_device_data[virtio_mmio_bus];
            data->virtio_mmio_bus = virtio_mmio_bus;
            data->device_id = device_id;
            data->addr = addr;
            return data;
        }
    }
    return NULL;
}

void virtio_device_irq_handler(struct virtio_device_data *data, void (*free_desc)(struct vring_desc *desc))
{
    assert(data && free_desc);
    virtio_device_interrupt_ack(data);

    vring_t *queue = &data->queue;
    while (data->ack_used_idx != queue->used->idx)
    {
        int idx = queue->used->ring[data->ack_used_idx % VIRTIO_RING_SIZE].id;

        struct vring_desc *desc = data->queue.desc;

        while (1)
        {
            uint16_t flags = desc[idx].flags;
            free_desc(desc + idx);
            desc[idx] = (struct vring_desc){};

            if (flags & VRING_DESC_F_NEXT)
                idx = desc[idx].next;
            else
                break;
        }

        __sync_synchronize();
        data->ack_used_idx += 1;
        __sync_synchronize();
    }
}