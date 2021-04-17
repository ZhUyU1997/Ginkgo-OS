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
#include <interrupt/interrupt.h>
#include <framebuffer/framebuffer.h>

class(virtio_framebuffer_t, framebuffer_t)
{
    struct virtio_mmio_desc_t *desc;
    struct virtio_queue_t controlq;
};

static uint32 get_features(uint32 features)
{
    return features;
}

static void create_2d_resource(struct virtio_queue_t *data, uint32_t resource_id, uint32_t width, uint32_t height)
{
    struct virtio_gpu_resource_create_2d *request = calloc(1, sizeof(struct virtio_gpu_resource_create_2d));
    struct virtio_gpu_ctrl_hdr *response = calloc(1, sizeof(struct virtio_gpu_ctrl_hdr));

    *request = (struct virtio_gpu_resource_create_2d){
        .hdr = (struct virtio_gpu_ctrl_hdr){
            .type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D,
        },
        .resource_id = resource_id,
        .format = VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM,
        .width = width,
        .height = height,
    };
    virtio_send_command_2(data, VRING_DESC(request), VRING_DESC(response));
}

static void attach_backing(struct virtio_queue_t *data, uint32_t resource_id, void *ptr, size_t buf_len)
{
    struct virtio_gpu_resource_attach_backing *request1 = calloc(1, sizeof(struct virtio_gpu_resource_attach_backing));
    struct virtio_gpu_mem_entry *request2 = calloc(1, sizeof(struct virtio_gpu_mem_entry));
    struct virtio_gpu_ctrl_hdr *response = calloc(1, sizeof(struct virtio_gpu_ctrl_hdr));

    *request1 = (struct virtio_gpu_resource_attach_backing){
        .hdr = (struct virtio_gpu_ctrl_hdr){
            .type = VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING,
        },
        .resource_id = resource_id,
        .nr_entries = 1,
    };
    *request2 = (struct virtio_gpu_mem_entry){
        .addr = (uint64)ptr,
        .length = buf_len,
    };

    virtio_send_command_3(data, VRING_DESC(request1), VRING_DESC(request2), VRING_DESC(response));
}

static void set_scanout(struct virtio_queue_t *data, uint32_t resource_id, uint32_t scanout_id, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    struct virtio_gpu_set_scanout *request = calloc(1, sizeof(struct virtio_gpu_set_scanout));
    struct virtio_gpu_ctrl_hdr *response = calloc(1, sizeof(struct virtio_gpu_ctrl_hdr));

    *request = (struct virtio_gpu_set_scanout){
        .hdr = (struct virtio_gpu_ctrl_hdr){
            .type = VIRTIO_GPU_CMD_SET_SCANOUT,
        },
        .r = (struct virtio_gpu_rect){x, y, width, height},
        .resource_id = resource_id,
        .scanout_id = scanout_id,
    };

    virtio_send_command_2(data, VRING_DESC(request), VRING_DESC(response));
}

static void transfer_to_host_2d(struct virtio_queue_t *data, uint32_t resource_id, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    struct virtio_gpu_transfer_to_host_2d *request = calloc(1, sizeof(struct virtio_gpu_transfer_to_host_2d));
    struct virtio_gpu_ctrl_hdr *response = calloc(1, sizeof(struct virtio_gpu_ctrl_hdr));

    *request = (struct virtio_gpu_transfer_to_host_2d){
        .hdr = (struct virtio_gpu_ctrl_hdr){
            .type = VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D,
        },
        .r = (struct virtio_gpu_rect){x, y, width, height},
        .resource_id = resource_id,
    };

    virtio_send_command_2(data, VRING_DESC(request), VRING_DESC(response));
}

static void flush_resource(struct virtio_queue_t *data, uint32_t resource_id, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    struct virtio_gpu_resource_flush *request = calloc(1, sizeof(struct virtio_gpu_resource_flush));
    struct virtio_gpu_ctrl_hdr *response = calloc(1, sizeof(struct virtio_gpu_ctrl_hdr));

    *request = (struct virtio_gpu_resource_flush){
        .hdr = (struct virtio_gpu_ctrl_hdr){
            .type = VIRTIO_GPU_CMD_RESOURCE_FLUSH,
        },
        .r = (struct virtio_gpu_rect){x, y, width, height},
        .resource_id = resource_id,
    };

    virtio_send_command_2(data, VRING_DESC(request), VRING_DESC(response));
}

