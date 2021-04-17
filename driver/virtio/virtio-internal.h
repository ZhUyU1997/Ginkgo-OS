#pragma once

#include <types.h>
#include <virtio/virtio_ring.h>

#define VIRTIO_RING_SIZE (1 << 7)

struct virtio_mmio_desc_t
{
    addr_t addr;
    uint32 device_id;
    int virtio_mmio_bus;
};

struct virtio_queue_t
{
    vring_t queue;
    int idx;
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