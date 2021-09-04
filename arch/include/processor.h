#pragma once

#define CONFIG_CPU 1
#define PGSIZE 4096 // bytes per page
#define PGSHIFT 12  // bits of offset within a page
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12
#define PAGE_MASK (PGSIZE - 1)