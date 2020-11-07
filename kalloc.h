#pragma once

void kfree(void *pa);
void freerange(void *pa_start, void *pa_end);
void kinit();
void *kalloc(void);