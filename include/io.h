#ifndef IO_H
#define IO_H

#include <stdint.h>

static inline uint8_t inb(uint16_t port)
{
    uint8_t val;
    __asm__ volatile
    (
        "in al, dx"
        : "=a"(val)
        : "d"(port)
        :
    );
    return val;
}

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile
    (
        "out dx, al"
        :
        : "d"(port), "a"(val)
        :
    );
}

#endif
