#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <log.h>
#include <types.h>
#include <malloc.h>

#define panic PANIC

#include "str.h"
#include "array.h"

typedef enum
{
    TYPE_NULL = 0,
    TYPE_BOOL = 1,
    TYPE_NUMBER = 2,
    TYPE_STRING = 3,
    TYPE_ARRAY = 4,
    TYPE_OBJECT = 8,
} value_type_t;

typedef struct xjil_value_t xjil_value_t;

typedef struct xjil_value_t
{
    union
    {
        u64_t num;
        string_t *str;
        struct
        {
            array_t *arr;
            array_t *obj;
        };
    };
    string_t *tag;
    value_type_t type;
} xjil_value_t;

typedef struct
{
    string_t *key;
    xjil_value_t *v;
} member_t;

typedef struct
{
    const char *src;
    int len;
    int pos;
    int line;
    u64_t num;
    string_t *str;
} context_t;

typedef enum
{
    TOKEN_PARENTHESS_OPEN = '(',
    TOKEN_PARENTHESS_CLOSE = ')',
    TOKEN_MUL = '*',
    TOKEN_PLUS = '+',
    TOKEN_COMMA = ',',
    TOKEN_MINUS = '-',
    TOKEN_DIV = '/',
    TOKEN_COLON = ':',
    TOKEN_LT = '<',
    TOKEN_GT = '>',
    TOEKN_BRACKET_OPEN = '[',
    TOEKN_BRACKET_CLOSE = ']',
    TOKEN_BRACE_OPEN = '{',
    TOKEN_BRACE_CLOSE = '}',
    TOKEN_NUM,
    TOKEN_ID,
    TOKEN_BOOL,
    TOKEN_STRING,
    TOKEN_TYPE_MAX,
} token_type_t;

static void value_init(xjil_value_t *value, value_type_t type)
{
    value->type |= type;

    if (type == TYPE_ARRAY)
    {
        if (value->arr)
            array_clear(value->arr);
        else
            value->arr = array_new(1);
    }
    else if (type == TYPE_OBJECT)
    {
        if (value->obj)
            array_clear(value->obj);
        else
            value->obj = array_new(1);
    }
    else if (type == TYPE_STRING)
    {
        if (value->str)
            string_clear(value->str);
        else
            value->str = string_new(10);
    }
    else if (type == TYPE_NUMBER)
    {
        value->num = 0;
    }
    else if (type == TYPE_BOOL)
    {
        value->num = 0;
    }
}

static xjil_value_t *value_new(value_type_t type)
{
    xjil_value_t *value = calloc(1, sizeof(xjil_value_t));
    if (!value)
    {
        assert(value);
    }
    value->type = type;

    value_init(value, type);
    return value;
}

static void member_del(member_t *mem);

static void value_del(xjil_value_t *value)
{
    assert(value);

    value_type_t type = value->type;

    if (value->tag)
    {
        string_destory(value->tag);
        value->tag = NULL;
    }

    if (type & (TYPE_ARRAY | TYPE_OBJECT))
    {
        if (type & TYPE_ARRAY)
        {
            if (value->arr)
            {
                for (int i = 0; i < value->arr->size; i++)
                {
                    xjil_value_t *v = (xjil_value_t *)array_get(value->arr, i);
                    value_del(v);
                }

                array_destory(value->arr);
                value->arr = NULL;
            }
        }

        if (type & TYPE_OBJECT)
        {
            if (value->obj)
            {
                for (int i = 0; i < value->obj->size; i++)
                {
                    member_t *m = (member_t *)array_get(value->obj, i);
                    member_del(m);
                }

                array_destory(value->obj);
                value->obj = NULL;
            }
        }
    }
    else if (type == TYPE_STRING)
    {
        if (value->str)
        {
            string_destory(value->str);
            value->str = NULL;
        }
    }
    else if (type == TYPE_NUMBER)
    {
        value->num = 0;
    }
    else if (type == TYPE_BOOL)
    {
        value->num = 0;
    }

    free(value);
    return;
}

static member_t *member_new()
{
    member_t *mem = malloc(sizeof(member_t));
    if (!mem)
    {
        assert(mem);
    }

    mem->v = value_new(TYPE_NULL);
    mem->key = string_new(10);
    return mem;
}

static void member_del(member_t *mem)
{
    value_del(mem->v);
    string_destory(mem->key);
    free(mem);
    return;
}

static void push_element(xjil_value_t *arr, xjil_value_t *value)
{
    assert(arr && value && (arr->type & TYPE_ARRAY));
    array_push(arr->arr, value);
}

