#include <core/vmspace.h>
#include <core/vmo.h>
#include <string.h>
#include <vm.h>
#include <radix.h>
#include <malloc.h>
#include <list.h>
#include <log.h>

class_impl(vmspace_t, kobject_t){

};

constructor(vmspace_t)
{
	init_list_head(&this->vmr_list);
	/* alloc the root page table page */
	this->pgtbl = alloc_page(1);
	memset((void *)this->pgtbl, 0, PAGE_SIZE);
	map_kernel_page(this->pgtbl);
	/* architecture dependent initilization */
	this->user_current_heap = 0;
}

/* release the resource when a process exits */
int vmspace_destroy(vmspace_t *vmspace)
{
	return 0;
}

static struct vmregion *vmregion_alloc(void)
{
	struct vmregion *vmr;

	vmr = malloc(sizeof(*vmr));
	return vmr;
}

static void vmregion_free(struct vmregion *vmr)
{
	free((void *)vmr);
}

static bool_t check_vmr_intersect(vmspace_t *vmspace, struct vmregion *vmr_to_add)
{
	struct vmregion *vmr;
	vaddr_t new_start, start;
	vaddr_t new_end, end;

	new_start = vmr_to_add->start;
	new_end = new_start + vmr_to_add->size - 1;

	list_for_each_entry(vmr, &vmspace->vmr_list, node)
	{
		start = vmr->start;
		end = start + vmr->size;
		if ((new_start >= start && new_start < end) ||
			(new_end >= start && new_end < end))
			return TRUE;
	}
	return FALSE;
}

static bool_t is_vmr_in_vmspace(vmspace_t *vmspace, struct vmregion *vmr)
{
	struct vmregion *iter;
	list_for_each_entry(iter, &vmspace->vmr_list, node)
	{
		if (iter == vmr)
			return TRUE;
	}
	return FALSE;
}

static int add_vmr_to_vmspace(vmspace_t *vmspace, struct vmregion *vmr)
{
	if (check_vmr_intersect(vmspace, vmr))
	{
		LOGE("warning: vmr overlap\n");
		return FALSE;
	}
	list_add(&(vmr->node), &(vmspace->vmr_list));
	return TRUE;
}

static void del_vmr_from_vmspace(vmspace_t *vmspace, struct vmregion *vmr)
{
	if (!is_vmr_in_vmspace(vmspace, vmr))
		list_del(&(vmr->node));
	vmregion_free(vmr);
}

struct vmregion *find_vmr_for_va(vmspace_t *vmspace, vaddr_t addr)
{
	struct vmregion *vmr;
	vaddr_t start, end;

	list_for_each_entry(vmr, &vmspace->vmr_list, node)
	{
		start = vmr->start;
		end = start + vmr->size;
		if (addr >= start && addr < end)
			return vmr;
	}
	return NULL;
}

static int fill_page_table(vmspace_t *vmspace, struct vmregion *vmr)
{
	size_t vm_size;
	paddr_t pa;
	vaddr_t va;
	int ret;

	vm_size = vmr->vmo->size;
	pa = vmr->vmo->start;
	va = vmr->start;

	ret = map_range_in_pgtbl(vmspace->pgtbl, va, pa, vm_size, vmr->perm);

	return ret;
}

bool_t arch_vmspace_map_range(vmspace_t *vmspace, vaddr_t va, size_t len, u64_t flags, vmobject_t *vmo)
{
	struct vmregion *vmr;
	int ret;

	va = PGROUNDUP(va);
	if (len < PAGE_SIZE)
		len = PAGE_SIZE;

	vmr = vmregion_alloc();
	if (!vmr)
	{
		ret = FALSE;
		goto out_fail;
	}
	vmr->start = va;
	vmr->size = len;
	vmr->perm = flags;
	vmr->vmo = vmo;

	ret = add_vmr_to_vmspace(vmspace, vmr);

	if (ret < 0)
		goto out_free_vmr;

	/* on-demand mapping for anonymous mapping */
	if (vmo->type == VMO_DATA)
		fill_page_table(vmspace, vmr);
	return 0;
out_free_vmr:
	vmregion_free(vmr);
out_fail:
	return ret;
}

bool_t vmspace_map_range_user(vmspace_t *vmspace, vaddr_t va, size_t len, u64_t prot, u64_t flags, vmobject_t *vmo)
{
	u64_t arch_flags = 0;

	if (prot | PROT_EXEC)
		arch_flags |= PTE_X;
	if (prot | PROT_WRITE)
		arch_flags |= PTE_W;
	if (prot | PROT_READ)
		arch_flags |= PTE_R;
	return arch_vmspace_map_range(vmspace, va, len, arch_flags | PTE_U, vmo);
}

bool_t vmspace_map_range_kernel(vmspace_t *vmspace, vaddr_t va, size_t len, u64_t prot, u64_t flags, vmobject_t *vmo)
{
	u64_t arch_flags = 0;

	if (prot | PROT_EXEC)
		arch_flags |= PTE_X;
	if (prot | PROT_WRITE)
		arch_flags |= PTE_W;
	if (prot | PROT_READ)
		arch_flags |= PTE_R;
	return arch_vmspace_map_range(vmspace, va, len, arch_flags, vmo);
}

bool_t vmspace_unmap_range(vmspace_t *vmspace, vaddr_t va, size_t len)
{
	struct vmregion *vmr;
	vaddr_t start;
	size_t size;

	vmr = find_vmr_for_va(vmspace, va);
	if (!vmr)
		return -1;
	start = vmr->start;
	size = vmr->size;

	if ((va != start) && (len != size))
	{
		PANIC("we only support unmap a whole vmregion now.\n");
	}
	del_vmr_from_vmspace(vmspace, vmr);

	unmap_range_in_pgtbl(vmspace->pgtbl, va, len);

	return 0;
}
