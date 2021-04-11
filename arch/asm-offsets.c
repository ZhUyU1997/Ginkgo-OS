#include "build-asm.h"
#include "task.h"

void asm_offsets(void)
{
    OFFSET(TASK_TI_KERNEL_SP, task_t, thread_info.kernel_sp);
	OFFSET(TASK_TI_USER_SP, task_t, thread_info.user_sp);
    DEFINE(PT_SIZE, sizeof(struct pt_regs));
    OFFSET(PT_SEPC, pt_regs, sepc);
    OFFSET(PT_RA, pt_regs, ra);
    OFFSET(PT_FP, pt_regs, s0);
    OFFSET(PT_S0, pt_regs, s0);
    OFFSET(PT_S1, pt_regs, s1);
    OFFSET(PT_S2, pt_regs, s2);
    OFFSET(PT_S3, pt_regs, s3);
    OFFSET(PT_S4, pt_regs, s4);
    OFFSET(PT_S5, pt_regs, s5);
    OFFSET(PT_S6, pt_regs, s6);
    OFFSET(PT_S7, pt_regs, s7);
    OFFSET(PT_S8, pt_regs, s8);
    OFFSET(PT_S9, pt_regs, s9);
    OFFSET(PT_S10, pt_regs, s10);
    OFFSET(PT_S11, pt_regs, s11);
    OFFSET(PT_SP, pt_regs, sp);
    OFFSET(PT_TP, pt_regs, tp);
    OFFSET(PT_A0, pt_regs, a0);
    OFFSET(PT_A1, pt_regs, a1);
    OFFSET(PT_A2, pt_regs, a2);
    OFFSET(PT_A3, pt_regs, a3);
    OFFSET(PT_A4, pt_regs, a4);
    OFFSET(PT_A5, pt_regs, a5);
    OFFSET(PT_A6, pt_regs, a6);
    OFFSET(PT_A7, pt_regs, a7);
    OFFSET(PT_T0, pt_regs, t0);
    OFFSET(PT_T1, pt_regs, t1);
    OFFSET(PT_T2, pt_regs, t2);
    OFFSET(PT_T3, pt_regs, t3);
    OFFSET(PT_T4, pt_regs, t4);
    OFFSET(PT_T5, pt_regs, t5);
    OFFSET(PT_T6, pt_regs, t6);
    OFFSET(PT_GP, pt_regs, gp);
    OFFSET(PT_ORIG_A0, pt_regs, orig_a0);
    OFFSET(PT_SSTATUS, pt_regs, sstatus);
    OFFSET(PT_SBADADDR, pt_regs, sbadaddr);
    OFFSET(PT_SCAUSE, pt_regs, scause);
	DEFINE(PT_SIZE_ON_STACK, ALIGN(sizeof(struct pt_regs), STACK_ALIGN));
}