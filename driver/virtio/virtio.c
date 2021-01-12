#include <virtio.h>

#include <types.h>
#include <riscv.h>
#include <memlayout.h>
#include <io.h>
#include <log.h>
#include <string.h>
#include <malloc.h>
#include <core/class.h>

//https://github.com/sgmarz/osblog/blob/master/risc_v/src/virtio.r

enum
{
    MMIO_VIRTIO_START = 0x10001000UL,
    MMIO_VIRTIO_END = 0x10008000UL,
    MMIO_VIRTIO_STRIDE = 0x1000UL,
    MMIO_VIRTIO_MAGIC = 0x74726976UL,
    MMIO_VIRTIO_NUM = 8,
};

typedef struct virtio_queue
{
    struct virtq_desc desc[VIRTIO_RING_SIZE];
    struct virtq_avail avail;
    char padding[PAGE_SIZE - (sizeof(struct virtq_desc) * VIRTIO_RING_SIZE) - sizeof(struct virtq_avail)];
    struct virtq_used used;
} virtio_queue_t;

class(virtio_device)
{
    int idx;
    addr_t addr;
    uint32 device_id;
    virtio_queue_t *queue;
    uint16 ack_used_idx;
    uint32 (*get_features)(uint32 features);
    void (*free_desc)(virtio_device * dev, int idx);
};

uint32 virtio_device_get_features(uint32 features)
{
    return features;
}

class_impl(virtio_device){
    get_features : virtio_device_get_features,
};

void virtio_device_gpu_free_desc(virtio_device *dev, int idx)
{
    free(dev->queue->desc[idx].addr);
}

class(virtio_device_gpu, virtio_device)
{
    addr_t framebuffer;
    uint32 width;
    uint32 height;
};

class_impl(virtio_device_gpu, virtio_device){};

constructor(virtio_device_gpu)
{
    virtio_device *device = dynamic_cast(virtio_device)(this);
    device->free_desc = virtio_device_gpu_free_desc;
}

class(virtio_device_block, virtio_device)
{
    uint8 status[VIRTIO_RING_SIZE];
};

class_impl(virtio_device_block, virtio_device){};

uint32 virtio_device_block_get_features(uint32 features)
{
    features &= ~(1 << VIRTIO_BLK_F_RO);
    features &= ~(1 << VIRTIO_BLK_F_SCSI);
    features &= ~(1 << VIRTIO_BLK_F_CONFIG_WCE);
    features &= ~(1 << VIRTIO_BLK_F_MQ);
    features &= ~(1 << VIRTIO_F_ANY_LAYOUT);
    features &= ~(1 << VIRTIO_RING_F_EVENT_IDX);
    features &= ~(1 << VIRTIO_RING_F_INDIRECT_DESC);
    return features;
}

void virtio_device_block_free_desc(virtio_device *dev, int idx)
{
    virtio_device_block *block = dynamic_cast(virtio_device_block)(dev);
    uint16 flags = dev->queue->desc[idx].flags;
    uint16 next = dev->queue->desc[idx].next;

    if ((flags & VIRTIO_DESC_F_WRITE) && (next == 0))
    {
        // block->status[idx] = 0;
    }
}

constructor(virtio_device_block)
{
    virtio_device *device = dynamic_cast(virtio_device)(this);
    device->get_features = virtio_device_block_get_features;
    device->free_desc = virtio_device_block_free_desc;
}

virtio_device *vdev[MMIO_VIRTIO_NUM] = {};

enum virtio_gpu_ctrl_type
{
    /* 2d commands */
    VIRTIO_GPU_CMD_GET_DISPLAY_INFO = 0x0100,
    VIRTIO_GPU_CMD_RESOURCE_CREATE_2D,
    VIRTIO_GPU_CMD_RESOURCE_UNREF,
    VIRTIO_GPU_CMD_SET_SCANOUT,
    VIRTIO_GPU_CMD_RESOURCE_FLUSH,
    VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D,
    VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING,
    VIRTIO_GPU_CMD_RESOURCE_DETACH_BACKING,
    VIRTIO_GPU_CMD_GET_CAPSET_INFO,
    VIRTIO_GPU_CMD_GET_CAPSET,
    VIRTIO_GPU_CMD_GET_EDID,
    /* cursor commands */
    VIRTIO_GPU_CMD_UPDATE_CURSOR = 0x0300,
    VIRTIO_GPU_CMD_MOVE_CURSOR,
    /* success responses */
    VIRTIO_GPU_RESP_OK_NODATA = 0x1100,
    VIRTIO_GPU_RESP_OK_DISPLAY_INFO,
    VIRTIO_GPU_RESP_OK_CAPSET_INFO,
    VIRTIO_GPU_RESP_OK_CAPSET,
    VIRTIO_GPU_RESP_OK_EDID,
    /* error responses */
    VIRTIO_GPU_RESP_ERR_UNSPEC = 0x1200,
    VIRTIO_GPU_RESP_ERR_OUT_OF_MEMORY,
    VIRTIO_GPU_RESP_ERR_INVALID_SCANOUT_ID,
    VIRTIO_GPU_RESP_ERR_INVALID_RESOURCE_ID,
    VIRTIO_GPU_RESP_ERR_INVALID_CONTEXT_ID,
    VIRTIO_GPU_RESP_ERR_INVALID_PARAMETER,
};

