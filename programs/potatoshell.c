#include <stdint.h>
#include <unistd.h>

#define INPUT_MAX_LENGTH 256

static void run_command(const char *name)
{
    const char bin_dir[] = "/bin/";
    const char not_found[] = ": command not found\n";
    char path[sizeof(bin_dir) + INPUT_MAX_LENGTH];
    uint32_t path_length = 0;
    for (uint32_t i = 0; bin_dir[i]; i++)
    {
        path[path_length++] = bin_dir[i];
    }
    for (uint32_t i = 0; name[i]; i++)
    {
        path[path_length++] = name[i];
    }
    path[path_length] = '\0';
    int32_t fd = open(path, 0);
    if (fd < 0)
    {
        write(1, name, path_length - (sizeof(bin_dir) - 1));
        write(1, not_found, sizeof(not_found) - 1);
        return;
    }
    close(fd);
    execve(path, 0, 0);
}

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
            input[input_char_count] = '\0';
            if (input_char_count > 0)
            {
                run_command(input);
            }
            input_char_count = 0;
            write(1, shell_prompt, sizeof(shell_prompt) - 1);
            continue;
        }
        write(1, input + input_char_count, 1);
        input_char_count++;
    }
}
