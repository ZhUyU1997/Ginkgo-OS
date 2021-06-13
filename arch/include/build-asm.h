#pragma once

#define STACK_ALIGN 16

#define __ALIGN_KERNEL_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define __ALIGN_KERNEL(x, a) __ALIGN_KERNEL_MASK(x, (typeof(x))(a)-1)
#define ALIGN(x, a) __ALIGN_KERNEL((x), (a))

#define STRING(str) asm volatile("\n.ascii \"" str "\"")
#define DEFINE(sym, val) asm volatile("\n.ascii \"#define " #sym " %0"  "\"" : : "i" (val))
#define OFFSET(sym, str, mem) DEFINE(sym, offsetof(struct str, mem))