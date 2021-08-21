#pragma once

#include <core/class.h>
#include <types.h>
#include <riscv.h>
#include <list.h>
#include <barrier.h>
#include <core/kobject.h>
#include <core/vmspace.h>
#include <core/ipc.h>
#include <core/timer.h>
#include <thread_info.h>

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

// Saved registers for kernel context switches.
struct context
{
	unsigned long ra;
	unsigned long sp;

	// callee-saved
	unsigned long s0;
	unsigned long s1;
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
	struct list_head wake_list;

	process_t *process;
	vmspace_t *vmspace;

	enum task_status_t status;
	vaddr_t kstack;
	size_t sz;
	struct context context;
	const char *name;

	ipc_connection_t *active_conn;
	struct server_ipc_config *server_ipc_config;
	timer_t sleep_timer;
};

typedef void (*task_func_t)();

#define process_self() (thread_self()->process)

static inline void thread_self_set_state(enum task_status_t status)
{
	thread_self()->status = status;
	//TODO: Linux 有特别的说明，没法理解，姑且在这个加个memory barrier,thread_resume逻辑还需recheck. 具体参考：Linux： try_to_wake_up
	smp_mb();
}

thread_t *thread_self(void);
thread_t *thread_self_set(thread_t *thread);
thread_t *thread_create(process_t *process, void *stack, void *pc, void *arg);
void thread_resume(thread_t *);
void thread_suspend(thread_t *);
void thread_destroy(thread_t *);
void thread_exit(thread_t *);
void thread_need_sched(thread_t *);
void thread_clear_sched_flag(thread_t *);

int slot_alloc(process_t *process);
kobject_t *slot_get(process_t *process, int slot);

void _slot_install(process_t *process, int slot_id, kobject_t *obj);
int _slot_alloc_install(process_t *process, kobject_t *obj);

#define slot_install(_1, _2, _3) _slot_install(_1, _2, dynamic_cast(kobject_t)(_3))
#define slot_alloc_install(_1, _2) _slot_alloc_install(_1, dynamic_cast(kobject_t)(_2))

int slot_copy(process_t *src, process_t *dest, int slot);

void do_task_init();
void do_user_init();