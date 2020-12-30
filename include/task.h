#pragma once

#include "types.h"
#include "riscv.h"
#include "list.h"

// Saved registers for kernel context switches.
struct context {
  register_t ra;
  register_t sp;

  // callee-saved
  register_t s0;
  register_t s1;
  register_t s2;
  register_t s3;
  register_t s4;
  register_t s5;
  register_t s6;
  register_t s7;
  register_t s8;
  register_t s9;
  register_t s10;
  register_t s11;
};

typedef struct task_t {
	struct list_head list;
  int id;                     // Process ID
  virtual_addr_t kstack;
  size_t sz;
  pagetable_t pagetable;
  struct context context;
  const char *name;
}task_t;

typedef void (*task_func_t)();

extern task_t *current;

void swtch(struct context*, struct context*);
void schedule();
void task_init();
void task_create(const char *name, task_func_t func);