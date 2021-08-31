#include <types.h>
#include <task.h>
#include <jhash.h>
#include <assert.h>
#include <atomic.h>
#include <spinlock.h>
#include <list.h>
#include <log2.h>
#include <malloc.h>
#include <core/sched.h>

#define FUTEX_BITSET_MATCH_ANY 0xffffffff

union futex_key
{
    struct
    {
        vmspace_t *vmspace;
        unsigned long address;
    };
};
#define FUTEX_KEY_INIT \
    (union futex_key) { .vmspace = NULL, .address = 0 }

struct futex_q
{
    struct hlist_node list;
    thread_t *task;
    spinlock_t *lock_ptr;
    union futex_key key;
    u32_t bitset;
};

static const struct futex_q futex_q_init = {
    /* list gets initialized in queue_me()*/
    .key = FUTEX_KEY_INIT,
    .bitset = FUTEX_BITSET_MATCH_ANY,
};

struct futex_hash_bucket
{
    atomic_t waiters;
    spinlock_t lock;
    struct hlist_head chain;
};

#define __aligned(x) __attribute__((__aligned__(x)))

static struct
{
    struct futex_hash_bucket *queues;
    unsigned long hashsize;
} __futex_data __aligned(2 * sizeof(long));

#define futex_queues (__futex_data.queues)
#define futex_hashsize (__futex_data.hashsize)

static_assert((sizeof(union futex_key) % 4) == 0, "(sizeof(struct futex_key) % 4) != 0");

static struct futex_hash_bucket *hash_futex(union futex_key *key)
{
    u32_t hash = jhash2((u32_t *)key, sizeof(union futex_key) / 4, 0);
    return &futex_queues[hash & (futex_hashsize - 1)];
}

static inline int match_futex(union futex_key *key1, union futex_key *key2)
{
    return (key1 && key2 && key1->address == key2->address && key1->vmspace == key2->vmspace);
}

static int get_futex_key(u32_t *uaddr, union futex_key *key)
{
    key->vmspace = thread_self()->vmspace;
    key->address = (unsigned long)uaddr;
    return 0;
}

static inline void hb_waiters_inc(struct futex_hash_bucket *hb)
{
    atomic_inc(&hb->waiters);
    barrier();
}

static inline void hb_waiters_dec(struct futex_hash_bucket *hb)
{
    atomic_dec(&hb->waiters);
}

static inline int hb_waiters_pending(struct futex_hash_bucket *hb)
{
    return atomic_get(&hb->waiters);
}

/* The key must be already stored in q->key. */
static inline struct futex_hash_bucket *queue_lock(struct futex_q *q)
{
    struct futex_hash_bucket *hb;

    hb = hash_futex(&q->key);

    /*
	 * Increment the counter before taking the lock so that
	 * a potential waker won't miss a to-be-slept task that is
	 * waiting for the spinlock. This is safe as all queue_lock()
	 * users end up calling queue_me(). Similarly, for housekeeping,
	 * decrement the counter at queue_unlock() when some error has
	 * occurred and we don't end up adding the task to the list.
	 */
    hb_waiters_inc(hb); /* implies smp_mb(); (A) */

    q->lock_ptr = &hb->lock;

    spin_lock(&hb->lock);
    return hb;
}

static inline void queue_unlock(struct futex_hash_bucket *hb)
{
    spin_unlock(&hb->lock);
    hb_waiters_dec(hb);
}

static int futex_wait_setup(u32_t *uaddr, u32_t val,
                            struct futex_q *q, struct futex_hash_bucket **hb)
{
    u32_t uval;
    int ret;

    ret = get_futex_key(uaddr, &q->key);

    uval = *uaddr; //TODO: copy from user
    *hb = queue_lock(q);
    LOGI($(uval) $(val));

    if (uval != val)
    {
        queue_unlock(*hb);
        ret = -1;
    }
    return ret;
}

