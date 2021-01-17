#pragma once

#include <types.h>
#include <virtio/virtio_types.h>

/* This marks a buffer as continuing via the next field. */
#define VRING_DESC_F_NEXT 1
/* This marks a buffer as write-only (otherwise read-only). */
#define VRING_DESC_F_WRITE 2
/* This means the buffer contains a list of buffer descriptors. */
#define VRING_DESC_F_INDIRECT 4

/*
 * Mark a descriptor as available or used in packed ring.
 * Notice: they are defined as shifts instead of shifted values.
 */
#define VRING_PACKED_DESC_F_AVAIL 7
#define VRING_PACKED_DESC_F_USED 15

/* The Host uses this in used->flags to advise the Guest: don't kick me when
 * you add a buffer.  It's unreliable, so it's simply an optimization.  Guest
 * will still kick if it's out of buffers. */
#define VRING_USED_F_NO_NOTIFY 1
/* The Guest uses this in avail->flags to advise the Host: don't interrupt me
 * when you consume a buffer.  It's unreliable, so it's simply an
 * optimization.  */
#define VRING_AVAIL_F_NO_INTERRUPT 1

/* Enable events in packed ring. */
#define VRING_PACKED_EVENT_FLAG_ENABLE 0x0
/* Disable events in packed ring. */
#define VRING_PACKED_EVENT_FLAG_DISABLE 0x1
/*
 * Enable events for a specific descriptor in packed ring.
 * (as specified by Descriptor Ring Change Event Offset/Wrap Counter).
 * Only valid if VIRTIO_RING_F_EVENT_IDX has been negotiated.
 */
#define VRING_PACKED_EVENT_FLAG_DESC 0x2

/*
 * Wrap counter bit shift in event suppression structure
 * of packed ring.
 */
#define VRING_PACKED_EVENT_F_WRAP_CTR 15

/* We support indirect buffer descriptors */
#define VIRTIO_RING_F_INDIRECT_DESC 28

/* The Guest publishes the used index for which it expects an interrupt
 * at the end of the avail ring. Host should ignore the avail->flags field. */
/* The Host publishes the avail index for which it expects a kick
 * at the end of the used ring. Guest should ignore the used->flags field. */
#define VIRTIO_RING_F_EVENT_IDX 29

/* Alignment requirements for vring elements.
 * When using pre-virtio 1.0 layout, these fall out naturally.
 */
#define VRING_AVAIL_ALIGN_SIZE 2
#define VRING_USED_ALIGN_SIZE 4
#define VRING_DESC_ALIGN_SIZE 16

/* Virtio ring descriptors: 16 bytes.  These can chain together via "next". */
struct vring_desc
{
    /* Address (guest-physical). */
    __virtio64 addr;
    /* Length. */
    __virtio32 len;
    /* The flags as indicated above. */
    __virtio16 flags;
    /* We chain unused descriptors via this, too */
    __virtio16 next;
};

struct vring_avail
{
    __virtio16 flags;
    __virtio16 idx;
    __virtio16 ring[];
};

/* uint32_t is used here for ids for padding reasons. */
struct vring_used_elem
{
    /* Index of start of used descriptor chain. */
    __virtio32 id;
    /* Total length of the descriptor chain which was used (written to) */
    __virtio32 len;
};

typedef struct vring_used_elem __attribute__((aligned(VRING_USED_ALIGN_SIZE)))
vring_used_elem_t;

struct vring_used
{
    __virtio16 flags;
    __virtio16 idx;
    vring_used_elem_t ring[];
};

/*
 * The ring element addresses are passed between components with different
 * alignments assumptions. Thus, we might need to decrease the compiler-selected
 * alignment, and so must use a typedef to make sure the aligned attribute
 * actually takes hold:
 *
 * https://gcc.gnu.org/onlinedocs//gcc/Common-Type-Attributes.html#Common-Type-Attributes
 *
 * When used on a struct, or struct member, the aligned attribute can only
 * increase the alignment; in order to decrease it, the packed attribute must
 * be specified as well. When used as part of a typedef, the aligned attribute
 * can both increase and decrease alignment, and specifying the packed
 * attribute generates a warning.
 */
