#include <stdint.h>
#include <io.h>
#include <kernel/console.h>

static uint16_t cursor_x = 0;
static uint16_t cursor_y = 0;

void set_console_cursor_pos(void)
{
    uint16_t pos = cursor_y * CONSOLE_WIDTH + cursor_x;
    outb(0x3d4, 0x0f);
    outb(0x3d5, (uint8_t) (pos & 0xff));
    outb(0x3d4, 0x0e);
    outb(0x3d5, (uint8_t) (pos >> 8));
}

void enable_console_cursor(void)
{
    outb(0x3d4, 0x0a);
    outb(0x3d5, (inb(0x3d5) & 0xc0 | CURSOR_START));
    outb(0x3d4, 0x0b);
    outb(0x3d5, (inb(0x3d5) & 0xe0 | CURSOR_END));
    set_console_cursor_pos();
}

void disable_console_cursor(void)
{
    outb(0x3d4, 0x0a);
    outb(0x3d5, 0x20);
}

void console_reset(void)
{
    for (uint16_t y = 0; y < CONSOLE_HEIGHT; y++)
    {
        for (uint16_t x = 0; x < CONSOLE_WIDTH; x++)
        {
            VGA_ADDRESS[y * CONSOLE_WIDTH + x] = (VGA_ATTR << 8) | ' ';
        }
    }
    cursor_x = 0;
    cursor_y = 0;
    set_console_cursor_pos();
}

void console_scroll(void)
{
    if (cursor_y < CONSOLE_HEIGHT)
    {
        return;
    }
    for (uint16_t y = 1; y < CONSOLE_HEIGHT; y++)
    {
        for (uint16_t x = 0; x < CONSOLE_WIDTH; x++)
        {
            VGA_ADDRESS[(y - 1) * CONSOLE_WIDTH + x] = VGA_ADDRESS[y * CONSOLE_WIDTH + x];
        }
    }
    for (uint16_t x = 0; x < CONSOLE_WIDTH; x++)
    {
        VGA_ADDRESS[(CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH + x] = (VGA_ATTR << 8) | ' ';
    }
    cursor_y = CONSOLE_HEIGHT - 1;
}

void console_putc(char c)
{
    if (c == '\n')
    {
        cursor_x = 0;
        cursor_y++;
    }
    else
    {
        VGA_ADDRESS[cursor_y * CONSOLE_WIDTH + cursor_x] = (VGA_ATTR << 8) | (uint8_t) c;
        cursor_x++;
    }
    if (cursor_x >= CONSOLE_WIDTH)
    {
        cursor_x = 0;
        cursor_y++;
    }
    console_scroll();
    set_console_cursor_pos();
}

uint8_t console_puts(const char *s)
{
    uint8_t str_len = 0;
    while (*s)
    {
        console_putc(*s++);
        str_len++;
    }
    return str_len;
}
