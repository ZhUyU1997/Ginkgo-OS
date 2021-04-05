#include <types.h>
#include <malloc.h>

void *memset(void *dst, int c, uint32 n)
{
    char *cdst = (char *)dst;
    int i;
    for (i = 0; i < n; i++)
    {
        cdst[i] = c;
    }
    return dst;
}

int memcmp(const void *v1, const void *v2, uint32 n)
{
    const uchar *s1, *s2;

    s1 = v1;
    s2 = v2;
    while (n-- > 0)
    {
        if (*s1 != *s2)
            return *s1 - *s2;
        s1++, s2++;
    }

    return 0;
}

void *memmove(void *dst, const void *src, uint32 n)
{
    const char *s;
    char *d;

    s = src;
    d = dst;
    if (s < d && s + n > d)
    {
        s += n;
        d += n;
        while (n-- > 0)
            *--d = *--s;
    }
    else
        while (n-- > 0)
            *d++ = *s++;

    return dst;
}

// memcpy exists to placate GCC.  Use memmove.
void *memcpy(void *dst, const void *src, uint32 n)
{
    return memmove(dst, src, n);
}

int strncmp(const char *p, const char *q, uint32 n)
{
    while (n > 0 && *p && *p == *q)
        n--, p++, q++;
    if (n == 0)
        return 0;
    return (uchar)*p - (uchar)*q;
}

char *strncpy(char *s, const char *t, int n)
{
    char *os;

    os = s;
    while (n-- > 0 && (*s++ = *t++) != 0)
        ;
    while (n-- > 0)
        *s++ = 0;
    return os;
}

// Like strncpy but guaranteed to NUL-terminate.
char *safestrcpy(char *s, const char *t, int n)
{
    char *os;

    os = s;
    if (n <= 0)
        return os;
    while (--n > 0 && (*s++ = *t++) != 0)
        ;
    *s = 0;
    return os;
}

int strlen(const char *s)
{
    int n;

    for (n = 0; s[n]; n++)
        ;
    return n;
}

int strcmp(const char *s1, const char *s2)
{
    int res;

    while (1)
    {
        if ((res = *s1 - *s2++) != 0 || !*s1++)
            break;
    }
    return res;
}

char *strcpy(char *dest, const char *src)
{
    char *tmp = dest;

    while ((*dest++ = *src++) != '\0')
        ;
    return tmp;
}

char *strdup(const char *s)
{
    char *p;

    if (!s)
        return NULL;

    p = malloc(strlen(s) + 1);
    if (p)
        return (strcpy(p, s));

    return NULL;
}

char *strstr(const char *s1, const char *s2)
{
    size_t l1, l2;

    l2 = strlen(s2);
    if (!l2)
        return (char *)s1;

    l1 = strlen(s1);
    while (l1 >= l2)
    {
        l1--;
        if (!memcmp(s1, s2, l2))
            return (char *)s1;
        s1++;
    }

    return NULL;
}

size_t strlcpy(char *dest, const char *src, size_t n)
{
    size_t len;
    size_t ret = strlen(src);

    if (n)
    {
        len = (ret >= n) ? n - 1 : ret;
        memcpy(dest, src, len);
        dest[len] = '\0';
    }
    return ret;
}

char *strchr(const char *s, int c)
{
    for (; *s != (char)c; ++s)
        if (*s == '\0')
            return NULL;
    return (char *)s;
}

char *strrchr(const char *s, int c)
{
    const char *p = s + strlen(s);

    do
    {
        if (*p == (char)c)
            return (char *)p;
    } while (--p >= s);

    return NULL;
}