typedef struct vring_desc __attribute__((aligned(VRING_DESC_ALIGN_SIZE)))
vring_desc_t;
typedef struct vring_avail __attribute__((aligned(VRING_AVAIL_ALIGN_SIZE)))
vring_avail_t;
typedef struct vring_used __attribute__((aligned(VRING_USED_ALIGN_SIZE)))
vring_used_t;

struct vring
{
    unsigned int num;
    vring_desc_t *desc;
    vring_avail_t *avail;
    vring_used_t *used;
};

typedef struct vring vring_t;

#ifndef VIRTIO_RING_NO_LEGACY

/* The standard layout for the ring is a continuous chunk of memory which looks
 * like this.  We assume num is a power of 2.
 *
 * struct vring
 * {
 *	// The actual descriptors (16 bytes each)
 *	struct vring_desc desc[num];
 *
 *	// A ring of available descriptor heads with free-running index.
 *	__virtio16 avail_flags;
 *	__virtio16 avail_idx;
 *	__virtio16 available[num];
 *	__virtio16 used_event_idx;
 *
 *	// Padding to the next align boundary.
 *	char pad[];
 *
 *	// A ring of used descriptor heads with free-running index.
 *	__virtio16 used_flags;
 *	__virtio16 used_idx;
 *	struct vring_used_elem used[num];
 *	__virtio16 avail_event_idx;
 * };
 */
/* We publish the used event index at the end of the available ring, and vice
 * versa. They are at the end for backwards compatibility. */
#define vring_used_event(vr) ((vr)->avail->ring[(vr)->num])
#define vring_avail_event(vr) (*(__virtio16 *)&(vr)->used->ring[(vr)->num])

static inline void vring_init(struct vring *vr, unsigned int num, void *p,
                              unsigned long align)
{
    vr->num = num;
    vr->desc = p;
    vr->avail = (struct vring_avail *)((char *)p + num * sizeof(struct vring_desc));
    vr->used = (void *)(((uintptr_t)&vr->avail->ring[num] + sizeof(__virtio16) + align - 1) & ~(align - 1));
}

static inline unsigned vring_size(unsigned int num, unsigned long align)
{
    return ((sizeof(struct vring_desc) * num + sizeof(__virtio16) * (3 + num) + align - 1) & ~(align - 1)) + sizeof(__virtio16) * 3 + sizeof(struct vring_used_elem) * num;
}

#endif /* VIRTIO_RING_NO_LEGACY */

/* The following is used with USED_EVENT_IDX and AVAIL_EVENT_IDX */
/* Assuming a given event_idx value from the other side, if
 * we have just incremented index from old to new_idx,
 * should we trigger an event? */
static inline int vring_need_event(uint16_t event_idx, uint16_t new_idx, uint16_t old)
{
    /* Note: Xen has similar logic for notification hold-off
	 * in include/xen/interface/io/ring.h with req_event and req_prod
	 * corresponding to event_idx + 1 and new_idx respectively.
	 * Note also that req_event and req_prod in Xen start at 1,
	 * event indexes in virtio start at 0. */
    return (uint16_t)(new_idx - event_idx - 1) < (uint16_t)(new_idx - old);
}

struct vring_packed_desc_event
{
    /* Descriptor Ring Change Event Offset/Wrap Counter. */
    uint16_t off_wrap;
    /* Descriptor Ring Change Event Flags. */
    uint16_t flags;
};

struct vring_packed_desc
{
    /* Buffer Address. */
    uint64_t addr;
    /* Buffer Length. */
    uint32_t len;
    /* Buffer ID. */
    uint16_t id;
    /* The flags depending on descriptor type. */
    uint16_t flags;
};