static void update_cursor(struct virtio_queue_t *data, uint32_t resource_id, uint32_t scanout_id, uint32_t x, uint32_t y)
{
    struct virtio_gpu_update_cursor *request = calloc(1, sizeof(struct virtio_gpu_update_cursor));
    struct virtio_gpu_ctrl_hdr *response = calloc(1, sizeof(struct virtio_gpu_ctrl_hdr));

    *request = (struct virtio_gpu_update_cursor){
        .hdr = (struct virtio_gpu_ctrl_hdr){
            .type = VIRTIO_GPU_CMD_UPDATE_CURSOR,
        },
        .pos = (struct virtio_gpu_cursor_pos){
            .scanout_id = scanout_id,
            .x = x,
            .y = y,
        },
        .resource_id = resource_id,
    };

    virtio_send_command_2(data, VRING_DESC(request), VRING_DESC(response));
}

static void move_cursor(struct virtio_queue_t *data, uint32_t resource_id, uint32_t scanout_id, uint32_t x, uint32_t y)
{
    struct virtio_gpu_update_cursor *request = calloc(1, sizeof(struct virtio_gpu_update_cursor));
    struct virtio_gpu_ctrl_hdr *response = calloc(1, sizeof(struct virtio_gpu_ctrl_hdr));

    *request = (struct virtio_gpu_update_cursor){
        .hdr = (struct virtio_gpu_ctrl_hdr){
            .type = VIRTIO_GPU_CMD_MOVE_CURSOR,
        },
        .pos = (struct virtio_gpu_cursor_pos){
            .scanout_id = scanout_id,
            .x = x,
            .y = y,
        },
        .resource_id = resource_id,
    };

    virtio_send_command_2(data, VRING_DESC(request), VRING_DESC(response));
}

static void get_display_info(struct virtio_queue_t *data)
{
    struct virtio_gpu_ctrl_hdr *request = calloc(1, sizeof(struct virtio_gpu_ctrl_hdr));
    struct virtio_gpu_resp_display_info *response = calloc(1, sizeof(struct virtio_gpu_resp_display_info));

    *request = (struct virtio_gpu_ctrl_hdr){
        .type = VIRTIO_GPU_CMD_GET_DISPLAY_INFO,
    };

    virtio_send_command_2(data, VRING_DESC(request), VRING_DESC(response));
}

static void free_desc(struct vring_desc *desc, void *data)
{

    struct virtio_gpu_ctrl_hdr *hdr = (struct virtio_gpu_ctrl_hdr *)desc->addr;
    uint32_t type = hdr->type;

    switch (type)
    {
    case VIRTIO_GPU_RESP_OK_NODATA:
        LOGI("VIRTIO_GPU_RESP_OK_NODATA");
        break;
    case VIRTIO_GPU_RESP_OK_DISPLAY_INFO:
        LOGI("VIRTIO_GPU_RESP_OK_DISPLAY_INFO");
        struct virtio_gpu_resp_display_info *info = (struct virtio_gpu_resp_display_info *)hdr;
        for (int i = 0; i < VIRTIO_GPU_MAX_SCANOUTS; i++)
        {
            // LOGI("[" $(info->pmodes[i].r.x) "," $(info->pmodes[i].r.y) "," $(info->pmodes[i].r.width) "," $(info->pmodes[i].r.height) "]");
            // LOGI("enabled:" $(info->pmodes[i].enabled));
            // LOGI("flags:" $(info->pmodes[i].flags));
        }
        break;
    case VIRTIO_GPU_RESP_OK_CAPSET_INFO:
        LOGI("VIRTIO_GPU_RESP_OK_CAPSET_INFO");
        break;
    case VIRTIO_GPU_RESP_OK_CAPSET:
        LOGI("VIRTIO_GPU_RESP_OK_CAPSET");
        break;
    case VIRTIO_GPU_RESP_OK_EDID:
        LOGI("VIRTIO_GPU_RESP_OK_EDID");
        break;
    case VIRTIO_GPU_RESP_ERR_UNSPEC:
        LOGI("VIRTIO_GPU_RESP_ERR_UNSPEC");
        break;
    case VIRTIO_GPU_RESP_ERR_OUT_OF_MEMORY:
        LOGI("VIRTIO_GPU_RESP_ERR_OUT_OF_MEMORY");
        break;
    case VIRTIO_GPU_RESP_ERR_INVALID_SCANOUT_ID:
        LOGI("VIRTIO_GPU_RESP_ERR_INVALID_SCANOUT_ID");
        break;
    case VIRTIO_GPU_RESP_ERR_INVALID_RESOURCE_ID:
        LOGI("VIRTIO_GPU_RESP_ERR_INVALID_RESOURCE_ID");
        break;
    case VIRTIO_GPU_RESP_ERR_INVALID_CONTEXT_ID:
        LOGI("VIRTIO_GPU_RESP_ERR_INVALID_CONTEXT_ID");
        break;
    case VIRTIO_GPU_RESP_ERR_INVALID_PARAMETER:
        LOGI("VIRTIO_GPU_RESP_ERR_INVALID_PARAMETER");
        break;

    default:
        break;
    }
    free(hdr);
}

