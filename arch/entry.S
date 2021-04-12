#include <asm.h>
#include <asm-offsets.h>
#include <csr.h>
#include <syscall.h>

.align 4
.globl handle_exception
handle_exception:    
        # sscratch points to where the process's p->trapframe is
        # mapped into user space, at TRAPFRAME.
        #
        
	# swap a0 and sscratch
        # so that a0 is TRAPFRAME
	csrrw tp, CSR_SSCRATCH, tp
	bnez tp, _save_context

_restore_kernel_tpsp:
        csrr tp, CSR_SSCRATCH
	REG_S sp, TASK_TI_KERNEL_SP(tp)
_save_context:
	REG_S sp, TASK_TI_USER_SP(tp)
	REG_L sp, TASK_TI_KERNEL_SP(tp)
	addi sp, sp, -(PT_SIZE_ON_STACK)
	REG_S x1,  PT_RA(sp)
	REG_S x3,  PT_GP(sp)
	REG_S x5,  PT_T0(sp)
	REG_S x6,  PT_T1(sp)
	REG_S x7,  PT_T2(sp)
	REG_S x8,  PT_S0(sp)
	REG_S x9,  PT_S1(sp)
	REG_S x10, PT_A0(sp)
	REG_S x11, PT_A1(sp)
	REG_S x12, PT_A2(sp)
	REG_S x13, PT_A3(sp)
	REG_S x14, PT_A4(sp)
	REG_S x15, PT_A5(sp)
	REG_S x16, PT_A6(sp)
	REG_S x17, PT_A7(sp)
	REG_S x18, PT_S2(sp)
	REG_S x19, PT_S3(sp)
	REG_S x20, PT_S4(sp)
	REG_S x21, PT_S5(sp)
	REG_S x22, PT_S6(sp)
	REG_S x23, PT_S7(sp)
	REG_S x24, PT_S8(sp)
	REG_S x25, PT_S9(sp)
	REG_S x26, PT_S10(sp)
	REG_S x27, PT_S11(sp)
	REG_S x28, PT_T3(sp)
	REG_S x29, PT_T4(sp)
	REG_S x30, PT_T5(sp)
	REG_S x31, PT_T6(sp)

	REG_L s0, TASK_TI_USER_SP(tp)
	csrrc s1, CSR_SSTATUS, t0
	csrr s2, CSR_SEPC
	csrr s3, CSR_STVAL
	csrr s4, CSR_SCAUSE
	csrr s5, CSR_SSCRATCH
	REG_S s0, PT_SP(sp)
	REG_S s1, PT_SSTATUS(sp)
	REG_S s2, PT_SEPC(sp)
	REG_S s3, PT_SBADADDR(sp)
	REG_S s4, PT_SCAUSE(sp)
	REG_S s5, PT_TP(sp)

	/*
	 * Set sscratch register to 0, so that if a recursive exception
	 * occurs, the exception vector knows it came from the kernel
	 */
	csrw CSR_SSCRATCH, x0

	la ra, ret_from_exception

	/* Handle syscalls */
	li t0, 8
	beq s4, t0, handle_syscall

	/* Handle interrupts */
	move a0, sp /* pt_regs */
	tail do_IRQ

handle_syscall:
	/* save the initial A0 value (needed in signal handlers) */
	REG_S a0, PT_ORIG_A0(sp)
	/*
	 * Advance SEPC to avoid executing the original
	 * scall instruction on sret
	 */
	addi s2, s2, 0x4
	REG_S s2, PT_SEPC(sp)
check_syscall_nr:
	/* Check to make sure we don't jump to a bogus syscall number. */
	li t0, __NR_syscalls
	la s0, sys_ni_syscall
	/* Syscall number held in a7 */
	bgeu a7, t0, 1f
	la s0, sys_call_table
	slli t0, a7, 3
	add s0, s0, t0
	REG_L s0, 0(s0)
1:
	jalr s0

ret_from_syscall:
	/* Set user a0 to kernel a0 */
	REG_S a0, PT_A0(sp)

.globl ret_from_exception
ret_from_exception:
	/* Save unwound kernel stack pointer in thread_info */
	addi s0, sp, PT_SIZE_ON_STACK
	REG_S s0, TASK_TI_KERNEL_SP(tp)

	/*
	 * Save TP into sscratch, so we can find the kernel data structures
	 * again.
	 */
	csrw CSR_SSCRATCH, tp

	REG_L a0, PT_SSTATUS(sp)
	REG_L  a2, PT_SEPC(sp)
	REG_SC x0, a2, PT_SEPC(sp)

	csrw CSR_SSTATUS, a0
	csrw CSR_SEPC, a2

    REG_L x1,  PT_RA(sp)
	REG_L x3,  PT_GP(sp)
	REG_L x4,  PT_TP(sp)
	REG_L x5,  PT_T0(sp)
	REG_L x6,  PT_T1(sp)
	REG_L x7,  PT_T2(sp)
	REG_L x8,  PT_S0(sp)
	REG_L x9,  PT_S1(sp)
	REG_L x10, PT_A0(sp)
	REG_L x11, PT_A1(sp)
	REG_L x12, PT_A2(sp)
	REG_L x13, PT_A3(sp)
	REG_L x14, PT_A4(sp)
	REG_L x15, PT_A5(sp)
	REG_L x16, PT_A6(sp)
	REG_L x17, PT_A7(sp)
	REG_L x18, PT_S2(sp)
	REG_L x19, PT_S3(sp)
	REG_L x20, PT_S4(sp)
	REG_L x21, PT_S5(sp)
	REG_L x22, PT_S6(sp)
	REG_L x23, PT_S7(sp)
	REG_L x24, PT_S8(sp)
	REG_L x25, PT_S9(sp)
	REG_L x26, PT_S10(sp)
	REG_L x27, PT_S11(sp)
	REG_L x28, PT_T3(sp)
	REG_L x29, PT_T4(sp)
	REG_L x30, PT_T5(sp)
	REG_L x31, PT_T6(sp)

	REG_L x2,  PT_SP(sp)
    sret

.globl timer_vector
.align 4
timer_vector:
	# start.c has set up the memory that mscratch points to:
	# scratch[0,8,16] : register save area.
	# scratch[24] : address of CLINT's MTIMECMP register.
	# scratch[32] : desired interval between interrupts.

	csrrw a0, mscratch, a0
	sd a1, 0(a0)
	sd a2, 8(a0)
	sd a3, 16(a0)

	# schedule the next timer interrupt
	# by adding interval to mtimecmp.
	ld a1, 24(a0) # CLINT_MTIMECMP(hart)
	ld a2, 32(a0) # interval
	ld a3, 0(a1)
	add a3, a3, a2
	sd a3, 0(a1)

	# raise a supervisor software interrupt.
	li a1, 2
	csrw sip, a1

	ld a3, 16(a0)
	ld a2, 8(a0)
	ld a1, 0(a0)
	csrrw a0, mscratch, a0

	mret