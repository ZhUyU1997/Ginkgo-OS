#include <csr.h>

    # qemu -kernel loads the kernel at 0x80000000
    # and causes each CPU to jump there.
    # kernel.ld causes the following code to
    # be placed at 0x80000000.
.section .text
.global _entry
_entry:
	/* Mask all interrupts */
	csrw mideleg, zero
	csrw medeleg, zero
	csrw mie, zero
	csrw mip, zero

    /* Enable FPU and accelerator if present */
	li t0, MSTATUS_FS | MSTATUS_XS
	csrs mstatus, t0

    # set up a stack for C.
    # stack0 is declared in start.c,
    # with a 4096-byte stack per CPU.
    # sp = stack0 + (hartid * 4096)
    la sp, stack0
    li a0, 1024*4
    csrr a1, mhartid
    addi a1, a1, 1
    mul a0, a0, a1
    add sp, sp, a0
    # jump to start() in start.c
    call start
spin:
    j spin 
