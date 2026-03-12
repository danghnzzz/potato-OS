bits 32

global _start
global kernel_stack_top

extern main

BOOT_STACK_TOP equ 0x0009f000
KERNEL_STACK_SIZE equ 8192
KERNEL_VMA_TO_LMA_OFFSET equ 0xc0000000
PAGE_FLAG_PRESENT equ 0x1
PAGE_FLAG_WRITABLE equ 0x2
PAGE_SIZE equ 4096
HIGHER_HALF_KERNEL_PDE_INDEX equ 768

section .multiboot
align 4
boot:
    dd 0x1badb002
    dd 0x00000000
    dd -(0x1badb002 + 0x00000000)

section .text
_start:
    cli
    mov esp, BOOT_STACK_TOP
    call setup_boot_paging
    mov esp, kernel_stack_top
    xor ebp, ebp
    jmp main

setup_boot_paging:
    mov edi, boot_page_table - KERNEL_VMA_TO_LMA_OFFSET
    xor eax, eax
    mov ecx, 1024
fill_page_table:
    mov edx, eax
    or edx, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE
    mov [edi], edx
    add eax, PAGE_SIZE
    add edi, 4
    loop fill_page_table
    mov edi, boot_page_directory - KERNEL_VMA_TO_LMA_OFFSET
    xor eax, eax
    mov ecx, 1024
    rep stosd
    mov eax, boot_page_table - KERNEL_VMA_TO_LMA_OFFSET
    or eax, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITABLE
    mov [boot_page_directory - KERNEL_VMA_TO_LMA_OFFSET], eax
    mov [boot_page_directory - KERNEL_VMA_TO_LMA_OFFSET + (HIGHER_HALF_KERNEL_PDE_INDEX * 4)], eax
    mov eax, boot_page_directory - KERNEL_VMA_TO_LMA_OFFSET
    mov cr3, eax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    ret

section .bss
align 16
kernel_stack_bottom:
    resb KERNEL_STACK_SIZE
kernel_stack_top:
align 4096
boot_page_directory:
    resd 1024
align 4096
boot_page_table:
    resd 1024
