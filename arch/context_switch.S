#include <asm.h>
#include <asm-offsets.h>

# Save current registers in old. Load from new.	


.globl switch_to
switch_to:
	li a4, TASK_CTX
	add a3, a0, a4
	add a4, a1, a4
        REG_S ra, 0(a3)
        REG_S sp, 8(a3)
        REG_S s0, 16(a3)
        REG_S s1, 24(a3)
        REG_S s2, 32(a3)
        REG_S s3, 40(a3)
        REG_S s4, 48(a3)
        REG_S s5, 56(a3)
        REG_S s6, 64(a3)
        REG_S s7, 72(a3)
        REG_S s8, 80(a3)
        REG_S s9, 88(a3)
        REG_S s10, 96(a3)
        REG_S s11, 104(a3)

        REG_L ra, 0(a4)
        REG_L sp, 8(a4)
        REG_L s0, 16(a4)
        REG_L s1, 24(a4)
        REG_L s2, 32(a4)
        REG_L s3, 40(a4)
        REG_L s4, 48(a4)
        REG_L s5, 56(a4)
        REG_L s6, 64(a4)
        REG_L s7, 72(a4)
        REG_L s8, 80(a4)
        REG_L s9, 88(a4)
        REG_L s10, 96(a4)
        REG_L s11, 104(a4)
        
        move tp, a1

        ret


.globl load_context
load_context:
	li a1, TASK_CTX
	add a1, a0, a1

        REG_L ra, 0(a1)
        REG_L sp, 8(a1)
        REG_L s0, 16(a1)
        REG_L s1, 24(a1)
        REG_L s2, 32(a1)
        REG_L s3, 40(a1)
        REG_L s4, 48(a1)
        REG_L s5, 56(a1)
        REG_L s6, 64(a1)
        REG_L s7, 72(a1)
        REG_L s8, 80(a1)
        REG_L s9, 88(a1)
        REG_L s10, 96(a1)
        REG_L s11, 104(a1)
        
        move tp, a0

        ret