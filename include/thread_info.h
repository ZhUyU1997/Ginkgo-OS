#pragma once

#define NEED_RESCHED 1

struct thread_info
{
    unsigned long flags; /* low level flags */
    int preempt_count;   /* 0=>preemptible, <0=>BUG */
    /*
	 * These stack pointers are overwritten on every system call or
	 * exception.  SP is also saved to the stack it can be recovered when
	 * overwritten.
	 */
    long kernel_sp; /* Kernel stack pointer */
    long user_sp;   /* User stack pointer */
    int cpu;
};