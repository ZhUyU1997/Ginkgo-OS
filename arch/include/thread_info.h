#pragma once

#include <stdint.h>
#include <processor.h>

#define NEED_RESCHED 1

#define THREAD_SIZE_ORDER (1)
#define THREAD_SIZE (PAGE_SIZE << THREAD_SIZE_ORDER)

extern __attribute__((aligned(16))) char stack0[CONFIG_CPU][THREAD_SIZE];

#define STACK_ALIGN 16

#define __ALIGN_KERNEL_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define __ALIGN_KERNEL(x, a) __ALIGN_KERNEL_MASK(x, (typeof(x))(a)-1)
#define ALIGN(x, a) __ALIGN_KERNEL((x), (a))

#define task_stack_page(tsk) (tsk->kstack)
#define task_pt_regs(tsk) ((struct pt_regs *)(task_stack_page(tsk) + THREAD_SIZE - ALIGN(sizeof(struct pt_regs), STACK_ALIGN)))

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

struct __riscv_d_ext_state
{
	uint64_t f[32];
	uint32_t fcsr;
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
	struct pt_regs *regs;
	struct __riscv_d_ext_state fstate;
};