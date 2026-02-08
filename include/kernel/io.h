#ifndef IO_H
#define IO_H

#include <stdint.h>

static uint8_t inb(uint16_t port)
{
    uint8_t val;
    __asm__ volatile(
        "in al, dx"
        : "=a"(val)
        : "d"(port)
        :
    );
    return val;
}
static void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile(
        "out dx, al"
        :
        : "d"(port), "a"(val)
        :
    );
}
static uint16_t inw(uint16_t port)
{
    uint16_t val;
    __asm__ volatile(
        "in ax, dx"
        : "=a"(val)
        : "d"(port)
        :
    );
    return val;
}
static void outw(uint16_t port, uint16_t val)
{
    __asm__ volatile(
        "out dx, ax"
        :
        : "d"(port), "a"(val)
        :
    );
}
static uint32_t inl(uint16_t port)
{
    uint32_t val;
    __asm__ volatile(
        "in eax, dx"
        : "=a"(val)
        : "d"(port)
        :
    );
    return val;
}
static void outl(uint16_t port, uint32_t val)
{
    __asm__ volatile(
        "out dx, eax"
        :
        : "d"(port), "a"(val)
        :
    );
}

#endif
