
#include <stdint.h>

#include "shell.h"
#include "terminal.h"
#include "defs.h"
#include "vga.h"
#include "sfs.h"

#define INPUT_SIZE 128

static char input[INPUT_SIZE];
static int input_length = 0;



static int strcmp_local(const char *a, const char *b)
{
    while (*a && *a == *b)
    {
        a++;
        b++;
    }

    return (unsigned char)*a -
           (unsigned char)*b;
}

static int starts_with(
    const char *str,
    const char *prefix
)
{
    while (*prefix)
    {
        if (*str != *prefix)
            return 0;

        str++;
        prefix++;
    }

    return 1;
}

static void shell_prompt(void)
{
    terminal_print("] ", COLOR_YELLOW);
}

static void shell_execute(char *cmd)
{
    if (cmd[0] == '\0')
        return;

    if (strcmp_local(cmd, "help") == 0)
    {
        terminal_print("Commands:\n", COLOR_WHITE);
        terminal_print("  help\n", COLOR_WHITE);
        terminal_print("  clear\n", COLOR_WHITE);
        terminal_print("  echo <text>\n", COLOR_WHITE);

        terminal_print("  cat <file>\n", COLOR_WHITE);

        terminal_print("  ls\n", COLOR_WHITE);

        return;
    }
     if (strcmp_local(cmd, "goob") == 0)
    {
        terminal_print("goob\n", COLOR_WHITE);
        return;
    }
   if (strcmp_local(cmd, "ls") == 0)
{
    for (int i = 0; i < fs_count(); i++)
    {
        const struct node *n = fs_get(i);

        if (n)
        {
            terminal_print(n->name, COLOR_WHITE);
            terminal_print("\n", COLOR_WHITE);
        }
    }

    return;
}
if (starts_with(cmd, "cat "))
{
    const struct node *n = fs_find(cmd + 4);

    if (!n)
    {
        terminal_print("file not found\n", COLOR_WHITE);
        return;
    }

    for (uint32_t i = 0; i < n->size; i++)
    {
        char c[2] = { (char)n->data[i], '\0' };
        terminal_print(c, COLOR_WHITE);
    }

    terminal_print("\n", COLOR_WHITE);
    return;
}


    if (strcmp_local(cmd, "clear") == 0)
    {
        terminal_clear();
        return;
    }

    if (starts_with(cmd, "echo "))
    {
        terminal_print(
            cmd + 5,
            COLOR_WHITE
        );

        terminal_newline();

        return;
    }

    terminal_print(
        "Unknown command: ",
        COLOR_RED
    );

    terminal_print(
        cmd,
        COLOR_GRAY
    );

    terminal_newline();
}

void shell_init(void)
{
    input_length = 0;
    input[0] = '\0';

    shell_prompt();
}

void shell_input(int c)
{
    /*
     * Enter
     */
    if (c == '\n')
    {
        terminal_newline();

        input[input_length] = '\0';

        shell_execute(input);

        input_length = 0;
        input[0] = '\0';

        shell_prompt();

        return;
    }

    /*
     * Backspace
     */
    if (c == '\b')
    {
        if (input_length > 0)
        {
            input_length--;

            input[input_length] = '\0';

            terminal_backspace();
        }

        return;
    }

    /*
     * Ignore non-printable characters.
     */
    if (c < 32 || c > 126)
        return;

    /*
     * Prevent input buffer overflow.
     */
    if (input_length >= INPUT_SIZE - 1)
    {
        beep();
        return;
    }

    /*
     * Store character.
     */
    input[input_length++] = (char)c;
    input[input_length] = '\0';

    /*
     * Display character.
     */
    terminal_putchar(
        (char)c,
        COLOR_WHITE
    );
}