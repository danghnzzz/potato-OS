#ifndef UNISTD_H
#define UNISTD_H

#include <stddef.h>
#include <stdint.h>

__attribute__((noreturn)) void _exit(int status_code);
ssize_t read(int fd, void *buf, uint32_t count);
ssize_t write(int fd, const void *buf, uint32_t count);

#endif