#define VIRTIO_GPU_FLAG_FENCE (1 << 0)

struct virtio_gpu_ctrl_hdr
{
    uint32 type;
    uint32 flags;
    uint64 fence_id;
    uint32 ctx_id;
    uint32 padding;
};

#define VIRTIO_GPU_MAX_SCANOUTS 16
struct virtio_gpu_rect
{
    uint32 x;
    uint32 y;
    uint32 width;
    uint32 height;
};

struct virtio_gpu_resp_display_info
{
    struct virtio_gpu_ctrl_hdr hdr;
    struct virtio_gpu_display_one
    {
        struct virtio_gpu_rect r;
        uint32 enabled;
        uint32 flags;
    } pmodes[VIRTIO_GPU_MAX_SCANOUTS];
};

enum virtio_gpu_formats
{
    VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM = 1,
    VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM = 2,
    VIRTIO_GPU_FORMAT_A8R8G8B8_UNORM = 3,
    VIRTIO_GPU_FORMAT_X8R8G8B8_UNORM = 4,
    VIRTIO_GPU_FORMAT_R8G8B8A8_UNORM = 67,
    VIRTIO_GPU_FORMAT_X8B8G8R8_UNORM = 68,
    VIRTIO_GPU_FORMAT_A8B8G8R8_UNORM = 121,
    VIRTIO_GPU_FORMAT_R8G8B8X8_UNORM = 134,
};
struct virtio_gpu_resource_create_2d
{
    struct virtio_gpu_ctrl_hdr hdr;
    uint32 resource_id;
    uint32 format;
    uint32 width;
    uint32 height;
};

struct virtio_gpu_resource_attach_backing
{
    struct virtio_gpu_ctrl_hdr hdr;
    uint32 resource_id;
    uint32 nr_entries;
};

struct virtio_gpu_mem_entry
{
    uint64 addr;
    uint32 length;
    uint32 padding;
};

struct virtio_gpu_set_scanout
{
    struct virtio_gpu_ctrl_hdr hdr;
    struct virtio_gpu_rect r;
    uint32 scanout_id;
    uint32 resource_id;
};

struct virtio_gpu_transfer_to_host_2d
{
    struct virtio_gpu_ctrl_hdr hdr;
    struct virtio_gpu_rect r;
    uint64 offset;
    uint32 resource_id;
    uint32 padding;
};

struct virtio_gpu_resource_flush
{
    struct virtio_gpu_ctrl_hdr hdr;
    struct virtio_gpu_rect r;
    uint32 resource_id;
    uint32 padding;
};

void virtio_desc_new(struct virtio_device *dev, struct virtq_desc *desc)
{
    dev->queue->desc[dev->idx] = *desc;
    dev->idx = (dev->idx + 1) % VIRTIO_RING_SIZE;
}

void virtio_desc_del(struct virtio_device *dev, int idx)
{
    struct virtq_desc *desc = dev->queue->desc;

    while (1)
    {

        dev->free_desc(dev, idx);
        desc[idx] = (struct virtq_desc){};

        if (desc[idx].flags & VIRTIO_DESC_F_NEXT)
            idx = desc[idx].next;
        else
            break;
    }
}

void virtio_desc_new2(virtio_device *dev, struct virtq_desc *request, struct virtq_desc *response)
{
    request->flags |= VIRTIO_DESC_F_NEXT;
    request->next = (dev->idx + 1) % VIRTIO_RING_SIZE,
    dev->queue->desc[dev->idx] = *request;
    dev->idx = (dev->idx + 1) % VIRTIO_RING_SIZE;
    LOGD("dev->idx:" $(dev->idx));

    response->flags |= VIRTIO_DESC_F_WRITE;
    dev->queue->desc[dev->idx] = *response;
    dev->idx = (dev->idx + 1) % VIRTIO_RING_SIZE;

    LOGD("dev->idx:" $(dev->idx));
}

