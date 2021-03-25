/*
Refer to:
the virtio spec:
    https://docs.oasis-open.org/virtio/virtio/v1.1/virtio-v1.1.pdf
Linux:
    /usr/include/linux/virtio*.h
Qemu:
    /include/standard-headers/linux/virtio*.h
 */

#pragma once

#include <types.h>

#include <virtio/virtio_types.h>
#include <virtio/virtio_config.h>
#include <virtio/virtio_ids.h>
#include <virtio/virtio_ring.h>
#include <virtio/virtio_mmio.h>
#include <virtio/virtio_blk.h>
#include <virtio/virtio_gpu.h>

#include <core/class.h>
#include <core/device.h>

#define VIRTIO_RING_SIZE (1 << 7)

typedef void (*virtio_device_handle)(device_t *dev, int idx);

struct virtio_device_data
{
    addr_t addr;
    vring_t queue;
    uint32 device_id;
    int idx;
    int virtio_mmio_bus;
    uint16 ack_used_idx;
};

void virtio_desc_new(struct virtio_device_data *data, struct vring_desc *desc, __virtio16 flags);
void virtio_desc_del(struct virtio_device_data *data, int idx);
void virtio_desc_new2(struct virtio_device_data *data, struct vring_desc *request, struct vring_desc *response);
void virtio_desc_new3(struct virtio_device_data *data, struct vring_desc *request1, struct vring_desc *request2, struct vring_desc *response);
void virtio_avail_new(struct virtio_device_data *data, int idx);
int virtio_device_setup(struct virtio_device_data *data, uint32 (*get_features)(uint32 features));
int virtio_mmio_search_device(struct virtio_device_data *data);
void virtio_device_interrupt_ack(struct virtio_device_data *data);

enum
{
    MMIO_VIRTIO_START = 0x10001000UL,
    MMIO_VIRTIO_END = 0x10008000UL,
    MMIO_VIRTIO_STRIDE = 0x1000UL,
    MMIO_VIRTIO_MAGIC = 0x74726976UL,
    MMIO_VIRTIO_NUM = 8,
};

class(virtio_mmio_t, device_t)
{
    device_t *dev[MMIO_VIRTIO_NUM];
    virtio_device_handle handle[MMIO_VIRTIO_NUM];
};