static void push_member(xjil_value_t *obj, member_t *mem)
{
    assert(obj && mem && (obj->type & TYPE_OBJECT));
    array_push(obj->obj, mem);
}

token_type_t next(context_t *ctx)
{
    token_type_t type = TOKEN_TYPE_MAX;

    char c;
    const char *src = ctx->src + ctx->pos;
    const char *old_src = src;

    while ((c = *src))
    {
        if (c == '\n')
        {
            old_src = src;
            ctx->line++;
        }
        else if (c == '\t' || c == '\r' || c == ' ')
        {
        }
        else if (isalpha(c) || (c == '_'))
        {
            string_clear(ctx->str);
            string_push(ctx->str, c);

            while ((c = *++src))
            {
                if (!isalnum(c) && (c != '_') && (c != '-'))
                    break;
                string_push(ctx->str, c);
            }

            if (!strncmp(ctx->str->arr, "true", ctx->str->size))
            {
                ctx->num = 1;
                type = TOKEN_BOOL;
            }
            else if (!strncmp(ctx->str->arr, "false", ctx->str->size))
            {
                ctx->num = 0;
                type = TOKEN_BOOL;
            }
            else
            {
                type = TOKEN_ID;
            }
            break;
        }
        else if (isdigit(c))
        {
            u64_t token_val = c - '0';
            if (token_val > 0)
            {
                while ((c = *++src))
                {
                    if (!isalnum(c))
                        break;
                    token_val = token_val * 10 + c - '0';
                }
            }
            else
            {
                c = *++src;

                if (c == 'x' || c == 'X')
                {
                    int count = 0;
                    while ((c = *++src))
                    {
                        if (!(isalnum(c) || ((c >= 'A') && (c <= 'F')) | ((c >= 'a') && (c <= 'f'))))
                            break;
                        count++;
                        token_val = token_val * 16 + (c & 15) + (c >= 'A' ? 9 : 0);
                    }

                    if (!count)
                    {
                        panic("exception");
                    }
                }
            }
            ctx->num = token_val;
            type = TOKEN_NUM;
            break;
        }
        else if (c == '"')
        {
            string_clear(ctx->str);
            while ((c = *++src))
            {

                if ((c == '\0') || (c == '"'))
                {
                    ++src;
                    break;
                }

                if (c == '\\')
                {
                    c = *++src;
                    switch (c)
                    {
                    case 'n':
                        c = '\n';
                        break;
                    case 'r':
                        c = '\r';
                        break;
                    case 'f':
                        c = '\f';
                        break;
                    case 't':
                        c = '\t';
                        break;
                    case '"':
                        c = '\"';
                        break;
                    case '\'':
                        c = '\'';
                        break;
                    case '\\':
                        c = '\\';
                        break;
                    case '\f':
                        c = '\f';
                        break;
                    default:
                        break;
                    }
                }

                string_push(ctx->str, c);
            }

            type = TOKEN_STRING;
            break;
        }
        else if (c == '{' || c == '}' || c == ':' || c == ',' || c == '[' || c == ']' || c == '>' || c == '<' || c == '+' || c == '-' || c == '*' || c == '(' || c == ')')
        {
            type = c;
            ++src;
            break;
        }
        else if (c == '/')
        {
            c = *++src;

            if (c == '/')
            {
                while ((c = *++src))
                {
                    if (c == '\n')
                        break;
                }
            }
            else
            {
                type = '/';
                break;
            }
        }
        else
        {
            panic("unknown token: (", $(c & 0xff) ")");
        }

        ++src;
    }

    ctx->pos = src - ctx->src;
    return type;
}

token_type_t expect(context_t *ctx, token_type_t type)
{
    token_type_t t = next(ctx);
    if (t != type)
    {
        panic("Expect {%d}\n", type);
    }
    return t;
}

int next_if(context_t *ctx, token_type_t type)
{
    int pos = ctx->pos;
    int line = ctx->line;

    token_type_t t = next(ctx);
    if (t != type)
    {
        ctx->pos = pos;
        ctx->line = line;
    }
    return t == type;
}

token_type_t peek(context_t *ctx)
{
    int pos = ctx->pos;
    int line = ctx->line;
    token_type_t t = next(ctx);
    ctx->pos = pos;
    ctx->line = line;
    return t;
}

static void parser_value(context_t *ctx, xjil_value_t *value);

