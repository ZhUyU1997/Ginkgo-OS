#pragma once

#include "types.h"

static inline u8_t read8(vaddr_t addr)
{
    return (*((volatile u8_t *)(addr)));
}

static inline void write8(vaddr_t addr, u8_t value)
{
    *((volatile u8_t *)(addr)) = value;
}

static inline u32_t read32(vaddr_t addr)
{
    return (*((volatile u32_t *)(addr)));
}

static inline void write32(vaddr_t addr, u32_t value)
{
    *((volatile u32_t *)(addr)) = value;
}

static inline u64_t read64(vaddr_t addr)
{
    return (*((volatile u64_t *)(addr)));
}

static inline void write64(vaddr_t addr, u64_t value)
{
    *((volatile u64_t *)(addr)) = value;
}
