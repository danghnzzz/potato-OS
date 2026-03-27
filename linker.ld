OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)
SECTIONS
{
    KERNEL_VMA = 0xc0100000;
    KERNEL_LMA = 0x00100000;
    . = KERNEL_VMA;
    .multiboot : AT(KERNEL_LMA) { *(.multiboot) }
    .text : AT(KERNEL_LMA + (ADDR(.text) - KERNEL_VMA)) {*(.text)}
    .rodata : AT(KERNEL_LMA + (ADDR(.rodata) - KERNEL_VMA)) {*(.rodata)}
    .data : AT(KERNEL_LMA + (ADDR(.data) - KERNEL_VMA)) {*(.data)}
    .bss : AT(KERNEL_LMA + (ADDR(.bss) - KERNEL_VMA)) {*(.bss)}
}