static void irq_handler(void *data)
{
    virtio_framebuffer_t *vfb = (virtio_framebuffer_t *)data;
    struct virtio_mmio_desc_t *desc = vfb->desc;
    virtio_mmio_interrupt_ack(desc);
    virtio_device_irq_handler(&vfb->controlq, free_desc, NULL);
}

class_impl(virtio_framebuffer_t, framebuffer_t){};

struct surface_t *virtio_framebuffer_create(framebuffer_t *this)
{
    virtio_framebuffer_t *fb = dynamic_cast(virtio_framebuffer_t)(this);
    struct virtio_queue_t *data = &fb->controlq;
    this->surface = surface_create(this->width, this->height, 4);
    memset(this->surface->pixels, 0, this->width * this->width * 4);

    uint32_t resource_id = 1;
    get_display_info(data);

    struct virtio_gpu_config *config = (struct virtio_gpu_config *)virtio_mmio_get_config(fb->desc);
    LOGI("events_read:" $(config->events_read));
    LOGI("events_clear:" $(config->events_clear));
    LOGI("num_scanouts:" $(config->num_scanouts));
    LOGI("num_capsets:" $(config->num_capsets));

    LOGI("STEP 1: Create a host resource using create 2d");
    create_2d_resource(data, resource_id, this->width, this->height);
    LOGI("STEP 2: Attach backing");
    attach_backing(data, resource_id, this->surface->pixels, this->width * this->height * 4);
    LOGI("STEP 3: Set scanout");
    set_scanout(data, resource_id, 0, 0, 0, this->width, this->height);
    LOGI("STEP 4: Transfer to host");
    transfer_to_host_2d(data, resource_id, 0, 0, this->width, this->height);
    LOGI("STEP 5: Flush");
    flush_resource(data, resource_id, 0, 0, this->width, this->height);

    virtio_mmio_notify(fb->desc);
    return this->surface;
}

void virtio_framebuffer_destroy(framebuffer_t *this, struct surface_t *s)
{
}

void virtio_framebuffer_present(framebuffer_t *this, struct surface_t *s)
{
    virtio_framebuffer_t *fb = dynamic_cast(virtio_framebuffer_t)(this);
    struct virtio_queue_t *data = &fb->controlq;
    uint32_t resource_id = 1;
    transfer_to_host_2d(data, resource_id, 0, 0, this->width, this->height);
    flush_resource(data, resource_id, 0, 0, this->width, this->height);
    virtio_mmio_notify(fb->desc);
}

int virtio_framebuffer_probe(device_t *this, xjil_value_t *value)
{
    virtio_framebuffer_t *vfb = dynamic_cast(virtio_framebuffer_t)(this);
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

    vfb->desc = desc;

    framebuffer_t *fb = dynamic_cast(framebuffer_t)(this);
    fb->width = xjil_read_int(value, "width", -1);
    fb->height = xjil_read_int(value, "height", -1);

    if (fb->width <= 0 || fb->height <= 0)
    {
        return -1;
    }

    virtio_mmio_setup(vfb->desc, get_features);
    virtio_mmio_queue_set(vfb->desc, &vfb->controlq, 0);

    request_irq(irq, irq_handler, vfb);
    return 0;
}

constructor(virtio_framebuffer_t)
{
    dynamic_cast(device_t)(this)->name = "virtio-block";
    dynamic_cast(device_t)(this)->probe = virtio_framebuffer_probe;

    dynamic_cast(framebuffer_t)(this)->create = virtio_framebuffer_create;
    dynamic_cast(framebuffer_t)(this)->destroy = virtio_framebuffer_destroy;
    dynamic_cast(framebuffer_t)(this)->present = virtio_framebuffer_present;
}