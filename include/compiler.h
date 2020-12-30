#pragma once

#define __acquire(x) (void)0
#define __release(x) (void)0

#if (defined(__GNUC__) && (__GNUC__ >= 3))
#define likely(expr)	(__builtin_expect(!!(expr), 1))
#define unlikely(expr)	(__builtin_expect(!!(expr), 0))
#else
#define likely(expr)	(!!(expr))
#define unlikely(expr)	(!!(expr))
#endif

/* Optimization barrier */
#ifndef barrier
# define barrier() __asm__ __volatile__("": : :"memory")
#endif

/* Unreachable code */
#ifndef unreachable
# define unreachable() do { } while (1)
#endif

//  Inline Assembly Language in C Code
// RISC-V—‘config/riscv/constraints.md’
// f A floating-point register (if availiable).
// I An I-type 12-bit signed immediate.
// J Integer zero.
// K A 5-bit unsigned immediate for CSR access instructions.
// A An address that is held in a general-purpose register.