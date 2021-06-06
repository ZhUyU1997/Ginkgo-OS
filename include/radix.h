#pragma once

#include <types.h>


#define RADIX_NODE_BITS (9)
#define RADIX_NODE_SIZE (1 << (RADIX_NODE_BITS))
#define RADIX_NODE_MASK (RADIX_NODE_SIZE - 1)
#define RADIX_MAX_BITS (64)

struct radix_node
{
    union
    {
        struct radix_node *children[RADIX_NODE_SIZE];
        void *values[RADIX_NODE_SIZE];
    };
};

struct radix
{
    struct radix_node *root;
    void (*value_deleter)(void *);
};

/* interfaces */
struct radix *new_radix(void);
void init_radix(struct radix *radix);
void init_radix_w_deleter(struct radix *radix, void (*value_deleter)(void *));

bool_t radix_set(struct radix *radix, u64_t key, void *value);
void *radix_get(struct radix *radix, u64_t key);
bool_t radix_free(struct radix *radix);
bool_t radix_clear(struct radix *radix, u64_t key);