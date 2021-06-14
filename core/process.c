#include <task.h>
#include <malloc.h>
#include <log.h>
#include <assert.h>
#include <core/vmspace.h>
#include <vfs/vfs.h>

int slot_alloc(process_t *process)
{

	assert(process);

	unsigned int size = process->slot_table.slots_size;
	kobject_t **slots = process->slot_table.slots;

	for (int i = 0; i < size; i++)
	{
		if (slots[i] == NULL)
		{
			return i;
		}
	}

	return -1;
}

kobject_t *slot_get(process_t *process, int slot)
{
	unsigned int size = process->slot_table.slots_size;
	kobject_t **slots = process->slot_table.slots;

	if (size > slot)
	{
		return slots[slot];
	}
	return NULL;
}

void slot_install(process_t *process, int slot_id,
				  kobject_t *obj)
{
	assert(process && obj);

	process->slot_table.slots[slot_id] = obj;
}

int slot_alloc_install(process_t *process, kobject_t *obj)
{
	assert(process && obj);

	int slot = slot_alloc(process);

	if (slot < 0)
	{
		return -1;
	}
	slot_install(process, slot, dynamic_cast(kobject_t)(obj));
	return slot;
}

int slot_copy(process_t *src, process_t *dest, int slot)
{
	kobject_t *obj = slot_get(src, slot);
	int ret = slot_alloc_install(dest, obj);
	return ret;
}

int sys_process_create(void)
{
	process_t *new_process = new (process_t);
	vmspace_t *vmspace = new (vmspace_t);

	int slot = slot_alloc_install(process_self(), dynamic_cast(kobject_t)(new_process));
	slot_alloc_install(new_process, dynamic_cast(kobject_t)(new_process));
	slot_alloc_install(new_process, dynamic_cast(kobject_t)(vmspace));

	return slot;
}

void sys_process_exit(s64_t retcode)
{
	// send dying signal to all threads in this process

	// exit this thread.
	thread_exit(thread_self());
}

class_impl(process_t, kobject_t){};

constructor(process_t)
{
	init_list_head(&this->thread_list);
	this->slot_table.slots_size = MAX_SLOTS_SIZE;
	this->slot_table.slots = malloc(MAX_SLOTS_SIZE * sizeof(kobject_t *));
}

static u64_t load_binary(vmspace_t *vmspace, const char *file, virtual_addr_t addr)
{
	vfs_mount("virtio-block", "/", "cpio", MOUNT_RO);
	int fd = vfs_open(file, O_RDONLY, 0);
	struct vfs_stat_t st;
	vfs_fstat(fd, &st);
	int page_num = (st.st_size + PAGE_SIZE - 1) >> PAGE_SHIFT;
	void *buf = alloc_page(page_num);
	LOGI("size:" $((int)st.st_size));
	LOGI("buf:" $(buf));

	vfs_read(fd, buf, st.st_size);

	map_range_in_pgtbl(vmspace->pgtbl, addr, buf, st.st_size, PTE_R | PTE_W | PTE_X | PTE_U);
	return addr;
}

vaddr_t prepare_env(char *top)
{
	{
		char s[2][20] = {"/test", "test args"};
		top -= sizeof(s);
		memcpy(top, &s, sizeof(s));
	}
	char *s = top;

	{
		char *envp[1] = {NULL};
		top -= sizeof(envp);
		memcpy(top, &envp, sizeof(envp));
	}
	char *envp = top;

	{
		char *argv[2] = {s, s + 20};
		top -= sizeof(argv);
		memcpy(top, &argv, sizeof(argv));
	}

	char **argv = top;

	top -= sizeof(u64_t);
	*((u64_t *)top) = envp;

	top -= sizeof(u64_t);
	*((u64_t *)top) = argv;

	top -= sizeof(u64_t);
	*((u64_t *)top) = 1;
	return top;
}

void launch_user_init(const char *filename)
{
	process_t *root = new (process_t);
	vmspace_t *vmspace = new (vmspace_t);
	thread_t *current = thread_self();

	slot_alloc_install(root, dynamic_cast(kobject_t)(root));
	slot_alloc_install(root, dynamic_cast(kobject_t)(vmspace));

	u64_t pc = load_binary(vmspace, filename, 0x20000000UL);

	virtual_addr_t ustack = (virtual_addr_t)alloc_page(1);
	map_range_in_pgtbl(vmspace->pgtbl, ustack, ustack, PGSIZE, PTE_R | PTE_W | PTE_U);

	vaddr_t sp = prepare_env(ustack + PGSIZE);
	thread_t *thread = thread_create(root, sp, pc, NULL);

	struct pt_regs *regs = current->thread_info.kernel_sp - sizeof(struct pt_regs);

	thread_resume(thread);
}