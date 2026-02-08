#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

static inline void *memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *d = (uint8_t *) dest;
    const uint8_t *s = (const uint8_t *) src;
    for (size_t i = 0; i < n; i++)
    {
        d[i] = s[i];
    }
    return dest;
}
static inline void *memset(void *dest, int val, size_t n)
{
    uint8_t *d = (uint8_t *) dest;
    for (size_t i = 0; i < n; i++)
    {
        d[i] = (uint8_t) val;
    }
    return dest;
}

#endif
