#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

#define VGA_CRTC_INDEX_REGISTER 0x3d4
#define VGA_CRTC_DATA_REGISTER 0x3d5
#define VGA_ADDRESS ((volatile uint16_t *) 0xb8000)
#define CONSOLE_WIDTH 80
#define CONSOLE_HEIGHT 25
#define VGA_ATTR 0xf
#define CURSOR_START 0x0
#define CURSOR_END 0xf

void set_console_cursor_pos(void);
void enable_console_cursor(void);
void disable_console_cursor(void);
void console_reset(void);
void console_scroll(void);
uint8_t console_putc(char c);
uint8_t console_puts(const char *s);
uint8_t console_put_hex8(uint8_t val, uint8_t prefix);
uint8_t console_put_hex16(uint16_t val, uint8_t prefix);
uint8_t console_put_hex32(uint32_t val, uint8_t prefix);

#endif