static inline void queue_me(struct futex_q *q, struct futex_hash_bucket *hb)
{
    init_hlist_node(&q->list);
    hlist_add_head(&q->list, &hb->chain);
    q->task = thread_self();
    spin_unlock(&hb->lock);
}

static void futex_wait_queue_me(struct futex_hash_bucket *hb, struct futex_q *q)
{
    /*
	 * The task state is guaranteed to be set before another task can
	 * wake it. set_current_state() is implemented using smp_store_mb() and
	 * queue_me() calls spin_unlock() upon completion, both serializing
	 * access to the hash list and forcing another memory barrier.
	 */
    thread_self_set_state(TASK_STATUS_SUSPEND);
    queue_me(q, hb);

    /*
	 * If we have been removed from the hash list, then another task
	 * has tried to wake us, and we can skip the call to schedule().
	 */
    if (likely(!hlist_unhashed(&q->list)))
    {
        schedule();
    }
    thread_self_set_state(TASK_STATUS_RUNNING);
}

int sys_futex_wait(u32_t *uaddr, u32_t val)
{
    struct futex_hash_bucket *hb;
    struct futex_q q = futex_q_init;
    int ret;

    q.bitset = FUTEX_BITSET_MATCH_ANY;
    /*
	 * Prepare to wait on uaddr. On success, holds hb lock and increments
	 * q.key refs.
	 */
    ret = futex_wait_setup(uaddr, val, &q, &hb);
    if (ret)
        goto out;

    /* queue_me and wait for wakeup, timeout, or a signal. */
    futex_wait_queue_me(hb, &q);

    /* If we were woken (and unqueued), we succeeded, whatever. */
    ret = 0;
out:
    return ret;
}

static void unqueue_futex(struct futex_q *q)
{
    struct futex_hash_bucket *hb;

    if ((!q->lock_ptr) || (hlist_unhashed(&q->list)))
        return;

    hb = container_of(q->lock_ptr, struct futex_hash_bucket, lock);
    hlist_del(&q->list);
    hb_waiters_dec(hb);
}

static void mark_wake_futex(struct list_head *wake_q, struct futex_q *q)
{
    thread_t *p = q->task;
    unqueue_futex(q);
    q->lock_ptr = NULL;
    smp_mb();
    list_add_tail(&p->wake_list, wake_q);
}

int sys_futex_wake(u32_t *uaddr)
{
    struct futex_hash_bucket *hb;
    struct futex_q *this;
    struct hlist_node *next;

    union futex_key key = FUTEX_KEY_INIT;
    int ret;
    u32_t bitset = FUTEX_BITSET_MATCH_ANY;
    LIST_HEAD(wake_list);

    ret = get_futex_key(uaddr, &key);
    if (unlikely(ret != 0))
        goto out;

    hb = hash_futex(&key);
    /* Make sure we really have tasks to wakeup */
    if (!hb_waiters_pending(hb))
        goto out;
    spin_lock(&hb->lock);

    hlist_for_each_entry_safe(this, next, &hb->chain, list)
    {
        if (match_futex(&this->key, &key))
        {
            /* Check if one of the bits is set in both bitsets */
            if (!(this->bitset & bitset))
                continue;
            mark_wake_futex(&wake_list, this);
        }
    }

    spin_unlock(&hb->lock);

    thread_t *pos, *n;
    list_for_each_entry_safe(pos, n, &wake_list, wake_list)
    {
        list_del_init(&pos->wake_list);
        thread_resume(pos);
    }
out:
    return ret;
}

void do_futex_init()
{
    futex_hashsize = roundup_pow_of_two(256 * CONFIG_CPU);
    futex_queues = malloc(sizeof(struct futex_hash_bucket) * futex_hashsize);

    for (int i = 0; i < futex_hashsize; i++)
    {
        atomic_set(&futex_queues[i].waiters, 0);
        init_hlist_head(&futex_queues[i].chain);
        spin_lock_init(&futex_queues[i].lock);
    }
}