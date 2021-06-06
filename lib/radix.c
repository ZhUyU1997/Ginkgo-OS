#include <types.h>
#include <malloc.h>
#include <log.h>
#include <assert.h>
#include <radix.h>

#define radix_assert(x, msg) assert((x) && (msg))
#define DIV_UP(a, b) (((a) + (b)-1) / (b))
#define RADIX_LEVELS (DIV_UP(RADIX_MAX_BITS, RADIX_NODE_BITS))

struct radix *new_radix(void)
{
    struct radix *radix;

    radix = calloc(1, sizeof(*radix));

    return radix;
}

void init_radix(struct radix *radix)
{
    assert(radix);
}

void init_radix_w_deleter(struct radix *radix, void (*value_deleter)(void *))
{
    assert(radix);
    radix->value_deleter = value_deleter;
}

static struct radix_node *new_radix_node(void)
{
    struct radix_node *n = calloc(1, sizeof(struct radix_node));
    return n;
}

static void radix_free_node(struct radix_node *node, int node_level, void (*value_deleter)(void *))
{
    if (!node)
    {
        LOGD("should not try to free a node pointed by NULL");
        return;
    }

    if (node_level == RADIX_LEVELS - 1)
    {
        if (value_deleter)
        {
            for (int i = 0; i < RADIX_NODE_SIZE; i++)
            {
                if (node->values[i])
                    value_deleter(node->values[i]);
            }
        }
    }
    else
    {
        for (int i = 0; i < RADIX_NODE_SIZE; i++)
        {
            if (node->children[i])
                radix_free_node(node->children[i],
                                node_level + 1, value_deleter);
        }
    }
}

bool_t radix_set(struct radix *radix, u64_t key, void *value)
{
    struct radix_node *node;
    struct radix_node *new;
    u16_t index[RADIX_LEVELS];

    int k;

    if (!radix->root)
    {
        new = new_radix_node();
        if (!new)
            return FALSE;
        radix->root = new;
    }
    node = radix->root;

    /* calculate index for each level */
    for (int i = 0; i < RADIX_LEVELS; ++i)
    {
        index[i] = key & RADIX_NODE_MASK;
        key >>= RADIX_NODE_BITS;
    }

    /* the intermediate levels */
    for (int i = RADIX_LEVELS - 1; i > 0; --i)
    {
        k = index[i];
        if (!node->children[k])
        {
            new = new_radix_node();
            if (!new)
                return FALSE;
            node->children[k] = new;
        }
        node = node->children[k];
    }

    /* the leaf level */
    k = index[0];
    node->values[k] = value;

    return TRUE;
}

void *radix_get(struct radix *radix, u64_t key)
{
    struct radix_node *node;
    u16_t index[RADIX_LEVELS];
    int k;

    if (!radix->root)
        return NULL;
    node = radix->root;

    /* calculate index for each level */
    for (int i = 0; i < RADIX_LEVELS; ++i)
    {
        index[i] = key & RADIX_NODE_MASK;
        key >>= RADIX_NODE_BITS;
    }

    /* the intermediate levels */
    for (int i = RADIX_LEVELS - 1; i > 0; --i)
    {
        k = index[i];
        if (!node->children[k])
            return NULL;
        node = node->children[k];
    }

    /* the leaf level */
    k = index[0];
    return node->values[k];
}

bool_t radix_clear(struct radix *radix, u64_t key)
{
    return radix_set(radix, key, NULL);
}

bool_t radix_free(struct radix *radix)
{
    if (!radix || !radix->root)
    {
        LOGD("trying to free an empty radix tree");
        return FALSE;
    }
    // recurssively free nodes and values (if value_deleter is not NULL)
    radix_free_node(radix->root, 0, radix->value_deleter);

    return TRUE;
}
