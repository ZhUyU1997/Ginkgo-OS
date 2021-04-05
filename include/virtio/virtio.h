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

struct virtio_device_data;

void virtio_desc_new(struct virtio_device_data *data, struct vring_desc *desc, __virtio16 flags);
void virtio_desc_del(struct virtio_device_data *data, int idx);
void virtio_send_command_2(struct virtio_device_data *data, struct vring_desc *request, struct vring_desc *response);
void virtio_send_command_3(struct virtio_device_data *data, struct vring_desc *request1, struct vring_desc *request2, struct vring_desc *response);
void virtio_avail_new(struct virtio_device_data *data, int idx);
int virtio_device_setup(struct virtio_device_data *data, uint32 (*get_features)(uint32 features));
struct virtio_device_data *virtio_mmio_search_device(uint32 device_id, int virtio_mmio_bus);
void virtio_device_interrupt_ack(struct virtio_device_data *data);
int virtio_desc_get_index(struct virtio_device_data *data);
void virtio_mmio_notify(struct virtio_device_data *data);
void virtio_device_irq_handler(struct virtio_device_data *data, void (*free_desc)(struct vring_desc *desc, void *data), void *priv);
void *virtio_device_get_configuration_layout(struct virtio_device_data *data);

#define VRING_DESC(data) \
    &(struct vring_desc) { .addr = (uint64)data, .len = sizeof(*data), }
#define VRING_DESC_LEN(data, _len) \
    &(struct vring_desc) { .addr = (uint64)data, .len = _len, }
#define VRING_DESC_LEN_FLAG(data, _len, _flags) \
    &(struct vring_desc) { .addr = (uint64)data, .len = _len, .flags = _flags }