void virtio_desc_new3(virtio_device *dev, struct virtq_desc *request1, struct virtq_desc *request2, struct virtq_desc *response)
{
    request1->flags |= VIRTIO_DESC_F_NEXT;
    request1->next = (dev->idx + 1) % VIRTIO_RING_SIZE,
    dev->queue->desc[dev->idx] = *request1;
    dev->idx = (dev->idx + 1) % VIRTIO_RING_SIZE;

    request2->flags |= VIRTIO_DESC_F_NEXT;
    request2->next = (dev->idx + 1) % VIRTIO_RING_SIZE,
    dev->queue->desc[dev->idx] = *request2;
    dev->idx = (dev->idx + 1) % VIRTIO_RING_SIZE;

    response->flags |= VIRTIO_DESC_F_WRITE;
    dev->queue->desc[dev->idx] = *response;
    dev->idx = (dev->idx + 1) % VIRTIO_RING_SIZE;
}

void virtio_avail_new(virtio_device *dev, int idx)
{
    LOGD("idx:" $(idx) " dev->queue->avail.idx:" $(dev->queue->avail.idx));
    dev->queue->avail.ring[dev->queue->avail.idx % VIRTIO_RING_SIZE] = idx;
    __sync_synchronize();
    dev->queue->avail.idx += 1;
    __sync_synchronize();
}

void gpu_interrupt(int idx)
{
    LOGI("gpu_interrupt idx:"$(idx));
    write32(VIRTIO7 + VIRTIO_MMIO_INTERRUPT_ACK, read32(VIRTIO7 + VIRTIO_MMIO_INTERRUPT_STATUS) & 0x3);

    virtio_queue_t *queue = vdev[idx]->queue;
    struct virtio_device *dev = vdev[idx];

    LOGD("dev->ack_used_idx:" $(dev->ack_used_idx));
    LOGD("queue->used.idx:" $(queue->used.idx));

    while (dev->ack_used_idx != queue->used.idx)
    {
        LOGD("handle");
        int idx = queue->used.ring[dev->ack_used_idx % VIRTIO_RING_SIZE].id;
        virtio_desc_del(dev, idx);
        __sync_synchronize();
        dev->ack_used_idx += 1;
        __sync_synchronize();
    }
}

static void clear(uint32 *buffer, int w, int h)
{
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            buffer[i * w + j] = 0xffff0000;
        }
    }
}

void virtio_disk_rw(void *addr, uint64 sector, uint32 size, int write)
{
    virtio_device *dev = vdev[0];
    struct virtio_device_block *block = dynamic_cast(virtio_device_block)(dev);

    // the spec's Section 5.2 says that legacy block operations use
    // three descriptors: one for type/reserved/sector, one for the
    // data, one for a 1-byte status result.

    // allocate the three descriptors.

    // format the three descriptors.
    // qemu's virtio-blk.c reads them.

    struct virtio_blk_req *buf0 = calloc(1, sizeof(struct virtio_blk_req));

    if (write)
        buf0->type = VIRTIO_BLK_T_OUT; // write the disk
    else
        buf0->type = VIRTIO_BLK_T_IN; // read the disk
    buf0->reserved = 0;
    buf0->sector = sector;

    uint32 head = dev->idx;

    uint32 flags = 0;
    if (!write)
        flags = VIRTIO_DESC_F_WRITE;

    block->status[head] = 0;
    virtio_desc_new3(dev,
                     &(struct virtq_desc){
                         .addr = buf0,
                         .len = sizeof(struct virtio_blk_req),
                     },
                     &(struct virtq_desc){
                         .addr = addr,
                         .len = size,
                         .flags = flags,
                     },
                     &(struct virtq_desc){
                         .addr = (uint64)&block->status[head],
                         .len = sizeof(uint8),
                     });

    virtio_avail_new(dev, head);

    write32(dev->addr + VIRTIO_MMIO_QUEUE_NOTIFY, 0);
    while (block->status[head] == 0xff)
        ;
}