static void parser_object(context_t *ctx, xjil_value_t *value)
{
    expect(ctx, '{');
    while (peek(ctx) == TOKEN_ID)
    {
        member_t *mem = member_new();
        expect(ctx, TOKEN_ID);
        string_copy(mem->key, ctx->str);
        expect(ctx, ':');
        parser_value(ctx, mem->v);
        push_member(value, mem);
        if (!next_if(ctx, ','))
            break;
    }
    expect(ctx, '}');
}

static void parser_array(context_t *ctx, xjil_value_t *arr)
{
    expect(ctx, '[');
    while (peek(ctx) != ']')
    {
        xjil_value_t *v = value_new(TYPE_NULL);
        parser_value(ctx, v);
        push_element(arr, v);
        if (!next_if(ctx, ','))
            break;
    }
    expect(ctx, ']');
}

static void parser_tag(context_t *ctx, xjil_value_t *value)
{
    expect(ctx, '<');
    expect(ctx, TOKEN_ID);
    if (value->tag == NULL)
    {
        value->tag = string_new(10);
    }
    string_copy(value->tag, ctx->str);
    expect(ctx, '>');
}

static int parser_exp(context_t *ctx);

static int parser_factor(context_t *ctx)
{
    token_type_t type = peek(ctx);
    int value;
    if (type == '(')
    {
        expect(ctx, '(');
        value = parser_exp(ctx);
        expect(ctx, ')');
    }
    else if (type == TOKEN_NUM)
    {
        next(ctx);
        value = ctx->num;
    }
    else
    {
        panic("");
    }
    return value;
}

static int parser_term(context_t *ctx)
{
    int value = parser_factor(ctx);

    token_type_t type = peek(ctx);

    while (1)
    {
        if (!(type == '*' || type == '/'))
            break;

        next(ctx);

        int v = parser_factor(ctx);
        if (type == '*')
            value *= v;
        else if (type == '/')
            value /= v;
        type = peek(ctx);
    }
    return value;
}

static int parser_exp(context_t *ctx)
{
    int value = parser_term(ctx);

    token_type_t type = peek(ctx);

    while (1)
    {
        if (!(type == '+' || type == '-'))
            break;
        next(ctx);
        int v = parser_term(ctx);

        if (type == '+')
            value += v;
        else if (type == '-')
            value -= v;
        type = peek(ctx);
    }

    return value;
}

static void parser_value(context_t *ctx, xjil_value_t *value)
{
    token_type_t type = peek(ctx);
    int with_tag = 0;
    if (type == '<')
    {
        parser_tag(ctx, value);
        type = peek(ctx);
        with_tag = 1;
    }

    if (type == '{')
    {
        value_init(value, TYPE_OBJECT);
        parser_object(ctx, value);
        token_type_t type = peek(ctx);
        if (type == '[')
        {
            value_init(value, TYPE_ARRAY);
            parser_array(ctx, value);
        }
    }
    else if (type == '[')
    {
        value_init(value, TYPE_ARRAY);
        parser_array(ctx, value);
    }
    else if (type == TOKEN_NUM || type == '(')
    {
        value_init(value, TYPE_NUMBER);
        value->num = parser_exp(ctx);
    }
    else if (type == TOKEN_STRING)
    {
        value_init(value, TYPE_STRING);
        next(ctx);
        string_copy(value->str, ctx->str);
    }
    else if (type == TOKEN_BOOL)
    {
        value_init(value, TYPE_BOOL);
        next(ctx);
        value->num = ctx->num;
    }
    else
    {
        if (!with_tag)
        {
            panic("error type\n");
        }
    }
}

int xjil_parse(xjil_value_t *v, const char *json, size_t len)
{
    if (!(v && json && len))
        return -1;

    context_t ctx = {
        .src = json,
        .len = len,
        .pos = 0,
        .line = 1,
        .str = string_new(10),
    };
    parser_value(&ctx, v);
    string_destory(ctx.str);
    // print_value(v, 0);
    return 0;
}

xjil_value_t *xjil_new()
{
    return value_new(TYPE_NULL);
}

void xjil_del(xjil_value_t *v)
{
    assert(v);
    value_del(v);
    return;
}

char *xjil_value_get_tag(const xjil_value_t *v)
{
    assert(v);
    return v->tag->arr;
}

int xjil_is_null(const xjil_value_t *v)
{
    assert(v);
    return v->type == TYPE_NULL;
}
int xjil_is_string(const xjil_value_t *v)
{
    assert(v);
    return v->type == TYPE_STRING;
}
int xjil_is_boolean(const xjil_value_t *v)
{
    assert(v);
    return v->type == TYPE_BOOL;
}

