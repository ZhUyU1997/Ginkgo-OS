#pragma once

#include <types.h>

int memcmp(const void *, const void *, uint32);
void *memmove(void *, const void *, uint32);
void *memcpy(void *dst, const void *src, uint32 n);
void *memset(void *, int, uint32);
char *safestrcpy(char *, const char *, int);
int strlen(const char *);
int strncmp(const char *, const char *, uint32);
char *strncpy(char *, const char *, int);

int strcmp(const char *s1, const char *s2);
char *strdup(const char *s);
char *strcpy(char *dest, const char *src);