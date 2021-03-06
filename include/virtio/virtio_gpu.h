#pragma once

#include <types.h>

#define VIRTIO_GPU_F_VIRGL               0
#define VIRTIO_GPU_F_EDID                1


enum virtio_gpu_ctrl_type {
	VIRTIO_GPU_UNDEFINED = 0,

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

	/* 3d commands */
	VIRTIO_GPU_CMD_CTX_CREATE = 0x0200,
	VIRTIO_GPU_CMD_CTX_DESTROY,
	VIRTIO_GPU_CMD_CTX_ATTACH_RESOURCE,
	VIRTIO_GPU_CMD_CTX_DETACH_RESOURCE,
	VIRTIO_GPU_CMD_RESOURCE_CREATE_3D,
	VIRTIO_GPU_CMD_TRANSFER_TO_HOST_3D,
	VIRTIO_GPU_CMD_TRANSFER_FROM_HOST_3D,
	VIRTIO_GPU_CMD_SUBMIT_3D,

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

struct virtio_gpu_ctrl_hdr {
	uint32_t type;
	uint32_t flags;
	uint64_t fence_id;
	uint32_t ctx_id;
	uint32_t padding;
};

/* data passed in the cursor vq */

struct virtio_gpu_cursor_pos {
	uint32_t scanout_id;
	uint32_t x;
	uint32_t y;
	uint32_t padding;
};

/* VIRTIO_GPU_CMD_UPDATE_CURSOR, VIRTIO_GPU_CMD_MOVE_CURSOR */
struct virtio_gpu_update_cursor {
	struct virtio_gpu_ctrl_hdr hdr;
	struct virtio_gpu_cursor_pos pos;  /* update & move */
	uint32_t resource_id;           /* update only */
	uint32_t hot_x;                 /* update only */
	uint32_t hot_y;                 /* update only */
	uint32_t padding;
};

/* data passed in the control vq, 2d related */

struct virtio_gpu_rect {
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
};

/* VIRTIO_GPU_CMD_RESOURCE_UNREF */
struct virtio_gpu_resource_unref {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t resource_id;
	uint32_t padding;
};

/* VIRTIO_GPU_CMD_RESOURCE_CREATE_2D: create a 2d resource with a format */
struct virtio_gpu_resource_create_2d {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t resource_id;
	uint32_t format;
	uint32_t width;
	uint32_t height;
};

/* VIRTIO_GPU_CMD_SET_SCANOUT */
struct virtio_gpu_set_scanout {
	struct virtio_gpu_ctrl_hdr hdr;
	struct virtio_gpu_rect r;
	uint32_t scanout_id;
	uint32_t resource_id;
};

/* VIRTIO_GPU_CMD_RESOURCE_FLUSH */
struct virtio_gpu_resource_flush {
	struct virtio_gpu_ctrl_hdr hdr;
	struct virtio_gpu_rect r;
	uint32_t resource_id;
	uint32_t padding;
};

/* VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D: simple transfer to_host */
struct virtio_gpu_transfer_to_host_2d {
	struct virtio_gpu_ctrl_hdr hdr;
	struct virtio_gpu_rect r;
	uint64_t offset;
	uint32_t resource_id;
	uint32_t padding;
};

struct virtio_gpu_mem_entry {
	uint64_t addr;
	uint32_t length;
	uint32_t padding;
};

/* VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING */
struct virtio_gpu_resource_attach_backing {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t resource_id;
	uint32_t nr_entries;
};

/* VIRTIO_GPU_CMD_RESOURCE_DETACH_BACKING */
struct virtio_gpu_resource_detach_backing {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t resource_id;
	uint32_t padding;
};

/* VIRTIO_GPU_RESP_OK_DISPLAY_INFO */
#define VIRTIO_GPU_MAX_SCANOUTS 16
struct virtio_gpu_resp_display_info {
	struct virtio_gpu_ctrl_hdr hdr;
	struct virtio_gpu_display_one {
		struct virtio_gpu_rect r;
		uint32_t enabled;
		uint32_t flags;
	} pmodes[VIRTIO_GPU_MAX_SCANOUTS];
};

/* data passed in the control vq, 3d related */

struct virtio_gpu_box {
	uint32_t x, y, z;
	uint32_t w, h, d;
};

/* VIRTIO_GPU_CMD_TRANSFER_TO_HOST_3D, VIRTIO_GPU_CMD_TRANSFER_FROM_HOST_3D */
struct virtio_gpu_transfer_host_3d {
	struct virtio_gpu_ctrl_hdr hdr;
	struct virtio_gpu_box box;
	uint64_t offset;
	uint32_t resource_id;
	uint32_t level;
	uint32_t stride;
	uint32_t layer_stride;
};

/* VIRTIO_GPU_CMD_RESOURCE_CREATE_3D */
#define VIRTIO_GPU_RESOURCE_FLAG_Y_0_TOP (1 << 0)
struct virtio_gpu_resource_create_3d {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t resource_id;
	uint32_t target;
	uint32_t format;
	uint32_t bind;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t array_size;
	uint32_t last_level;
	uint32_t nr_samples;
	uint32_t flags;
	uint32_t padding;
};

/* VIRTIO_GPU_CMD_CTX_CREATE */
struct virtio_gpu_ctx_create {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t nlen;
	uint32_t padding;
	char debug_name[64];
};

/* VIRTIO_GPU_CMD_CTX_DESTROY */
struct virtio_gpu_ctx_destroy {
	struct virtio_gpu_ctrl_hdr hdr;
};

/* VIRTIO_GPU_CMD_CTX_ATTACH_RESOURCE, VIRTIO_GPU_CMD_CTX_DETACH_RESOURCE */
struct virtio_gpu_ctx_resource {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t resource_id;
	uint32_t padding;
};

/* VIRTIO_GPU_CMD_SUBMIT_3D */
struct virtio_gpu_cmd_submit {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t size;
	uint32_t padding;
};

#define VIRTIO_GPU_CAPSET_VIRGL 1
#define VIRTIO_GPU_CAPSET_VIRGL2 2

/* VIRTIO_GPU_CMD_GET_CAPSET_INFO */
struct virtio_gpu_get_capset_info {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t capset_index;
	uint32_t padding;
};

/* VIRTIO_GPU_RESP_OK_CAPSET_INFO */
struct virtio_gpu_resp_capset_info {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t capset_id;
	uint32_t capset_max_version;
	uint32_t capset_max_size;
	uint32_t padding;
};

/* VIRTIO_GPU_CMD_GET_CAPSET */
struct virtio_gpu_get_capset {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t capset_id;
	uint32_t capset_version;
};

/* VIRTIO_GPU_RESP_OK_CAPSET */
struct virtio_gpu_resp_capset {
	struct virtio_gpu_ctrl_hdr hdr;
	uint8_t capset_data[];
};

/* VIRTIO_GPU_CMD_GET_EDID */
struct virtio_gpu_cmd_get_edid {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t scanout;
	uint32_t padding;
};

/* VIRTIO_GPU_RESP_OK_EDID */
struct virtio_gpu_resp_edid {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t size;
	uint32_t padding;
	uint8_t edid[1024];
};

#define VIRTIO_GPU_EVENT_DISPLAY (1 << 0)

struct virtio_gpu_config {
	uint32_t events_read;
	uint32_t events_clear;
	uint32_t num_scanouts;
	uint32_t num_capsets;
};

/* simple formats for fbcon/X use */
enum virtio_gpu_formats {
	VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM  = 1,
	VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM  = 2,
	VIRTIO_GPU_FORMAT_A8R8G8B8_UNORM  = 3,
	VIRTIO_GPU_FORMAT_X8R8G8B8_UNORM  = 4,

	VIRTIO_GPU_FORMAT_R8G8B8A8_UNORM  = 67,
	VIRTIO_GPU_FORMAT_X8B8G8R8_UNORM  = 68,

	VIRTIO_GPU_FORMAT_A8B8G8R8_UNORM  = 121,
	VIRTIO_GPU_FORMAT_R8G8B8X8_UNORM  = 134,
};