void gpu_init(virtio_device *dev)
{
    struct virtio_device_gpu *gpu = dynamic_cast(virtio_device_gpu)(dev);
    gpu->width = 640;
    gpu->height = 480;
    gpu->framebuffer = malloc(gpu->width * gpu->height * 4);

    clear(gpu->framebuffer, gpu->width, gpu->height);

    LOGI("STEP 1: Create a host resource using create 2d");
    // //// STEP 1: Create a host resource using create 2d
    {
        struct virtio_gpu_resource_create_2d *request = calloc(1, sizeof(struct virtio_gpu_resource_create_2d));
        struct virtio_gpu_ctrl_hdr *response = calloc(1, sizeof(struct virtio_gpu_resource_create_2d));
        LOGD("request:" $(request));
        LOGD("response:" $(response));
        *request = (struct virtio_gpu_resource_create_2d){
            .hdr = (struct virtio_gpu_ctrl_hdr){
                .type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D,
            },
            .resource_id = 1,
            .format = VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM,
            .width = gpu->width,
            .height = gpu->height,
        };

        LOGD("virtio_gpu_resource_create_2d:" $(sizeof(struct virtio_gpu_resource_create_2d)));
        LOGD("virtio_gpu_ctrl_hdr:" $(sizeof(struct virtio_gpu_ctrl_hdr)));
        LOGD("dev->queue:" $(dev->queue));
        LOGD("&dev->queue->used:" $(&dev->queue->used));

        uint32 head = dev->idx;
        virtio_desc_new2(dev,
                         &(struct virtq_desc){
                             .addr = request,
                             .len = sizeof(struct virtio_gpu_resource_create_2d),
                         },
                         &(struct virtq_desc){
                             .addr = response,
                             .len = sizeof(struct virtio_gpu_ctrl_hdr),
                         });

        virtio_avail_new(dev, head);
    }

    LOGI("STEP 2: Attach backing");
    // //// STEP 2: Attach backing
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
            .addr = gpu->framebuffer,
            .length = gpu->width * gpu->height * 4,
        };

        uint32 head = dev->idx;
        virtio_desc_new3(dev,
                         &(struct virtq_desc){
                             .addr = request1,
                             .len = sizeof(struct virtio_gpu_resource_attach_backing),
                         },
                         &(struct virtq_desc){
                             .addr = request2,
                             .len = sizeof(struct virtio_gpu_mem_entry),
                         },
                         &(struct virtq_desc){
                             .addr = response,
                             .len = sizeof(struct virtio_gpu_ctrl_hdr),
                         });

        virtio_avail_new(dev, head);
    }

    LOGI("STEP 3: Set scanout");
    // //// STEP 3: Set scanout
    {
        struct virtio_gpu_set_scanout *request = calloc(1, sizeof(struct virtio_gpu_set_scanout));
        struct virtio_gpu_ctrl_hdr *response = calloc(1, sizeof(struct virtio_gpu_resource_create_2d));

        *request = (struct virtio_gpu_set_scanout){
            .hdr = (struct virtio_gpu_ctrl_hdr){
                .type = VIRTIO_GPU_CMD_SET_SCANOUT,
            },
            .r = (struct virtio_gpu_rect){0, 0, gpu->width, gpu->height},
            .resource_id = 1,
        };

        uint32 head = dev->idx;
        virtio_desc_new2(dev,
                         &(struct virtq_desc){
                             .addr = request,
                             .len = sizeof(struct virtio_gpu_set_scanout),
                         },
                         &(struct virtq_desc){
                             .addr = response,
                             .len = sizeof(struct virtio_gpu_ctrl_hdr),
                         });

        virtio_avail_new(dev, head);
    }

    LOGI("STEP 4: Transfer to host");
    // //// STEP 4: Transfer to host
    {
        struct virtio_gpu_transfer_to_host_2d *request = calloc(1, sizeof(struct virtio_gpu_transfer_to_host_2d));
        struct virtio_gpu_ctrl_hdr *response = calloc(1, sizeof(struct virtio_gpu_resource_create_2d));

        *request = (struct virtio_gpu_transfer_to_host_2d){
            .hdr = (struct virtio_gpu_ctrl_hdr){
                .type = VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D,
            },
            .r = (struct virtio_gpu_rect){0, 0, gpu->width, gpu->height},
            .resource_id = 1,
        };

        uint32 head = dev->idx;
        virtio_desc_new2(dev,
                         &(struct virtq_desc){
                             .addr = request,
                             .len = sizeof(struct virtio_gpu_transfer_to_host_2d),
                         },
                         &(struct virtq_desc){
                             .addr = response,
                             .len = sizeof(struct virtio_gpu_ctrl_hdr),
                         });

        virtio_avail_new(dev, head);
    }

    LOGI("Step 5: Flush");
    // Step 5: Flush
    {
        struct virtio_gpu_resource_flush *request = calloc(1, sizeof(struct virtio_gpu_resource_flush));
        struct virtio_gpu_ctrl_hdr *response = calloc(1, sizeof(struct virtio_gpu_resource_create_2d));

        *request = (struct virtio_gpu_resource_flush){
            .hdr = (struct virtio_gpu_ctrl_hdr){
                .type = VIRTIO_GPU_CMD_RESOURCE_FLUSH,
            },
            .r = (struct virtio_gpu_rect){0, 0, gpu->width, gpu->height},
            .resource_id = 1,
        };

        uint32 head = dev->idx;
        virtio_desc_new2(dev,
                         &(struct virtq_desc){
                             .addr = request,
                             .len = sizeof(struct virtio_gpu_resource_flush),
                         },
                         &(struct virtq_desc){
                             .addr = response,
                             .len = sizeof(struct virtio_gpu_ctrl_hdr),
                         });

        virtio_avail_new(dev, head);
    }

    write32(dev->addr + VIRTIO_MMIO_QUEUE_NOTIFY, 0);
}

