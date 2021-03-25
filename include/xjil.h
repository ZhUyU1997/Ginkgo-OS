#pragma once

#include <types.h>

typedef struct xjil_value_t xjil_value_t;

int xjil_parse(xjil_value_t *v, const char *json, size_t len);
xjil_value_t *xjil_new();
void xjil_del(xjil_value_t *v);
char *xjil_value_get_tag(const xjil_value_t *v);

int xjil_is_null(const xjil_value_t *v);
int xjil_is_boolean(const xjil_value_t *v);
int xjil_is_string(const xjil_value_t *v);
int xjil_is_number(const xjil_value_t *v);
int xjil_is_array(const xjil_value_t *v);
int xjil_is_object(const xjil_value_t *v);

int xjil_get_boolean(const xjil_value_t *v);
u64_t xjil_get_number(const xjil_value_t *v);
size_t xjil_get_string_size(const xjil_value_t *v);
const char *xjil_get_string(const xjil_value_t *v);
size_t xjil_get_array_size(const xjil_value_t *v);
xjil_value_t *xjil_get_array_element(const xjil_value_t *v, size_t index);
size_t xjil_get_object_size(const xjil_value_t *v);
const char *xjil_get_object_key(const xjil_value_t *v, size_t index);
xjil_value_t *xjil_get_object_value(const xjil_value_t *v, size_t index);
xjil_value_t *xjil_find_object_value(const xjil_value_t *v, const char *key);
const char *xjil_read_string(const xjil_value_t *v, const char *name, char *def);
int xjil_read_int(const xjil_value_t *v, const char *name, int def);
long xjil_read_long(const xjil_value_t *v, const char *name, long def);
u8_t xjil_read_u8(const xjil_value_t *v, const char *name, u8_t def);
u16_t xjil_read_u16(const xjil_value_t *v, const char *name, u16_t def);
u32_t xjil_read_u32(const xjil_value_t *v, const char *name, u32_t def);
u64_t xjil_read_u64(const xjil_value_t *v, const char *name, u64_t def);
