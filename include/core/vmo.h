#pragma once

#include <types.h>
#include <radix.h>
#include <core/class.h>
#include <core/kobject.h>

typedef enum
{
	VMO_ANONYM = 0,		/* lazy allocation */
	VMO_DATA = 1,		/* immediate allocation */
	VMO_FILE = 2,		/* file backed */
	VMO_SHM = 3,		/* shared memory */
	VMO_USER_PAGER = 4, /* support user pager */
	VMO_DEVICE = 5,		/* memory mapped device registers */
} vmo_type_t;

class(vmobject_t, kobject_t)
{
	struct radix *radix; /* record physical pages */
	paddr_t start;
	size_t size;
	vmo_type_t type;
};

vmobject_t *vmo_create(u64_t size, u64_t type);
void vmo_init(vmobject_t *vmo, vmo_type_t type, size_t len, paddr_t paddr);

int sys_vmo_create(u64_t size, u64_t type);
int sys_vmo_write(u64_t slot, u64_t offset, u64_t user_ptr, u64_t len);
int sys_vmo_read(u64_t slot, u64_t offset, u64_t user_ptr, u64_t len);
int sys_vmo_map(u64_t target_process_slot, u64_t slot, u64_t addr, u64_t prot, u64_t flags);