#pragma once

#include "types.h"

static inline u8_t read8(virtual_addr_t addr)
{
    return (*((volatile u8_t *)(addr)));
}

static inline void write8(virtual_addr_t addr, u8_t value)
{
    *((volatile u8_t *)(addr)) = value;
}

static inline u32_t read32(virtual_addr_t addr)
{
    return (*((volatile u32_t *)(addr)));
}

static inline void write32(virtual_addr_t addr, u32_t value)
{
    *((volatile u32_t *)(addr)) = value;
}