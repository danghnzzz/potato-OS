#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

#define VGA_ADDRESS ((volatile uint16_t *) 0xb8000)
#define CONSOLE_WIDTH 80
#define CONSOLE_HEIGHT 25
#define VGA_ATTR 0x0f

void console_reset(void);
void console_scroll(void);
void console_putc(char c);
uint8_t console_puts(const char *s);

#endif
