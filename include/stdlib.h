#ifndef __STDLIB_H__
#define __STDLIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <malloc.h>

unsigned long strtoul(const char * nptr, char ** endptr, int base);


#ifdef __cplusplus
}
#endif

#endif /* __STDLIB_H__ */
