#pragma once

#include <riscv.h>
#include <types.h>
#include <core/class.h>
#include <core/kobject.h>
#include <core/vmo.h>
#include <kalloc.h>
#include <vm.h>

typedef u64_t vmr_prop_t;
#define VMR_READ (1 << 0)
#define VMR_WRITE (1 << 1)
#define VMR_EXEC (1 << 2)

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

bool_t vmspace_map_range(vmspace_t *vmspace, vaddr_t va, size_t len, vmr_prop_t flags, vmobject_t *vmo);
bool_t vmspace_unmap_range(vmspace_t *vmspace, vaddr_t va, size_t len);