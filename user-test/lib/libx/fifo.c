/*
 * libx/fifo.c
 */

#include <stddef.h>
#include <log2.h>
#include <string.h>
#include <malloc.h>
#include <fifo.h>
#include <sizes.h>
#include <defines.h>

void __fifo_reset(struct fifo_t * f)
{
	f->in = f->out = 0;
}

unsigned int __fifo_len(struct fifo_t * f)
{
	return f->in - f->out;
}

unsigned int __fifo_put(struct fifo_t * f, unsigned char * buf, unsigned int len)
{
	unsigned int l;

	len = min(len, f->size - f->in + f->out);
	l = min(len, f->size - (f->in & (f->size - 1)));
	memcpy(f->buffer + (f->in & (f->size - 1)), buf, l);
	memcpy(f->buffer, buf + l, len - l);
	f->in += len;

	return len;
}

unsigned int __fifo_get(struct fifo_t * f, unsigned char * buf, unsigned int len)
{
	unsigned int l;

	len = min(len, f->in - f->out);
	l = min(len, f->size - (f->out & (f->size - 1)));
	memcpy(buf, f->buffer + (f->out & (f->size - 1)), l);
	memcpy(buf + l, f->buffer, len - l);
	f->out += len;

	return len;
}

struct fifo_t * fifo_alloc(unsigned int size)
{
	struct fifo_t * f;

	if(size & (size - 1))
		size = roundup_pow_of_two(size);

	f = malloc(sizeof(struct fifo_t));
	if(!f)
		return NULL;

	f->buffer = malloc(size);
	if(!f->buffer)
	{
		free(f);
		return NULL;
	}
	f->size = size;
	f->in = 0;
	f->out = 0;
	return f;
}

void fifo_free(struct fifo_t * f)
{
	if(f)
	{
		free(f->buffer);
		free(f);
	}
}

void fifo_reset(struct fifo_t * f)
{
	__fifo_reset(f);
}

unsigned int fifo_len(struct fifo_t * f)
{
	unsigned int ret;
	ret = __fifo_len(f);
	return ret;
}

unsigned int fifo_put(struct fifo_t * f, unsigned char * buf, unsigned int len)
{
	unsigned int ret;
	ret = __fifo_put(f, buf, len);
	return ret;
}

unsigned int fifo_get(struct fifo_t * f, unsigned char * buf, unsigned int len)
{
	unsigned int ret;
	ret = __fifo_get(f, buf, len);
	if(f->in == f->out)
		f->in = f->out = 0;
	return ret;
}
