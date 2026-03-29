#ifndef UNISTD_H
#define UNISTD_H

#include <stddef.h>
#include <stdint.h>

__attribute__((noreturn)) void _exit(int status_code);
ssize_t read(int32_t fd, void *buf, uint32_t count);
ssize_t write(int32_t fd, const void *buf, uint32_t count);
int32_t open(const char *pathname, int32_t flags);
int32_t close(int32_t fd);
int32_t execve(const char *pathname, char *const argv[], char *const envp[]);

#endif
