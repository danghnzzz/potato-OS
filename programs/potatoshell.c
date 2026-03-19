#include <stdint.h>
#include <unistd.h>

#define INPUT_MAX_LENGTH 256

void _start(void)
{
    const char shell_prompt[] = "potato$hell> ";
    char input[INPUT_MAX_LENGTH];
    uint8_t input_char_count = 0;
    write(1, shell_prompt, sizeof(shell_prompt) - 1);
    for (;;)
    {
        ssize_t received = read(0, input + input_char_count, 1);
        if (received <= 0)
        {
            continue;
        }
        if (input[input_char_count] == '\b')
        {
            if (input_char_count == 0)
            {
                continue;
            }
            write(1, input + input_char_count, 1);
            input_char_count--;
            continue;
        }
        if (input[input_char_count] == '\n')
        {
            write(1, input + input_char_count, 1);
            input_char_count = 0;
            write(1, shell_prompt, sizeof(shell_prompt) - 1);
            continue;
        }
        write(1, input + input_char_count, 1);
        input_char_count++;
    }
}
