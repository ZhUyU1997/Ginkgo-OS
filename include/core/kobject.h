#pragma once

#include <core/class.h>
#include <core/sys.h>
#include <core/errno.h>
#include <core/time.h>

#include <list.h>
#include <atomic.h>

class(kobject_t)
{
    atomic_t refcount;
};


static void ref(kobject_t *this)
{
    atomic_inc(&this->refcount);
}
static void unref(kobject_t *this)
{
    atomic_dec(&this->refcount);
}