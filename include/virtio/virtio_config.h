#pragma once

/* Status byte for guest to report progress, and synchronize features. */
/* We have seen device and processed generic fields (VIRTIO_CONFIG_F_VIRTIO) */
#define VIRTIO_CONFIG_S_ACKNOWLEDGE 1
/* We have found a driver for the device. */
#define VIRTIO_CONFIG_S_DRIVER 2
/* Driver has used its parts of the config, and is happy */
#define VIRTIO_CONFIG_S_DRIVER_OK 4
/* Driver has finished configuring features */
#define VIRTIO_CONFIG_S_FEATURES_OK 8
/* Device entered invalid state, driver must reset it */
#define VIRTIO_CONFIG_S_NEEDS_RESET 0x40
/* We've given up on this device. */
#define VIRTIO_CONFIG_S_FAILED 0x80

/*
 * Virtio feature bits VIRTIO_TRANSPORT_F_START through
 * VIRTIO_TRANSPORT_F_END are reserved for the transport
 * being used (e.g. virtio_ring, virtio_pci etc.), the
 * rest are per-device feature bits.
 */
#define VIRTIO_TRANSPORT_F_START 28
#define VIRTIO_TRANSPORT_F_END 38

#ifndef VIRTIO_CONFIG_NO_LEGACY
/* Do we get callbacks when the ring is completely used, even if we've
 * suppressed them? */
#define VIRTIO_F_NOTIFY_ON_EMPTY 24

/* Can the device handle any descriptor layout? */
#define VIRTIO_F_ANY_LAYOUT 27
#endif /* VIRTIO_CONFIG_NO_LEGACY */

/* v1.0 compliant. */
#define VIRTIO_F_VERSION_1 32

/*
 * If clear - device has the platform DMA (e.g. IOMMU) bypass quirk feature.
 * If set - use platform DMA tools to access the memory.
 *
 * Note the reverse polarity (compared to most other features),
 * this is for compatibility with legacy systems.
 */
#define VIRTIO_F_ACCESS_PLATFORM 33
/* Legacy name for VIRTIO_F_ACCESS_PLATFORM (for compatibility with old userspace) */
#define VIRTIO_F_IOMMU_PLATFORM VIRTIO_F_ACCESS_PLATFORM

/* This feature indicates support for the packed virtqueue layout. */
#define VIRTIO_F_RING_PACKED 34

/*
 * This feature indicates that memory accesses by the driver and the
 * device are ordered in a way described by the platform.
 */
#define VIRTIO_F_ORDER_PLATFORM 36

/*
 * Does the device support Single Root I/O Virtualization?
 */
#define VIRTIO_F_SR_IOV 37