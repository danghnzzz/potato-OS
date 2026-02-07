#ifndef TTY_H
#define TTY_H

#include <stddef.h>
#include <stdint.h>

#define TTY_QUEUE_SIZE 256

void init_tty(void);
void tty_putc(char c);
ssize_t tty_read(char *buf, uint32_t count);
ssize_t tty_write(const char *buf, uint32_t count);

#endif
