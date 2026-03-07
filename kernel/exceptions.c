#include <stdint.h>
#include <kernel/interrupts.h>
#include <kernel/exceptions.h>
#include <kernel/console.h>

static exception_handler_t exception_handlers[32];

void exceptions_dispatch(uint8_t exc, uint32_t err_code)
{
    if (exc < 32 && exception_handlers[exc])
    {
        exception_handlers[exc](exc, err_code);
    }
}

DECLARE_EXCEPTION_NOERR(0)
DECLARE_EXCEPTION_NOERR(1)
DECLARE_EXCEPTION_NOERR(2)
DECLARE_EXCEPTION_NOERR(3)
DECLARE_EXCEPTION_NOERR(4)
DECLARE_EXCEPTION_NOERR(5)
DECLARE_EXCEPTION_NOERR(6)
DECLARE_EXCEPTION_NOERR(7)
DECLARE_EXCEPTION_ERRCODE(8)
DECLARE_EXCEPTION_NOERR(9)
DECLARE_EXCEPTION_ERRCODE(10)
DECLARE_EXCEPTION_ERRCODE(11)
DECLARE_EXCEPTION_ERRCODE(12)
DECLARE_EXCEPTION_ERRCODE(13)
DECLARE_EXCEPTION_ERRCODE(14)
DECLARE_EXCEPTION_NOERR(15)
DECLARE_EXCEPTION_NOERR(16)
DECLARE_EXCEPTION_ERRCODE(17)
DECLARE_EXCEPTION_NOERR(18)
DECLARE_EXCEPTION_NOERR(19)
DECLARE_EXCEPTION_NOERR(20)
DECLARE_EXCEPTION_ERRCODE(21)
DECLARE_EXCEPTION_NOERR(22)
DECLARE_EXCEPTION_NOERR(23)
DECLARE_EXCEPTION_NOERR(24)
DECLARE_EXCEPTION_NOERR(25)
DECLARE_EXCEPTION_NOERR(26)
DECLARE_EXCEPTION_NOERR(27)
DECLARE_EXCEPTION_NOERR(28)
DECLARE_EXCEPTION_ERRCODE(29)
DECLARE_EXCEPTION_ERRCODE(30)
DECLARE_EXCEPTION_NOERR(31)

static void install_exception_gates(void)
{
    idt_set_gate(0, (uint32_t) exception0, 0x8e);
    idt_set_gate(1, (uint32_t) exception1, 0x8e);
    idt_set_gate(2, (uint32_t) exception2, 0x8e);
    idt_set_gate(3, (uint32_t) exception3, 0x8e);
    idt_set_gate(4, (uint32_t) exception4, 0x8e);
    idt_set_gate(5, (uint32_t) exception5, 0x8e);
    idt_set_gate(6, (uint32_t) exception6, 0x8e);
    idt_set_gate(7, (uint32_t) exception7, 0x8e);
    idt_set_gate(8, (uint32_t) exception8, 0x8e);
    idt_set_gate(9, (uint32_t) exception9, 0x8e);
    idt_set_gate(10, (uint32_t) exception10, 0x8e);
    idt_set_gate(11, (uint32_t) exception11, 0x8e);
    idt_set_gate(12, (uint32_t) exception12, 0x8e);
    idt_set_gate(13, (uint32_t) exception13, 0x8e);
    idt_set_gate(14, (uint32_t) exception14, 0x8e);
    idt_set_gate(15, (uint32_t) exception15, 0x8e);
    idt_set_gate(16, (uint32_t) exception16, 0x8e);
    idt_set_gate(17, (uint32_t) exception17, 0x8e);
    idt_set_gate(18, (uint32_t) exception18, 0x8e);
    idt_set_gate(19, (uint32_t) exception19, 0x8e);
    idt_set_gate(20, (uint32_t) exception20, 0x8e);
    idt_set_gate(21, (uint32_t) exception21, 0x8e);
    idt_set_gate(22, (uint32_t) exception22, 0x8e);
    idt_set_gate(23, (uint32_t) exception23, 0x8e);
    idt_set_gate(24, (uint32_t) exception24, 0x8e);
    idt_set_gate(25, (uint32_t) exception25, 0x8e);
    idt_set_gate(26, (uint32_t) exception26, 0x8e);
    idt_set_gate(27, (uint32_t) exception27, 0x8e);
    idt_set_gate(28, (uint32_t) exception28, 0x8e);
    idt_set_gate(29, (uint32_t) exception29, 0x8e);
    idt_set_gate(30, (uint32_t) exception30, 0x8e);
    idt_set_gate(31, (uint32_t) exception31, 0x8e);
}

uint8_t init_exceptions(void)
{
    console_puts("Setting up CPU exceptions ... ");
    install_exception_gates();
    console_puts("Done\n");
    return 1;
}

void register_exception_handler(uint8_t exc, exception_handler_t handler)
{
    if (exc < 32)
    {
        exception_handlers[exc] = handler;
    }
}
