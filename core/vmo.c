#include <core/vmspace.h>
#include <core/vmo.h>
#include <task.h>
#include <radix.h>
#include <malloc.h>
#include <uaccess.h>
#include <log.h>

class_impl(vmobject_t, kobject_t){};

void vmo_init(vmobject_t *vmo, vmo_type_t type, size_t len, paddr_t paddr)
{
    len = PGROUNDUP(len);
    vmo->size = len;
    vmo->type = type;

    /* for a VMO_DATA, the user will use it soon (we expect) */
    if (type == VMO_DATA)
    {
        /* kmalloc(>2048) returns continous physical pages */
        vmo->start = (paddr_t)malloc(len);
    }
    else if (type == VMO_DEVICE)
    {
        vmo->start = paddr;
    }
    else
    {
        /*
		 * for stack, heap, we do not allocate the physical memory at
		 * once
		 */
        vmo->radix = new_radix();
        init_radix(vmo->radix);
    }
}

vmobject_t *vmo_create(u64_t size, u64_t type)
{
    vmobject_t *vmo = new (vmobject_t);

    if (vmo == NULL)
        return NULL;
    vmo_init(vmo, type, size, 0);

    return vmo;
}

int sys_vmo_create(u64_t size, u64_t type)
{
    vmobject_t *vmo = new (vmobject_t);

    if (vmo == NULL)
        goto out_fail;
    vmo_init(vmo, type, size, 0);

    int slot = slot_alloc_install(process_self(), vmo);
    if (slot == -1)
        goto out_free_obj;
    return slot;
out_free_obj:
    delete (vmo);
out_fail:
    return -1;
}

#define WRITE_PMO 0
#define READ_PMO 1
static int read_write_vmo(u64_t slot, u64_t offset, u64_t user_buf,
                          u64_t size, u64_t type)
{
    vmobject_t *vmo;
    int r = 0;

    /* caller should have the slot */
    vmo = dynamic_cast(vmobject_t)(slot_get(process_self(), slot));
    if (!vmo)
    {
        r = -1;
        goto out_fail;
    }

    /* we only allow writing PMO_DATA now. */
    if (vmo->type != VMO_DATA)
    {
        r = -1;
        goto out_fail;
    }

    if (offset + size < offset || offset + size > vmo->size)
    {
        r = -1;
        goto out_fail;
    }

    if (type == WRITE_PMO)
        r = copy_from_user((char *)phys_to_virt(vmo->start) + offset,
                           (char *)user_buf, size);
    else if (type == READ_PMO)
        r = copy_to_user((char *)user_buf,
                         (char *)phys_to_virt(vmo->start) + offset,
                         size);
    else
        PANIC("read write vmo invalid type\n");

out_fail:
    return r;
}

int sys_vmo_write(u64_t slot, u64_t offset, u64_t user_ptr, u64_t len)
{
    return read_write_vmo(slot, offset, user_ptr, len, WRITE_PMO);
}

int sys_vmo_read(u64_t slot, u64_t offset, u64_t user_ptr, u64_t len)
{
    return read_write_vmo(slot, offset, user_ptr, len, READ_PMO);
}

int sys_vmo_map(u64_t target_process_slot, u64_t slot, u64_t addr, u64_t prot, u64_t flags)
{
    vmspace_t *vmspace;
    vmobject_t *vmo;
    process_t *target_process;
    int r;

    vmo = dynamic_cast(vmobject_t)(slot_get(process_self(), slot));
    if (!vmo)
    {
        r = -1;
        goto out_fail;
    }
    target_process = dynamic_cast(process_t)(slot_get(process_self(), target_process_slot));

    /* map the vmo to the target process */
    target_process = dynamic_cast(process_t)(slot_get(process_self(), target_process_slot));

    if (!target_process)
    {
        r = -1;
        goto out_fail;
    }

    vmspace = dynamic_cast(vmspace_t)(slot_get(target_process, VMSPACE_OBJ_ID));

    r = vmspace_map_range_user(vmspace, addr, vmo->size, prot, flags, vmo);
    if (r != 0)
    {
        r = -1;
        goto out_fail;
    }

    /*
	 * when a process maps a vmo to others,
	 * this func returns the new_cap in the target process.
	 */
    if (target_process != process_self())
        /* if using cap_move, we need to consider remove the mappings */
        r = slot_copy(process_self(), target_process, slot);
    else
        r = 0;

out_fail:
    return r;
}