int xjil_is_number(const xjil_value_t *v)
{
    assert(v);
    return v->type == TYPE_NUMBER;
}
int xjil_is_array(const xjil_value_t *v)
{
    assert(v);
    return v->type & TYPE_ARRAY;
}
int xjil_is_object(const xjil_value_t *v)
{
    assert(v);
    return v->type & TYPE_OBJECT;
}

int xjil_get_boolean(const xjil_value_t *v)
{
    assert(v);
    assert(v->type == TYPE_BOOL);
    return v->num;
}

u64_t xjil_get_number(const xjil_value_t *v)
{
    assert(v);
    assert(v->type == TYPE_NUMBER);
    return v->num;
}

size_t xjil_get_string_size(const xjil_value_t *v)
{
    assert(v);
    assert(v->type == TYPE_STRING);
    return v->str->size;
}

const char *xjil_get_string(const xjil_value_t *v)
{
    assert(v);
    assert(v->type == TYPE_STRING);
    return v->str->arr;
}

size_t xjil_get_array_size(const xjil_value_t *v)
{
    assert(v);
    assert(v->type & TYPE_ARRAY);
    return v->arr->size;
}

xjil_value_t *xjil_get_array_element(const xjil_value_t *v, size_t index)
{
    assert(v);
    assert(v->type & TYPE_ARRAY);
    assert(v->arr->size > index);
    return v->arr->arr[index];
}

size_t xjil_get_object_size(const xjil_value_t *v)
{
    assert(v);
    assert(v->type & TYPE_OBJECT);
    return v->obj->size;
}
const char *xjil_get_object_key(const xjil_value_t *v, size_t index)
{
    assert(v);
    assert(v->type & TYPE_OBJECT);
    assert(v->obj->size > index);

    return ((member_t *)v->obj->arr[index])->key->arr;
}

xjil_value_t *xjil_get_object_value(const xjil_value_t *v, size_t index)
{
    assert(v);
    assert(v->type & TYPE_OBJECT);
    assert(v->obj->size > index);
    return ((member_t *)v->obj->arr[index])->v;
}

xjil_value_t *xjil_find_object_value(const xjil_value_t *v, const char *key)
{
    assert(v && key);
    assert(v->type & TYPE_OBJECT);
    for (int i = 0; i < v->obj->size; i++)
    {
        member_t *m = (member_t *)array_get(v->obj, i);
        if (!strcmp(m->key->arr, key))
        {
            return m->v;
        }
    }
    return NULL;
}

const char *xjil_read_string(const xjil_value_t *v, const char *name, char *def)
{
    xjil_value_t *t = xjil_find_object_value(v, name);
    if (t && xjil_is_string(t))
    {
        return xjil_get_string(t);
    }
    return def;
}

int xjil_read_int(const xjil_value_t *v, const char *name, int def)
{
    xjil_value_t *t = xjil_find_object_value(v, name);
    if (t && xjil_is_number(t))
    {
        return xjil_get_number(t);
    }
    return def;
}

long xjil_read_long(const xjil_value_t *v, const char *name, long def)
{
    xjil_value_t *t = xjil_find_object_value(v, name);
    if (t && xjil_is_number(t))
    {
        return xjil_get_number(t);
    }
    return def;
}

u8_t xjil_read_u8(const xjil_value_t *v, const char *name, u8_t def)
{
    xjil_value_t *t = xjil_find_object_value(v, name);
    if (t && xjil_is_number(t))
    {
        return xjil_get_number(t);
    }
    return def;
}

u16_t xjil_read_u16(const xjil_value_t *v, const char *name, u16_t def)
{
    xjil_value_t *t = xjil_find_object_value(v, name);
    if (t && xjil_is_number(t))
    {
        return xjil_get_number(t);
    }
    return def;
}
u32_t xjil_read_u32(const xjil_value_t *v, const char *name, u32_t def)
{
    xjil_value_t *t = xjil_find_object_value(v, name);
    if (t && xjil_is_number(t))
    {
        return xjil_get_number(t);
    }
    return def;
}
u64_t xjil_read_u64(const xjil_value_t *v, const char *name, u64_t def)
{
    xjil_value_t *t = xjil_find_object_value(v, name);
    if (t && xjil_is_number(t))
    {
        return xjil_get_number(t);
    }
    return def;
}

int xjil_read_bool(const xjil_value_t *v, const char * name, int def)
{
    xjil_value_t *t = xjil_find_object_value(v, name);
    if (t && xjil_is_boolean(t))
    {
        return xjil_get_boolean(t);
    }
    return def;
}

