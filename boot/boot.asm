bits 32

global _start

extern main

section .multiboot
align 4
.boot:
    dd 0x1badb002
    dd 0x00000000
    dd -(0x1badb002 + 0x00000000)

section .text
_start:
    cli
    mov esp, stack
    call main
.hang:
    hlt
    jmp .hang

section .bss
resb 8192
stack:
