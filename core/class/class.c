#include <core/class.h>
#include <assert.h>
#include <string.h>
#include <log.h>
#include <malloc.h>

#define LOG(fmt, arg...) printf(fmt, ##args)

#define get_head_info_from_obj(obj) ((struct class_head_info *)obj - 1)
#define get_parent_obj_from_head_info(info) ((char *)info - __class_table_start[__class_table_start[info->type].parent_type].size)

struct class_head_info
{
	type_index type;
	type_index child;
	// Not used for now, save memory
	// unsigned int size;
	// type_index parent;
};

extern type_index __class_type_start[];
extern type_index __class_type_end[];

extern struct class_table_info __class_table_start[];
extern struct class_table_info __class_table_end[];

class_impl(object_t){};

char *get_class_name(void *obj)
{
	if (obj)
	{
		struct class_head_info *info = obj - sizeof(struct class_head_info);
		return __class_table_start[info->type].name;
	}
	return "unknown";
}

type_index get_class_type(void *obj)
{
	assert(obj);

	if (obj)
	{
		struct class_head_info *info = get_head_info_from_obj(obj);
		return __class_table_start[info->type].type;
	}
	return 0xffffffff;
}

char *__class_name(type_index type)
{
	return __class_table_start[type].name;
}

void show_class_structure(void *obj)
{
	if (obj)
	{
		struct class_head_info *info = get_head_info_from_obj(obj);
		if (info->type == class_type(object_t))
		{
			LOGI($(__class_table_start[info->type].name));
			return;
		}
		else
			LOGI($(__class_table_start[info->type].name));
		//printf("(%llX %d %llX)",info, table[table[info->type].parent].size, (char *)info - table[table[info->type].parent].size);
		show_class_structure(get_parent_obj_from_head_info(info));
	}
	else
		LOGE("unknown");
}

void object_init(type_index type, type_index child, void *obj)
{
	struct class_head_info *info = get_head_info_from_obj(obj);
	info->type = type;
	info->child = child;
	//printf("type: %d data:%X size:%d info:%X\n",type,data,table[type].size,info);
	if (type == class_type(object_t))
	{
		//printf("%d",type);
		return;
	}
	else
	{
		//printf("%d",type);
	}
	object_init(__class_table_start[type].parent_type, type, get_parent_obj_from_head_info(info));
	//printf("memcpy %llX %llX %d\n",obj + sizeof(struct class_info),table[type].init_obj, table[type].size);
	memcpy(obj, __class_table_start[type].init_obj, __class_table_start[type].size);

	if (__class_table_start[type].pointer_constructor)
	{
		__class_table_start[type].pointer_constructor(obj);
	}
}

char *new_class_object(type_index type)
{
	char *base = calloc(1, __class_table_start[type].full_size);
	//printf("fullsize:%d obj=%X\n",table[type].full_size,obj);
	if (base)
	{
		char *obj = base + __class_table_start[type].full_size - __class_table_start[type].size;
		object_init(type, type, obj);
		return obj;
	}
	return NULL;
}

static void object_exit(void *obj)
{
	struct class_head_info *info = get_head_info_from_obj(obj);
	type_index type = info->type;

	if (__class_table_start[type].pointer_destructor)
	{
		__class_table_start[type].pointer_destructor(obj);
	}

	if (type == class_type(object_t))
		return;

	object_exit(get_parent_obj_from_head_info(info));
}

void delete_class_object(void *obj)
{
	assert(obj);
	object_exit(obj);

	type_index type = get_class_type(obj);

	char *base = (char *)obj - __class_table_start[type].full_size + __class_table_start[type].size;
	free(base);
}

void *class_cast(type_index type, void *obj)
{
	//printf("type = %d\n",type);
	for (;;)
	{
		struct class_head_info *info = get_head_info_from_obj(obj);
		if (info->type == type)
		{
			return obj;
		}
		else if (info->type == info->child)
		{
			break;
		}
		obj = (char *)obj + __class_table_start[info->type].size + sizeof(struct class_head_info);
	}

	for (;;)
	{
		struct class_head_info *info = get_head_info_from_obj(obj);
		if (info->type == type)
		{
			return obj;
		}
		else if (info->type == class_type(object_t))
		{
			return NULL;
		}
		obj = (char *)info - __class_table_start[__class_table_start[info->type].parent_type].size;
	}
}

static unsigned int set_class_table_info(struct class_table_info *info)
{
	unsigned int size = 0;
	if (info->type != object_t_class_type)
		size = set_class_table_info(&__class_table_start[info->parent_type]);
	info->full_size = size + sizeof(struct class_head_info) + info->size;
	return info->full_size;
}

void do_init_class()
{
	struct class_table_info *info;
	type_index type;

	info = &(*__class_table_start);
	type = 0;
	while (info < &(*__class_table_end))
	{
		info->type = type;
		info->pointer_type[0] = type;
		type++;
		info++;
	}

	info = &(*__class_table_start);
	while (info < &(*__class_table_end))
	{
		info->parent_type = info->pointer_parent_type[0];
		info++;
	}

	info = &(*__class_table_start);
	while (info < &(*__class_table_end))
	{
		set_class_table_info(info);
		//LOG("class: %s type: %d parent_type: %d size: %d full_size: %d", info->name, info->type, info->parent_type, info->size, info->full_size);
		info++;
	}
}