static int setup_virtio_device(virtio_device *dev)
{
    addr_t addr = dev->addr;

    LOGD("addr:" $(addr));
    uint32 status = 0;

    // [Driver] Device Initialization
    // 1. Reset the device (write 0 into status)
    write32(addr + VIRTIO_MMIO_STATUS, 0);
    // 2. Set ACKNOWLEDGE status bit
    status |= VIRTIO_CONFIG_S_ACKNOWLEDGE;
    write32(addr + VIRTIO_MMIO_STATUS, status);
    // 3. Set the DRIVER status bit
    status |= VIRTIO_CONFIG_S_DRIVER;
    write32(addr + VIRTIO_MMIO_STATUS, status);
    // 4. Read device feature bits, write subset of feature
    // bits understood by OS and driver to the device.
    uint32 features = read32(addr + VIRTIO_MMIO_DEVICE_FEATURES);
    LOGD("feature:", $(features));
    write32(addr + VIRTIO_MMIO_DRIVER_FEATURES, dev->get_features(features));
    // 5. Set the FEATURES_OK status bit
    status |= VIRTIO_CONFIG_S_FEATURES_OK;
    write32(addr + VIRTIO_MMIO_STATUS, status);
    // 6. Re-read status to ensure FEATURES_OK is still set.
    // Otherwise, it doesn't support our features.
    uint32 status_ok = read32(addr + VIRTIO_MMIO_STATUS);
    // If the status field no longer has features_ok set,
    // that means that the device couldn't accept
    // the features that we request. Therefore, this is
    // considered a "failed" state.
    if (!(status_ok & VIRTIO_CONFIG_S_FEATURES_OK))
    {
        LOGI("features fail...");
        write32(addr + VIRTIO_MMIO_STATUS, VIRTIO_CONFIG_S_FAILED);
        return 0;
    }

    // 7. Perform device-specific setup.
    // Set the queue num. We have to make sure that the
    // queue size is valid because the device can only take
    // a certain size.
    uint32 queue_num_max = read32(addr + VIRTIO_MMIO_QUEUE_NUM_MAX);
    LOGI("QUEUE_NUM_MAX:" $(queue_num_max));
    write32(addr + VIRTIO_MMIO_QUEUE_NUM, VIRTIO_RING_SIZE);

    if (VIRTIO_RING_SIZE > queue_num_max)
    {
        LOGI("queue size fail...");
        write32(addr + VIRTIO_MMIO_STATUS, VIRTIO_CONFIG_S_FAILED);
        return 0;
    }
    // First, if the block device array is empty, create it!
    // We add 4095 to round this up and then do an integer
    // divide to truncate the decimal. We don't add 4096,
    // because if it is exactly 4096 bytes, we would get two
    // pages, not one.
    uint32 num_pages = (sizeof(struct virtio_queue) + PAGE_SIZE - 1) / PAGE_SIZE;
    LOGI("num_pages:" $(num_pages));
    // We allocate a page for each device. This will the the
    // descriptor where we can communicate with the block
    // device. We will still use an MMIO register (in
    // particular, QueueNotify) to actually tell the device
    // we put something in memory. We also have to be
    // careful with memory ordering. We don't want to
    // issue a notify before all memory writes have
    // finished. We will look at that later, but we need
    // what is called a memory "fence" or barrier.
    write32(addr + VIRTIO_MMIO_QUEUE_SEL, 0);
    // TODO: Set up queue #1 (cursorq)

    // Alignment is very important here. This is the memory address
    // alignment between the available and used rings. If this is wrong,
    // then we and the device will refer to different memory addresses
    // and hence get the wrong data in the used ring.
    // ptr.add(MmioOffsets::QueueAlign.scale32()).write_volatile(2);
    write32(addr + VIRTIO_MMIO_GUEST_PAGE_SIZE, PAGE_SIZE);

    // QueuePFN is a physical page number, however it
    // appears for QEMU we have to write the entire memory
    // address. This is a physical memory address where we
    // (the OS) and the block device have in common for
    // making and receiving requests.

    addr_t queue_ptr = calloc(1, num_pages * PAGE_SIZE);
    LOGD("queue_ptr:" $(queue_ptr));
    uint32 queue_pfn = queue_ptr >> PAGE_SHIFT;
    write32(addr + VIRTIO_MMIO_QUEUE_PFN, queue_pfn);

    // 8. Set the DRIVER_OK status bit. Device is now "live"
    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    write32(addr + VIRTIO_MMIO_STATUS, status);

    dev->queue = queue_ptr;
    return 1;
}

