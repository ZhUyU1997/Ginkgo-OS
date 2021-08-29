#pragma once

/* Read and write memory barrier */
#define mb()		__asm__ __volatile__ ("fence iorw, iorw" : : : "memory");
/* Read memory barrier */
#define rmb()		__asm__ __volatile__ ("fence ir, ir" : : : "memory");
/* Write memory barrier */
#define wmb()		__asm__ __volatile__ ("fence ow, ow" : : : "memory");

/* SMP read and write memory barrier */
#define smp_mb()	__asm__ __volatile__ ("fence rw, rw" : : : "memory");
/* SMP read memory barrier */
#define smp_rmb()	__asm__ __volatile__ ("fence r, r" : : : "memory");
/* SMP write memory barrier */
#define smp_wmb()	__asm__ __volatile__ ("fence w, w" : : : "memory");
