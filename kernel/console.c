#include <stdint.h>
#include <kernel/io.h>
#include <kernel/console.h>

static uint16_t cursor_x = 0;
static uint16_t cursor_y = 0;
static const char *HEX_DIGITS = "0123456789abcdef";

void set_console_cursor_pos(void)
{
    uint16_t pos = cursor_y * CONSOLE_WIDTH + cursor_x;
    outb(VGA_CRTC_INDEX_REGISTER, 0xf);
    outb(VGA_CRTC_DATA_REGISTER, (uint8_t) (pos & 0xff));
    outb(VGA_CRTC_INDEX_REGISTER, 0xe);
    outb(VGA_CRTC_DATA_REGISTER, (uint8_t) (pos >> 8));
}

void enable_console_cursor(void)
{
    outb(VGA_CRTC_INDEX_REGISTER, 0xa);
    outb(VGA_CRTC_DATA_REGISTER, (inb(VGA_CRTC_DATA_REGISTER) & 0xc0 | CURSOR_START));
    outb(VGA_CRTC_INDEX_REGISTER, 0xb);
    outb(VGA_CRTC_DATA_REGISTER, (inb(VGA_CRTC_DATA_REGISTER) & 0xe0 | CURSOR_END));
    set_console_cursor_pos();
}

void disable_console_cursor(void)
{
    outb(VGA_CRTC_INDEX_REGISTER, 0xa);
    outb(VGA_CRTC_DATA_REGISTER, 0x20);
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

uint8_t console_putc(char c)
{
    if (!c)
    {
        return 0;
    }
    if (c == '\n')
    {
        cursor_x = 0;
        cursor_y++;
    }
    else if (c == '\b')
    {
        if (cursor_x > 0)
        {
            cursor_x--;
        }
        else if (cursor_y > 0)
        {
            cursor_y--;
            cursor_x = CONSOLE_WIDTH - 1;
        }
        VGA_ADDRESS[cursor_y * CONSOLE_WIDTH + cursor_x] = (VGA_ATTR << 8) | ' ';
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
    return 1;
}

uint8_t console_puts(const char *s)
{
    uint8_t written = 0;
    while (*s)
    {
        written += console_putc(*s);
        s++;
    }
    return written;
}

uint8_t console_put_hex8(uint8_t val, uint8_t prefix)
{
    uint8_t written = 0;
    if (prefix)
    {
        console_putc('0');
        console_putc('x');
        written += 2;
    }
    written += console_putc(HEX_DIGITS[(val >> 4) & 0xf]);
    written += console_putc(HEX_DIGITS[val & 0xf]);
    return written;
}

uint8_t console_put_hex16(uint16_t val, uint8_t prefix)
{
    uint8_t written = 0;
    if (prefix)
    {
        console_putc('0');
        console_putc('x');
        written += 2;
    }
    written += console_put_hex8((uint8_t) (val >> 8), 0);
    written += console_put_hex8((uint8_t) val, 0);
    return written;
}

uint8_t console_put_hex32(uint32_t val, uint8_t prefix)
{
    uint8_t written = 0;
    if (prefix)
    {
        console_putc('0');
        console_putc('x');
        written += 2;
    }
    written += console_put_hex16((uint16_t) (val >> 16), 0);
    written += console_put_hex16((uint16_t) val, 0);
    return written;
}
