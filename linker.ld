OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)
SECTIONS
{
    . = 0x100000;
    .multiboot : {*(.multiboot)}
    .text : {*(.text)}
    .rodata : {*(.rodata)}
    .data : {*(.data)}
    .bss : {*(.bss)}

    . = 0x00400000;
    .user_text : {*(.user_text)}
    .user_rodata : {*(.user_rodata)}
    .user_data : {*(.user_data)}
    .user_bss : {*(.user_bss)}
}
