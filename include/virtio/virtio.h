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
#include <virtio/virtio_input.h>

#include <core/class.h>
#include <core/device.h>

struct virtio_queue_t;
struct virtio_mmio_desc_t;

bool_t virtio_desc_new(struct virtio_queue_t *queue, struct vring_desc *desc, __virtio16 flags);
bool_t virtio_send_command_1(struct virtio_queue_t *queue, struct vring_desc *response);
bool_t virtio_send_command_2(struct virtio_queue_t *queue, struct vring_desc *request, struct vring_desc *response);
bool_t virtio_send_command_3(struct virtio_queue_t *queue, struct vring_desc *request1, struct vring_desc *request2, struct vring_desc *response);
void virtio_avail_new(struct virtio_queue_t *queue, int idx);
int virtio_desc_get_index(struct virtio_queue_t *queue);
void virtio_device_irq_handler(struct virtio_queue_t *queue, void (*free_desc)(struct vring_desc *desc, void *data), void *priv);
void virtio_desc_del(struct virtio_queue_t *queue, int idx);

struct virtio_mmio_desc_t *virtio_mmio_search_device(uint32 device_id, int virtio_mmio_bus);
int virtio_mmio_setup(struct virtio_mmio_desc_t *desc, uint32 (*get_features)(uint32 features));
int virtio_mmio_queue_set(struct virtio_mmio_desc_t *desc, struct virtio_queue_t *queue, int sel);
void *virtio_mmio_get_config(struct virtio_mmio_desc_t *desc);
void virtio_mmio_notify(struct virtio_mmio_desc_t *desc);
void virtio_mmio_interrupt_ack(struct virtio_mmio_desc_t *desc);

#define VRING_DESC(data) \
    &(struct vring_desc) { .addr = (uint64)data, .len = sizeof(*data), }
#define VRING_DESC_LEN(data, _len) \
    &(struct vring_desc) { .addr = (uint64)data, .len = _len, }
#define VRING_DESC_LEN_FLAG(data, _len, _flags) \
    &(struct vring_desc) { .addr = (uint64)data, .len = _len, .flags = _flags }
