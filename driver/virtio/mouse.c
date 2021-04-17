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

#define NUM_MOUSE_KEY_SUPPORTED 3
#define NUM_MOUSE_REL_SUPPORTED 2
static uint mouse_key_codes[] = {BTN_LEFT, BTN_RIGHT, BTN_MIDDLE};
static uint mouse_rel_codes[] = {REL_X, REL_Y, REL_WHEEL};

class(virtio_mouse_t, device_t)
{
    struct virtio_mmio_desc_t *desc;
    struct virtio_queue_t eventq;
    struct virtio_queue_t statusq;
    struct wait_queue_head wq_head;
};

class_impl(virtio_mouse_t, device_t){};

void virtio_mouse_supports_key(struct virtio_mmio_desc_t *desc)
{
    volatile struct virtio_input_config *config = (struct virtio_input_config *)virtio_mmio_get_config(desc);
    config->select = VIRTIO_INPUT_CFG_EV_BITS;
    config->subsel = EV_KEY;

    uint key_code;

    for (int i = 0; i < NUM_MOUSE_KEY_SUPPORTED; i++)
    {
        key_code = mouse_key_codes[i];
        if (!(config->u.bitmap[key_code / 8] & (1 << key_code % 8)))
            PANIC("virtio mouse key code not supported");
    }
}

void virtio_mouse_supports_rel(struct virtio_mmio_desc_t *desc)
{
    volatile struct virtio_input_config *config = (struct virtio_input_config *)virtio_mmio_get_config(desc);
    config->select = VIRTIO_INPUT_CFG_EV_BITS;
    config->subsel = EV_REL;

    if (config->size == 0)
        PANIC("virtio mouse EV_REL not supported");

    uint rel_code;
    for (int i = 0; i < NUM_MOUSE_REL_SUPPORTED; i++)
    {
        rel_code = mouse_rel_codes[i];
        if (!(config->u.bitmap[rel_code / 8] & (1 << rel_code % 8)))
            PANIC("virtio mouse rel code not supported");
    }
}

void virtio_mouse_request_event(struct virtio_mmio_desc_t *desc, struct virtio_queue_t *data)
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

void virtio_mouse_handle_syn_event(uint16 code, uint32 value)
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

void virtio_mouse_handle_key_event(uint16 code, uint32 value)
{
    switch (code)
    {
    case BTN_LEFT:
        printf("left click");
        if (value == 1)
        {
            LOGI("[pressed]");
        }
        else if (value == 0)
        {
            LOGI("[released]");
        }
        else
        {
            LOGI("[value not recognized]");
        }
        break;
    case BTN_RIGHT:
        printf("right click");
        if (value == 1)
        {
            LOGI("[pressed]");
        }
        else if (value == 0)
        {
            LOGI("[released]");
        }
        else
        {
            LOGI("[value not recognized]");
        }
        break;
    case BTN_MIDDLE:
        LOGI("middle click");
        break;
    case BTN_GEAR_DOWN:
        LOGI("gear down " $(value));
        break;
    case BTN_GEAR_UP:
        LOGI("gear UP ", $(value));
        break;
    default:
        LOGI("code " $((u64_t)code) " not recognized");
    }
}

void virtio_mouse_handle_rel_event(uint16 code, uint32 value)
{
    switch (code)
    {
    case REL_X:
        LOGI("x:" $(value));
        break;
    case REL_Y:
        LOGI("y:" $(value));
        break;
    case REL_WHEEL:
        LOGI("wheel:" $(value));
        break;
    default:
        LOGI("code " $(code) " not recognized");
        return;
    }
}

void virtio_mouse_handle_event(struct virtio_input_event *b)
{
    switch (b->type)
    {
    case EV_SYN:
        virtio_mouse_handle_syn_event(b->code, b->value);
        break;
    case EV_KEY:
        virtio_mouse_handle_key_event(b->code, b->value);
        break;
    case EV_REL:
        virtio_mouse_handle_rel_event(b->code, b->value);
        break;
    default:
        LOGI("EV TYPE " $(b->type) " NOT RECOGNIZED");
    }
}

static void free_desc(struct vring_desc *desc, void *data)
{
    virtio_mouse_t *vmouse = (virtio_mouse_t *)data;
    if (desc->flags == VRING_DESC_F_WRITE)
    {
        struct virtio_input_event *event = (struct virtio_input_event *)desc->addr;
        virtio_mouse_handle_event(event);
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
    virtio_mouse_t *vmouse = (virtio_mouse_t *)data;
    struct virtio_queue_t *eventq = &vmouse->eventq;
    virtio_mmio_interrupt_ack(vmouse->desc);
    virtio_device_irq_handler(eventq, free_desc, vmouse);
    virtio_mouse_request_event(vmouse->desc, eventq);
}

int virtio_mouse_probe(device_t *this, xjil_value_t *value)
{
    virtio_mouse_t *vmouse = dynamic_cast(virtio_mouse_t)(this);
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

    vmouse->desc = desc;
    virtio_mmio_setup(desc, virtio_block_get_features);
    virtio_mmio_queue_set(desc, &vmouse->eventq, 0);
    virtio_mmio_queue_set(desc, &vmouse->statusq, 1);

    struct virtio_queue_t *data = &vmouse->eventq;
    virtio_mouse_supports_key(desc);
    virtio_mouse_supports_rel(desc);
    virtio_mouse_request_event(vmouse->desc, data);

    init_waitqueue_head(&vmouse->wq_head);
    request_irq(irq, irq_handler, vmouse);
    return 0;
}

constructor(virtio_mouse_t)
{
    dynamic_cast(device_t)(this)->name = "virtio-mouse";
    dynamic_cast(device_t)(this)->probe = virtio_mouse_probe;
}