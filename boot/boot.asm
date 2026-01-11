bits 32

global _start

extern main

section .multiboot
align 4
.boot:
    dd 0x1badb002
    dd 0x00000000
    dd -(0x1badb002 + 0x00000000)

section .data
align 8
gdt_start:
    dq 0x0000000000000000
    dq 0x00cf9a000000ffff
    dq 0x00cf92000000ffff
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

section .text
_start:
    cli
    lgdt [gdt_descriptor]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.kernel_main
.kernel_main:
    mov esp, stack
    call main
.hang:
    hlt
    jmp .hang

section .bss
resb 8192
stack:
