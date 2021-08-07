/*
 * libx/queue.c
 */

#include <malloc.h>
#include <queue.h>

struct queue_t * queue_alloc(void)
{
	struct queue_t * q;

	q = malloc(sizeof(struct queue_t));
	if(!q)
		return NULL;

	init_list_head(&q->node.entry);
	q->available = 0;
	return q;
}

void queue_free(struct queue_t * q, void (*cb)(struct queue_node_t *))
{
	if(q)
	{
		queue_clear(q, cb);
		free(q);
	}
}

void queue_clear(struct queue_t * q, void (*cb)(struct queue_node_t *))
{
	struct queue_node_t * pos, * n;

	if(!q)
		return;

	list_for_each_entry_safe(pos, n, &(q->node.entry), entry)
	{
		list_del(&(pos->entry));
		if(cb)
			cb(pos);
		free(pos);
	}
	q->available = 0;
}

int queue_avail(struct queue_t * q)
{
	int ret = 0;

	if(q)
	{
		ret = q->available;
	}
	return ret;
}

void queue_push(struct queue_t * q, void * data)
{
	struct queue_node_t * node;

	if(!q || !data)
		return;

	node = malloc(sizeof(struct queue_node_t));
	if(!node)
		return;

	node->data = data;
	list_add_tail(&(node->entry), &(q->node.entry));
	q->available++;
}

void * queue_pop(struct queue_t * q)
{
	irq_flags_t flags;
	void * data = NULL;

	if(!q)
		return NULL;

	if(!list_empty(&(q->node.entry)))
	{
		struct list_head * pos = (&q->node.entry)->next;
		struct queue_node_t * node = list_entry(pos, struct queue_node_t, entry);
		data = node->data;
		list_del(pos);
		free(node);
		q->available--;
	}

	return data;
}

void * queue_peek(struct queue_t * q)
{
	irq_flags_t flags;
	void * data = NULL;

	if(!q)
		return NULL;

	if(!list_empty(&(q->node.entry)))
	{
		struct list_head * pos = (&q->node.entry)->next;
		struct queue_node_t * node = list_entry(pos, struct queue_node_t, entry);
		data = node->data;
	}

	return data;
}
