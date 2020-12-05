#include "list.h"
#include "memlayout.h"
#include "types.h"
#include "riscv.h"

static struct list_head *list = &(struct list_head){0};

extern char end[];

void kfree(void *pa)
{
  if (pa == NULL)
    return;
  list_add(pa, list);
}

void freerange(void *pa_start, void *pa_end)
{
  for (char *p = (char *)PGROUNDUP((uint64)pa_start);
       p + PGSIZE <= (char *)pa_end; p += PGSIZE)
    kfree(p);
}

void kinit()
{
  init_list_head(list);
  freerange(end, (void *)PHYSTOP);
}

void *kalloc(void)
{
  if (list_empty(list))
    return NULL;

  struct list_head *head = list->next;
  list_del(head);
  return head;
}
