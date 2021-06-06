#include <core/class.h>
#include <core/sys.h>
#include <core/errno.h>
#include <core/time.h>
#include <core/kobject.h>

#include <list.h>
#include <atomic.h>

class_impl(kobject_t){
    .refcount = (atomic_t){
        .counter = 0,
    },
};