static int setup_block_device(int idx, addr_t addr)
{
    virtio_device_block *block = new (virtio_device_block);
    virtio_device *dev = dynamic_cast(virtio_device)(block);
    dev->addr = addr;
    dev->idx = idx;
    setup_virtio_device(dev);

    vdev[idx] = dev;
}

static int setup_gpu_device(int idx, addr_t addr)
{
    virtio_device_gpu *gpu = new (virtio_device_gpu);
    virtio_device *dev = dynamic_cast(virtio_device)(gpu);
    dev->addr = addr;
    dev->idx = idx;
    setup_virtio_device(dev);

    vdev[idx] = dev;
}

void virtio_init()
{
    for (addr_t addr = MMIO_VIRTIO_START; addr <= MMIO_VIRTIO_END; addr += MMIO_VIRTIO_STRIDE)
    {
        uint32 magic_value = read32(addr + VIRTIO_MMIO_MAGIC_VALUE);
        uint32 version = read32(addr + VIRTIO_MMIO_VERSION);
        uint32 device_id = read32(addr + VIRTIO_MMIO_DEVICE_ID);
        uint32 vendor_id = read32(addr + VIRTIO_MMIO_VENDOR_ID);

        // 0x74_72_69_76 is "virt" in little endian, so in reality
        // it is triv. All VirtIO devices have this attached to the
        // MagicValue register (offset 0x000)
        if (magic_value != 0x74726976)
        {
            LOGI("not virtio.");
        }
        else
        {
            int idx = (addr - MMIO_VIRTIO_START) / MMIO_VIRTIO_STRIDE;
            switch (device_id)
            {
            case 0:
                LOGI("reserved (invalid)");
                break;
            case 1:
                LOGI("network card");
                break;
            case 2:
                LOGI("block device");
                setup_block_device(idx, addr);
                break;
            case 3:
                LOGI("console");
                break;
            case 4:
                LOGI("entropy source");
                break;
            case 5:
                LOGI("memory ballooning (traditional)");
                break;
            case 6:
                LOGI("ioMemory");
                break;
            case 7:
                LOGI("rpmsg");
                break;
            case 8:
                LOGI("SCSI host");
                break;
            case 9:
                LOGI("9P transport");
                break;
            case 10:
                LOGI("mac80211 wlan");
                break;
            case 11:
                LOGI("rproc serial");
                break;
            case 12:
                LOGI("virtio CAIF");
                break;
            case 13:
                LOGI("memory balloon");
                break;
            case 16:
                LOGI("GPU device");
                setup_gpu_device(idx, addr);
                gpu_init(vdev[idx]);
                break;
            case 17:
                LOGI("Timer/Clock device");
                break;
            case 18:
                LOGI("Input device");
                break;
            case 19:
                LOGI("Socket device");
                break;
            case 20:
                LOGI("Crypto device");
                break;
            case 21:
                LOGI("Signal Distribution Module");
                break;
            case 22:
                LOGI("pstore device");
                break;
            case 23:
                LOGI("IOMMU device");
                break;
            case 24:
                LOGI("Memory device");
                break;
            default:
                LOGI("unknown device type.");
                break;
            }
        }
    }
}

void virtio_handle_interrupt(int irq)
{
    LOGD("virtio_handle_interrupt");
    gpu_interrupt(irq - 1);
}