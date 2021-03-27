#pragma once

#include <types.h>
#include <virtio/virtio_ring.h>

#define VIRTIO_RING_SIZE (1 << 7)

struct virtio_device_data
{
    addr_t addr;
    vring_t queue;
    uint32 device_id;
    int idx;
    int virtio_mmio_bus;
    uint16 ack_used_idx;
};

enum
{
    MMIO_VIRTIO_START = 0x10001000UL,
    MMIO_VIRTIO_END = 0x10008000UL,
    MMIO_VIRTIO_STRIDE = 0x1000UL,
    MMIO_VIRTIO_MAGIC = 0x74726976UL,
    MMIO_VIRTIO_NUM = 8,
};