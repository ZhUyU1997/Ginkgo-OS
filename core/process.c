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

void _slot_install(process_t *process, int slot_id, kobject_t *obj)
{
	assert(process && obj);

	process->slot_table.slots[slot_id] = obj;
}

int _slot_alloc_install(process_t *process, kobject_t *obj)
{
	assert(process && obj);

	int slot = slot_alloc(process);

	if (slot < 0)
	{
		return -1;
	}
	slot_install(process, slot, obj);
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

	int slot = slot_alloc_install(process_self(), new_process);
	slot_alloc_install(new_process, new_process);
	slot_alloc_install(new_process, vmspace);

	return slot;
}

void sys_process_exit(s64_t retcode)
{
	LOGI();
	// send dying signal to all threads in this process

	// exit this thread.
	thread_exit(thread_self());
}

class_impl(process_t, kobject_t){};

constructor(process_t)
{
	init_list_head(&this->thread_list);
	this->slot_table.slots_size = MAX_SLOTS_SIZE;
	this->slot_table.slots = calloc(1, MAX_SLOTS_SIZE * sizeof(kobject_t *));
}

static paddr_t load_binary(vmspace_t *vmspace, const char *file, vaddr_t addr)
{
	int fd = vfs_open(file, O_RDONLY, 0);

	if (fd < 0)
		return 0;

	struct vfs_stat_t st;
	vfs_fstat(fd, &st);
	int page_num = (st.st_size + PAGE_SIZE - 1) >> PAGE_SHIFT;
	void *buf = alloc_page(page_num);
	LOGI("size:" $((int)st.st_size));
	LOGI("buf:" $(buf));

	vfs_read(fd, buf, st.st_size);

	map_range_in_pgtbl(vmspace->pgtbl, addr, (paddr_t)buf, page_num * PAGE_SIZE, PTE_R | PTE_W | PTE_X | PTE_U);
    LOGD("User Space: [" $(addr) " ~ " $(addr + page_num * PAGE_SIZE - 1) "]");

	return (paddr_t)addr;
}

vaddr_t prepare_env(vaddr_t top)
{
	{
		char s[2][20] = {"/test", "test args"};
		top -= sizeof(s);
		memcpy((void *)top, &s, sizeof(s));
	}
	char *s = (char *)top;

	{
		char *envp[1] = {NULL};
		top -= sizeof(envp);
		memcpy((void *)top, &envp, sizeof(envp));
	}
	char *envp = (char *)top;

	{
		char *argv[2] = {s, s + 20};
		top -= sizeof(argv);
		memcpy((void *)top, &argv, sizeof(argv));
	}

	char **argv = (char **)top;

	top -= sizeof(u64_t);
	*((u64_t *)top) = (u64_t)envp;

	top -= sizeof(u64_t);
	*((u64_t *)top) = (u64_t)argv;

	top -= sizeof(u64_t);
	*((u64_t *)top) = 1;
	return top;
}

process_t *launch_user_init(process_t *parent, const char *filename)
{
	LOGD("launch ["$(filename)"]");

	process_t *process = new (process_t);
	vmspace_t *vmspace = new (vmspace_t);
	thread_t *current = thread_self();

	slot_alloc_install(parent, process);

	slot_alloc_install(process, process);
	slot_alloc_install(process, vmspace);

	paddr_t pc = load_binary(vmspace, filename, 0x20000000UL);

	if (!pc)
	{
		LOGE("Failed to load " $(filename));
		return NULL;
	}

	int pagecount = 16;
	vaddr_t ustack = (vaddr_t)alloc_page(pagecount);
	map_range_in_pgtbl(vmspace->pgtbl, ustack, ustack, PGSIZE * pagecount, PTE_R | PTE_W | PTE_U);

	vaddr_t sp = prepare_env(ustack + PGSIZE * pagecount);
	thread_t *thread = thread_create(process, (void *)sp, (void *)pc, NULL);

	thread_resume(thread);

	return process;
}

static process_t *root;
extern void do_user_block_init(const char *dev);

void do_user_init()
{
	root = new (process_t);
	vmspace_t *vmspace = new (vmspace_t);
	slot_alloc_install(root, root);
	slot_alloc_install(root, vmspace);

	do_user_block_init("virtio-block");

	vfs_mount("virtio-block", "/", "cpio", MOUNT_RO);
	launch_user_init(root, "/idle");
	launch_user_init(root, "/vfs_server");
	launch_user_init(root, "/test");
	launch_user_init(root, "/wm");
}
