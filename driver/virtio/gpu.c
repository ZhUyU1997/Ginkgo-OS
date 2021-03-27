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
    struct virtio_device_data *data;
    void *framebuffer;
};

class_impl(virtio_framebuffer_t, framebuffer_t){};
struct surface_t *virtio_framebuffer_create(framebuffer_t *this)
{
    return NULL;
}

void virtio_framebuffer_destroy(framebuffer_t *this, struct surface_t *s)
{
}

void virtio_framebuffer_present(framebuffer_t *this, struct surface_t *s)
{
}

static uint32 get_features(uint32 features)
{
    return features;
}

static void virtio_framebuffer_init(framebuffer_t *this)
{
    virtio_framebuffer_t *fb = dynamic_cast(virtio_framebuffer_t)(this);
    struct virtio_device_data *data = fb->data;
    fb->framebuffer = malloc(this->width * this->height * 4);
    memset(fb->framebuffer, 0xff, this->width * this->width * 4);

    LOGI("STEP 1: Create a host resource using create 2d");
    {
        struct virtio_gpu_resource_create_2d *request = calloc(1, sizeof(struct virtio_gpu_resource_create_2d));
        struct virtio_gpu_ctrl_hdr *response = calloc(1, sizeof(struct virtio_gpu_resource_create_2d));

        *request = (struct virtio_gpu_resource_create_2d){
            .hdr = (struct virtio_gpu_ctrl_hdr){
                .type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D,
            },
            .resource_id = 1,
            .format = VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM,
            .width = this->width,
            .height = this->height,
        };

        uint32 head = virtio_desc_get_index(data);
        virtio_desc_new2(data, VRING_DESC(request), VRING_DESC(response));
        virtio_avail_new(data, head);
    }

    LOGI("STEP 2: Attach backing");
    {
        struct virtio_gpu_resource_attach_backing *request1 = calloc(1, sizeof(struct virtio_gpu_resource_attach_backing));
        struct virtio_gpu_mem_entry *request2 = calloc(1, sizeof(struct virtio_gpu_mem_entry));
        struct virtio_gpu_ctrl_hdr *response = calloc(1, sizeof(struct virtio_gpu_resource_create_2d));

        *request1 = (struct virtio_gpu_resource_attach_backing){
            .hdr = (struct virtio_gpu_ctrl_hdr){
                .type = VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING,
            },
            .resource_id = 1,
            .nr_entries = 1,
        };
        *request2 = (struct virtio_gpu_mem_entry){
            .addr = (uint64)fb->framebuffer,
            .length = this->width * this->height * 4,
        };

        uint32 head = virtio_desc_get_index(data);
        virtio_desc_new3(data, VRING_DESC(request1), VRING_DESC(request2), VRING_DESC(response));
        virtio_avail_new(data, head);
    }

    LOGI("STEP 3: Set scanout");
    {
        struct virtio_gpu_set_scanout *request = calloc(1, sizeof(struct virtio_gpu_set_scanout));
        struct virtio_gpu_ctrl_hdr *response = calloc(1, sizeof(struct virtio_gpu_resource_create_2d));

        *request = (struct virtio_gpu_set_scanout){
            .hdr = (struct virtio_gpu_ctrl_hdr){
                .type = VIRTIO_GPU_CMD_SET_SCANOUT,
            },
            .r = (struct virtio_gpu_rect){0, 0, this->width, this->height},
            .resource_id = 1,
        };

        uint32 head = virtio_desc_get_index(data);
        virtio_desc_new2(data, VRING_DESC(request), VRING_DESC(response));
        virtio_avail_new(data, head);
    }

    LOGI("STEP 4: Transfer to host");
    {
        struct virtio_gpu_transfer_to_host_2d *request = calloc(1, sizeof(struct virtio_gpu_transfer_to_host_2d));
        struct virtio_gpu_ctrl_hdr *response = calloc(1, sizeof(struct virtio_gpu_resource_create_2d));

        *request = (struct virtio_gpu_transfer_to_host_2d){
            .hdr = (struct virtio_gpu_ctrl_hdr){
                .type = VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D,
            },
            .r = (struct virtio_gpu_rect){0, 0, this->width, this->height},
            .resource_id = 1,
        };

        uint32 head = virtio_desc_get_index(data);
        virtio_desc_new2(data, VRING_DESC(request), VRING_DESC(response));
        virtio_avail_new(data, head);
    }

    LOGI("Step 5: Flush");
    {
        struct virtio_gpu_resource_flush *request = calloc(1, sizeof(struct virtio_gpu_resource_flush));
        struct virtio_gpu_ctrl_hdr *response = calloc(1, sizeof(struct virtio_gpu_resource_create_2d));

        *request = (struct virtio_gpu_resource_flush){
            .hdr = (struct virtio_gpu_ctrl_hdr){
                .type = VIRTIO_GPU_CMD_RESOURCE_FLUSH,
            },
            .r = (struct virtio_gpu_rect){0, 0, this->width, this->height},
            .resource_id = 1,
        };

        uint32 head = virtio_desc_get_index(data);
        virtio_desc_new2(data, VRING_DESC(request), VRING_DESC(response));
        virtio_avail_new(data, head);
    }
    virtio_mmio_notify(data);
}

static void free_desc(struct vring_desc *desc)
{
    uint32_t type;

    free(desc->addr);
    LOGI();
}

static void irq_handler(virtio_framebuffer_t *vfb)
{
    struct virtio_device_data *data = vfb->data;
    virtio_device_interrupt_ack(data);
    virtio_device_irq_handler(data, free_desc);
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

    struct virtio_device_data *data = virtio_mmio_search_device(device_id, virtio_mmio_bus);

    if (!data)
    {
        return -1;
    }

    vfb->data = data;

    framebuffer_t *fb = dynamic_cast(framebuffer_t)(this);
    fb->width = xjil_read_int(value, "width", -1);
    fb->height = xjil_read_int(value, "height", -1);

    if (fb->width <= 0 || fb->height <= 0)
    {
        return -1;
    }

    virtio_device_setup(vfb->data, get_features);
    request_irq(irq, irq_handler, vfb);
    virtio_framebuffer_init(dynamic_cast(framebuffer_t)(this));
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

register_driver(virtio_framebuffer_t);