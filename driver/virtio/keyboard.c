#include "virtio-internal.h"

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
#include <core/events.h>

class(virtio_keyboard_t, device_t)
{
    struct virtio_mmio_desc_t *desc;
    struct virtio_queue_t eventq;
    struct virtio_queue_t statusq;
    struct wait_queue_head wq_head;
};

class_impl(virtio_keyboard_t, device_t){};

void virtio_keyboard_supports_key(struct virtio_mmio_desc_t *desc)
{
    volatile struct virtio_input_config *config = (struct virtio_input_config *)virtio_mmio_get_config(desc);
    config->select = VIRTIO_INPUT_CFG_EV_BITS;
    config->subsel = EV_KEY;
}

void virtio_keyboard_request_event(struct virtio_mmio_desc_t *desc, struct virtio_queue_t *data)
{
    while (1)
    {
        struct virtio_input_event *event = calloc(1, sizeof(struct virtio_input_event));
        if (!virtio_send_command_1(data, VRING_DESC(event)))
            return;
        virtio_mmio_notify(desc);
    }
}

static uint32 virtio_block_get_features(uint32 features)
{
    return 0;
}

void virtio_keyboard_handle_syn_event(uint16 code, uint32 value)
{
    switch (code)
    {
    case SYN_REPORT:
        LOGI("report");
        break;
    case SYN_CONFIG:
        LOGI("config");
        break;
    case SYN_MT_REPORT:
        LOGI("mt report");
        break;
    case SYN_DROPPED:
        LOGI("dropped");
        break;
    default:
        LOGI("code " $(code) " not recognized");
    }
}

void virtio_keyboard_handle_key_event(uint16 code, uint32 value)
{
    LOGI("code:" $(code) " value:" $((u64_t)value));
}

void virtio_keyboard_handle_event(struct virtio_input_event *b)
{
    switch (b->type)
    {
    case EV_SYN:
        virtio_keyboard_handle_syn_event(b->code, b->value);
        break;
    case EV_KEY:
        virtio_keyboard_handle_key_event(b->code, b->value);
        break;
    default:
        LOGI("EV TYPE " $(b->type) " NOT RECOGNIZED");
    }
}

static void free_desc(struct vring_desc *desc, void *data)
{
    virtio_keyboard_t *vkb = (virtio_keyboard_t *)data;
    if (desc->flags == VRING_DESC_F_WRITE)
    {
        struct virtio_input_event *event = (struct virtio_input_event *)desc->addr;
        virtio_keyboard_handle_event(event);
        free(event);
    }
    else
    {
        PANIC("");
    }
}

static void irq_handler(void *data)
{
    LOGI();
    virtio_keyboard_t *vkb = (virtio_keyboard_t *)data;
    struct virtio_queue_t *eventq = &vkb->eventq;
    virtio_mmio_interrupt_ack(vkb->desc);
    virtio_device_irq_handler(eventq, free_desc, vkb);
    virtio_keyboard_request_event(vkb->desc, eventq);
}

int virtio_keyboard_probe(device_t *this, xjil_value_t *value)
{
    virtio_keyboard_t *vkb = dynamic_cast(virtio_keyboard_t)(this);
    uint32 device_id = xjil_read_int(value, "device_id", -1);
    int virtio_mmio_bus = xjil_read_int(value, "virtio-mmio-bus", -1);
    int irq = xjil_read_int(value, "interrupt", -1);

    if (device_id == -1)
    {
        LOGE("device_id == -1");
        return -1;
    }

    struct virtio_mmio_desc_t *desc = virtio_mmio_search_device(device_id, virtio_mmio_bus);

    if (!desc)
    {
        return -1;
    }

    vkb->desc = desc;
    virtio_mmio_setup(desc, virtio_block_get_features);
    virtio_mmio_queue_set(desc, &vkb->eventq, 0);
    virtio_mmio_queue_set(desc, &vkb->statusq, 1);

    struct virtio_queue_t *data = &vkb->eventq;
    virtio_keyboard_supports_key(desc);
    virtio_keyboard_request_event(vkb->desc, data);

    init_waitqueue_head(&vkb->wq_head);
    request_irq(irq, irq_handler, vkb);
    return 0;
}

constructor(virtio_keyboard_t)
{
    dynamic_cast(device_t)(this)->name = "virtio-keyboard";
    dynamic_cast(device_t)(this)->probe = virtio_keyboard_probe;
}