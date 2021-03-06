#include <core/mutex.h>
#include <task.h>

void mutex_init(struct mutex_t *m)
{
	atomic_set(&m->atomic, 1);
	init_list_head(&m->mwait);
	spin_lock_init(&m->lock);
}

void mutex_lock(struct mutex_t *m)
{
	thread_t *self;

	while (atomic_cmpxchg(&m->atomic, 1, 0) != 1)
	{
		self = thread_self();
		spin_lock(&m->lock);
		if (list_empty_careful(&self->mlist))
			list_add_tail(&self->mlist, &m->mwait);
		spin_unlock(&m->lock);
		thread_suspend(self);
	}
}

void mutex_unlock(struct mutex_t *m)
{
	thread_t *pos, *n;

	if (atomic_cmpxchg(&m->atomic, 0, 1) == 0)
	{
		spin_lock(&m->lock);
		if (!list_empty(&m->mwait))
		{
			list_for_each_entry_safe(pos, n, &m->mwait, mlist)
			{
				list_del_init(&pos->mlist);
				thread_resume(pos);
				break;
			}
		}
		spin_unlock(&m->lock);
	}
}