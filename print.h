#pragma once

#include "types.h"

void print_u8(u8_t c);

static inline u8_t read8(virtual_addr_t addr)
{
    return (*((volatile u8_t *)(addr)));
}

static inline void write8(virtual_addr_t addr, u8_t value)
{
    *((volatile u8_t *)(addr)) = value;
}


static void print(const char *buf)
{
    for (int i = 0; buf[i]; i++)
    {
        print_u8(buf[i]);
    }
}

static void print_u64(u64_t i)
{
    u64_t a = i % 10;
    u64_t b = i / 10;
    if (b)
        print_u64(b);
    print_u8('0' + a);
}

static void printx_u64(u64_t i)
{
    static char c[] = "0123456789ABCDEF";
    u64_t a = i % 16;
    u64_t b = i / 16;
    if (b)
        printx_u64(b);
    print_u8(c[a]);
}