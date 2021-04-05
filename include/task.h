#pragma once

#include "types.h"
#include "riscv.h"
#include "list.h"

// Saved registers for kernel context switches.
struct context
{
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

enum task_status_t
{
  TASK_STATUS_RUNNING = 0,
  TASK_STATUS_READY = 1,
  TASK_STATUS_SUSPEND = 2,
};

typedef struct
{
  struct list_head list;
  struct list_head mlist;

  int id; // Process ID
  enum task_status_t status;
  virtual_addr_t kstack;
  size_t sz;
  pagetable_t pagetable;
  struct context context;
  const char *name;
} task_t;

typedef void (*task_func_t)();

extern task_t *current;

static inline struct task_t *task_self(void)
{
  return current;
}

void switch_context(struct context *, struct context *);
void schedule();
void task_init();
task_t *task_create(const char *name, task_func_t func);
void task_resume(task_t *task);
void task_suspend(task_t *task);