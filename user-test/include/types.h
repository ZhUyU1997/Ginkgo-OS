#pragma once

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long uint64;

typedef signed char sint8;
typedef signed short sint16;
typedef signed int sint32;
typedef signed long sint64;

typedef uint8 u8_t;
typedef uint16 u16_t;
typedef uint32 u32_t;
typedef uint64 u64_t;
typedef sint8 s8_t;
typedef sint16 s16_t;
typedef sint32 s32_t;
typedef sint64 s64_t;

typedef uint8 uint8_t;
typedef uint16 uint16_t;
typedef uint32 uint32_t;
typedef uint64 uint64_t;

typedef uint64 pde_t;
typedef sint64 loff_t;
typedef uint64 vaddr_t;
typedef uint64 paddr_t;
typedef uint64 addr_t;
typedef uint64 size_t;
typedef uint64 register_t;
typedef uint64 irq_flags_t;

typedef sint64 ssize_t;

#if defined(__alpha__) || defined(__ia64__) || defined(__x86_64__) || defined(_WIN64) || defined(__LP64__) || defined(__LLP64__)
typedef long int ptrdiff_t;
typedef unsigned long uintptr_t;
typedef long intptr_t;
#else
typedef int ptrdiff_t;
typedef unsigned int uintptr_t;
typedef int intptr_t;
#endif

typedef signed int bool_t;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define NULL ((void *)0)

typedef struct
{
	volatile int counter;
} atomic_t;