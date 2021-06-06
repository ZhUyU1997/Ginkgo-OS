#pragma once

#include <core/class.h>
#include <types.h>
#include <riscv.h>
#include <list.h>
#include <core/kobject.h>
#include <core/vmspace.h>

struct pt_regs
{
	unsigned long sepc;
	unsigned long ra;
	unsigned long sp;
	unsigned long gp;
	unsigned long tp;
	unsigned long t0;
	unsigned long t1;
	unsigned long t2;
	unsigned long s0;
	unsigned long s1;
	unsigned long a0;
	unsigned long a1;
	unsigned long a2;
	unsigned long a3;
	unsigned long a4;
	unsigned long a5;
	unsigned long a6;
	unsigned long a7;
	unsigned long s2;
	unsigned long s3;
	unsigned long s4;
	unsigned long s5;
	unsigned long s6;
	unsigned long s7;
	unsigned long s8;
	unsigned long s9;
	unsigned long s10;
	unsigned long s11;
	unsigned long t3;
	unsigned long t4;
	unsigned long t5;
	unsigned long t6;
	unsigned long sstatus;
	unsigned long sbadaddr;
	unsigned long scause;
	unsigned long orig_a0; /* a0 value before the syscall */
};

struct thread_info
{
	unsigned long flags; /* low level flags */
	int preempt_count;	 /* 0=>preemptible, <0=>BUG */
	/*
	 * These stack pointers are overwritten on every system call or
	 * exception.  SP is also saved to the stack it can be recovered when
	 * overwritten.
	 */
	long kernel_sp; /* Kernel stack pointer */
	long user_sp;	/* User stack pointer */
	int cpu;
};

// Saved registers for kernel context switches.
struct context
{
	register_t ra;
	register_t sp;

	// callee-saved
	register_t s0;
	register_t s1;
	register_t s2;
	register_t s3;
	register_t s4;
	register_t s5;
	register_t s6;
	register_t s7;
	register_t s8;
	register_t s9;
	register_t s10;
	register_t s11;
};

enum task_status_t
{
	TASK_STATUS_RUNNING = 0,
	TASK_STATUS_READY = 1,
	TASK_STATUS_SUSPEND = 2,
	TASK_STATUS_EXIT = 3,
};

#define MAX_SLOTS_SIZE 64

#define PROCESS_OBJ_ID 0
#define VMSPACE_OBJ_ID 1

struct slot_table
{
	unsigned int slots_size;
	kobject_t **slots;
};

class(process_t, kobject_t)
{
	struct list_head thread_list;
	struct slot_table slot_table;
};

class(thread_t, kobject_t)
{
	struct thread_info thread_info;
	struct list_head node;
	struct list_head ready_queue_node;
	struct list_head mlist;

	process_t *process;
	vmspace_t *vmspace;

	enum task_status_t status;
	virtual_addr_t kstack;
	size_t sz;
	struct context context;
	const char *name;
};

typedef void (*task_func_t)();

extern thread_t *current;

#define current_process (current->process)

static inline thread_t *task_self(void)
{
	return current;
}

void switch_to(thread_t *, thread_t *);
void load_context(thread_t *);
void schedule();
void do_task_init();
void launch_user_init(const char *filename);
thread_t *thread_create(process_t *process, u64_t stack, u64_t pc, u64_t arg);
void thread_resume(thread_t *);
void thread_suspend(thread_t *);
void thread_destroy(thread_t *);

int slot_alloc(process_t *process);
kobject_t *slot_get(process_t *process, int slot);
void slot_install(process_t *process, int slot_id, kobject_t *obj);
int slot_alloc_install(process_t *process, kobject_t *obj);
int slot_copy(process_t *src, process_t *dest, int slot);