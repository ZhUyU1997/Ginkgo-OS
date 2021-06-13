#pragma once

#include <riscv.h>
#include <types.h>
#include <core/class.h>
#include <core/kobject.h>
#include <core/vmo.h>
#include <kalloc.h>
#include <vm.h>

typedef u64_t vmr_prop_t;

#define PROT_READ	0x1		/* Page can be read.  */
#define PROT_WRITE	0x2		/* Page can be written.  */
#define PROT_EXEC	0x4		/* Page can be executed.  */
#define PROT_NONE	0x0		/* Page can not be accessed.  */

struct vmregion
{
	struct list_head node; // vmr_list
	vaddr_t start;
	size_t size;
	vmr_prop_t perm;
	vmobject_t *vmo;
};

class(vmspace_t, kobject_t)
{
	/* list of vmregion */
	struct list_head vmr_list;
	/* root page table */
	pagetable_t pgtbl;

	struct vmregion *heap_vmr;
	vaddr_t user_current_heap;
};

bool_t vmspace_map_range_user(vmspace_t *vmspace, vaddr_t va, size_t len, u64_t prot, u64_t flags, vmobject_t *vmo);
bool_t vmspace_map_range_kernel(vmspace_t *vmspace, vaddr_t va, size_t len, u64_t prot, u64_t flags, vmobject_t *vmo);
bool_t vmspace_unmap_range(vmspace_t *vmspace, vaddr_t va, size_t len);