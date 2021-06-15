#pragma once

#include <rbtree_augmented.h>
#include <clockevent/clockevent.h>
#include <core/time.h>
#include <spinlock.h>
#include <core/class.h>
#include <core/kobject.h>

struct timer_base_t;
struct timer_t;

enum timer_state_t
{
	TIMER_STATE_INACTIVE = 0,
	TIMER_STATE_ENQUEUED = 1,
	TIMER_STATE_CALLBACK = 2,
};

struct timer_base_t
{
	struct rb_root head;
	struct timer_t *next;
	struct clockevent_t *ce;
	spinlock_t lock;
};

class(timer_t, kobject_t)
{
	struct rb_node node;
	struct timer_base_t *base;
	enum timer_state_t state;
	ktime_t expires;
	void *data;
	int (*function)(struct timer_t *, void *);
};

void timer_init(struct timer_t *timer, int (*function)(struct timer_t *, void *), void *data);
void timer_start(struct timer_t *timer, ktime_t now, ktime_t interval);
void timer_start_now(struct timer_t *timer, ktime_t interval);
void timer_forward(struct timer_t *timer, ktime_t now, ktime_t interval);
void timer_forward_now(struct timer_t *timer, ktime_t interval);
void timer_cancel(struct timer_t *timer);

void timer_bind_clockevent(struct clockevent_t *ce);