#include <stdint.h>
#include <unistd.h>

#define USER_STACK_SIZE 8192

static uint8_t user_stack[USER_STACK_SIZE] __attribute__((aligned(16)));
uint8_t *user_stack_top = user_stack + USER_STACK_SIZE;

void user_entry_point(void)
{
    const char msg[] = "Hello from user-space!\n";
    write(1, msg, sizeof(msg) - 1);
    for (;;);
}
