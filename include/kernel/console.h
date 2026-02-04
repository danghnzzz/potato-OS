#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

#define VGA_CRTC_INDEX_REGISTER 0x3d4
#define VGA_CRTC_DATA_REGISTER 0x3d5
#define VGA_ADDRESS ((volatile uint16_t *) 0xb8000)
#define CONSOLE_WIDTH 80
#define CONSOLE_HEIGHT 25
#define VGA_ATTR 0x0f
#define CURSOR_START 0x0
#define CURSOR_END 0xf

void set_console_cursor_pos(void);
void enable_console_cursor(void);
void disable_console_cursor(void);
void console_reset(void);
void console_scroll(void);
void console_putc(char c);
uint8_t console_puts(const char *s);

#endif
