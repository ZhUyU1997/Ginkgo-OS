#include <core/semaphore.h>
#include <task.h>
#include <list.h>
#include <log.h>

void semaphore_init(struct semaphore_t *m, unsigned int count)
{
	atomic_set(&m->atomic, count);
	init_list_head(&m->mwait);
	spin_lock_init(&m->lock);
}

void semaphore_down(struct semaphore_t *m)
{
	thread_t *self;

	while (1)
	{
		spin_lock(&m->lock);
		if (atomic_get(&m->atomic) > 0)
		{
			atomic_dec(&m->atomic);
			spin_unlock(&m->lock);
			break;
		}

		self = task_self();
		if (list_empty_careful(&self->mlist))
			list_add_tail(&self->mlist, &m->mwait);
		spin_unlock(&m->lock);
		thread_suspend(self);
	}
}

void semaphore_up(struct semaphore_t *m)
{
	thread_t *pos, *n;

	spin_lock(&m->lock);

	atomic_inc(&m->atomic);

	if (atomic_get(&m->atomic) > 0)
	{
		if (!list_empty(&m->mwait))
		{
			list_for_each_entry_safe(pos, n, &m->mwait, mlist)
			{
				list_del_init(&pos->mlist);
				thread_resume(pos);
				break;
			}
		}
	}
	spin_unlock(&m->lock);
}