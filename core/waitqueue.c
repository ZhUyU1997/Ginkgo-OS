
#include <list.h>
#include <spinlock.h>
#include <task.h>
#include <core/waitqueue.h>
#include <log.h>

void init_waitqueue_head(struct wait_queue_head *wq_head)
{
	spin_lock_init(&wq_head->lock);
	init_list_head(&wq_head->head);
}

void init_wait_entry(struct wait_queue_entry *wq_entry)
{
	wq_entry->priv = current;
	wq_entry->func = autoremove_wake_function;
	init_list_head(&wq_entry->entry);
}

int default_wake_function(wait_queue_entry_t *curr)
{
	thread_t *task = curr->priv;
	thread_resume(task);
	return 1;
}

int autoremove_wake_function(struct wait_queue_entry *wq_entry)
{
	int ret = default_wake_function(wq_entry);

	if (ret)
		list_del_init(&wq_entry->entry);

	return ret;
}

void prepare_to_wait_queue(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry)
{
	irq_flags_t flags;
	spin_lock_irqsave(&wq_head->lock, flags);
	if (list_empty(&wq_entry->entry))
	{
		list_add_tail(&wq_entry->entry, &wq_head->head);
	}
	spin_unlock_irqrestore(&wq_head->lock, flags);
}

void add_wait_queue(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry)
{
	irq_flags_t flags;
	spin_lock_irqsave(&wq_head->lock, flags);
	list_add_tail(&wq_entry->entry, &wq_head->head);
	spin_unlock_irqrestore(&wq_head->lock, flags);
}

void remove_wait_queue(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry)
{
	irq_flags_t flags;
	spin_lock_irqsave(&wq_head->lock, flags);
	list_del_init(&wq_entry->entry);
	spin_unlock_irqrestore(&wq_head->lock, flags);
}

void wake_up(struct wait_queue_head *wq_head)
{
	wait_queue_entry_t *curr, *next;

	curr = list_first_entry(&wq_head->head, wait_queue_entry_t, entry);

	if (&curr->entry == &wq_head->head)
		return;

	list_for_each_entry_safe_from(curr, next, &wq_head->head, entry)
	{
		int ret = curr->func(curr);
		if (ret < 0)
			break;
	}

	return;
}