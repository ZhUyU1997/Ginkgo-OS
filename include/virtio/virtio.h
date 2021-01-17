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

void virtio_handle_interrupt(int irq);
void virtio_init();
void virtio_disk_rw(void *addr, uint64 sector, uint32 size, int write);