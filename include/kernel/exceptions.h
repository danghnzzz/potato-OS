#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdint.h>

#define DECLARE_EXCEPTION_NOERR(n) \
    __attribute__((naked)) static void exception##n(void) \
    { \
        __asm__ volatile( \
            "pusha\n" \
            "push 0\n" \
            "push " #n "\n" \
            "call exceptions_dispatch\n" \
            "add esp, 8\n" \
            "popa\n" \
            "iret\n" \
            : \
            : \
            : \
        ); \
    }
#define DECLARE_EXCEPTION_ERRCODE(n) \
    __attribute__((naked)) static void exception##n(void) \
    { \
        __asm__ volatile( \
            "pusha\n" \
            "push dword [esp + 32]\n" \
            "push " #n "\n" \
            "call exceptions_dispatch\n" \
            "add esp, 8\n" \
            "popa\n" \
            "add esp, 4\n" \
            "iret\n" \
            : \
            : \
            : \
        ); \
    }

typedef void (*exception_handler_t)(uint8_t exc, uint32_t err_code);

void init_exceptions(void);
void register_exception_handler(uint8_t exc, exception_handler_t handler);

#endif
