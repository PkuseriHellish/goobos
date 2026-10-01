#include <stdint.h>

#include "defs.h"
#include "sfs.h"
#include "shell.h"
#include "terminal.h"
#include "vga.h"


/* --------------------------------------------------------------------------
 * Configuration
 * -------------------------------------------------------------------------- */

#define INPUT_SIZE 128
#define DEFAULT_FILE_SIZE 1024


/* --------------------------------------------------------------------------
 * Shell state
 * -------------------------------------------------------------------------- */

static char input[INPUT_SIZE];
static int input_length = 0;


/* --------------------------------------------------------------------------
 * String helpers
 * -------------------------------------------------------------------------- */

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


static int starts_with(const char *str, const char *prefix)
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


/* --------------------------------------------------------------------------
 * Terminal / prompt
 * -------------------------------------------------------------------------- */

static void shell_prompt(void)
{
    terminal_print("$ ", COLOR_YELLOW);
}


/* --------------------------------------------------------------------------
 * help
 * -------------------------------------------------------------------------- */

static void command_help(void)
{
    terminal_print("GoobOS Shell\n", COLOR_CYAN);
    terminal_print("\n", COLOR_WHITE);

    terminal_print("General commands:\n", COLOR_YELLOW);
    terminal_print("  help                 Show this help\n", COLOR_WHITE);
    terminal_print("  clear                Clear the terminal\n", COLOR_WHITE);
    terminal_print("  goob                 Print goob\n", COLOR_WHITE);

    terminal_print("\n", COLOR_WHITE);

    terminal_print("Filesystem commands:\n", COLOR_YELLOW);
    terminal_print("  ls                   List files\n", COLOR_WHITE);
    terminal_print("  touch <file>         Create a file\n", COLOR_WHITE);
    terminal_print("  cat <file>           Print a file\n", COLOR_WHITE);
    terminal_print("  write <file> <text>  Write text to a file\n", COLOR_WHITE);

    terminal_print("\n", COLOR_WHITE);

    terminal_print("Examples:\n", COLOR_YELLOW);
    terminal_print("  ls\n", COLOR_GRAY);
    terminal_print("  touch hello.txt\n", COLOR_GRAY);
    terminal_print("  write hello.txt hello world\n", COLOR_GRAY);
    terminal_print("  cat hello.txt\n", COLOR_GRAY);
}


/* --------------------------------------------------------------------------
 * ls
 * -------------------------------------------------------------------------- */

static void command_ls(void)
{
    int count = fs_count();

    if (count == 0)
    {
        terminal_print("filesystem is empty\n", COLOR_GRAY);
        return;
    }

    for (int i = 0; i < count; i++)
    {
        const struct node *n = fs_get(i);

        if (!n)
            continue;

        terminal_print(n->name, COLOR_WHITE);
        terminal_print("\n", COLOR_WHITE);
    }
}


/* --------------------------------------------------------------------------
 * cat
 * -------------------------------------------------------------------------- */

static void command_cat(const char *name)
{
    if (!*name)
    {
        terminal_print(
            "usage: cat <file>\n",
            COLOR_WHITE
        );

        return;
    }

    const struct node *n = fs_find(name);

    if (!n)
    {
        terminal_print(
            "file not found\n",
            COLOR_RED
        );

        return;
    }

    for (uint32_t i = 0; i < n->size; i++)
    {
        char c[2];

        c[0] = (char)n->data[i];
        c[1] = '\0';

        terminal_print(c, COLOR_WHITE);
    }

    terminal_newline();
}

/* --------------------------------------------------------------------------
 * touch
 * -------------------------------------------------------------------------- */

static void command_touch(const char *name)
{
    if (!*name)
    {
        terminal_print(
            "usage: touch <file>\n",
            COLOR_WHITE
        );

        return;
    }

    if (fs_create(name, DEFAULT_FILE_SIZE))
    {
        terminal_print(
            "created\n",
            COLOR_GREEN
        );
    }
    else
    {
        terminal_print(
            "cannot create file :(\n",
            COLOR_RED
        );
    }
}


/* --------------------------------------------------------------------------
 * write
 * -------------------------------------------------------------------------- */

static void command_write(char *args)
{
    /*
     * Expected:
     *
     *     write <file> <text>
     *
     * Example:
     *
     *     write hello.txt hello world
     */

    char *space = args;

    /*
     * Find the space between filename and text.
     */
    while (*space && *space != ' ')
        space++;

    if (*space == '\0')
    {
        terminal_print(
            "usage: write <file> <text>\n",
            COLOR_WHITE
        );

        return;
    }

    /*
     * Split the command:
     *
     * hello.txt hello world
     *          ^
     *
     * into:
     *
     * filename = "hello.txt"
     * text     = "hello world"
     */
    *space = '\0';

    const char *filename = args;
    const char *text = space + 1;

    /*
     * Find the file.
     */
    struct node *n = fs_find(filename);

    if (!n)
    {
        terminal_print(
            "file not found\n",
            COLOR_RED
        );

        return;
    }

    /*
     * Calculate text length.
     */
    uint32_t len = 0;

    while (text[len])
        len++;

    /*
     * Write the data.
     */
    uint32_t written = fs_write(
        n,
        (const uint8_t *)text,
        len
    );

    /*
     * fs_write() cannot grow a file.
     */
    if (written < len)
    {
        terminal_print(
            "cannot write file :(\n",
            COLOR_RED
        );

        return;
    }

    terminal_print(
        "written\n",
        COLOR_GREEN
    );
}


/* --------------------------------------------------------------------------
 * echo
 * -------------------------------------------------------------------------- */

static void command_echo(const char *text)
{
    terminal_print(
        text,
        COLOR_WHITE
    );

    terminal_newline();
}


/* --------------------------------------------------------------------------
 * Main command dispatcher
 * -------------------------------------------------------------------------- */

static void shell_execute(char *cmd)
{
    /*
     * Empty command.
     */
    if (cmd[0] == '\0')
        return;


    /*
     * help
     */
    if (strcmp_local(cmd, "help") == 0)
    {
        command_help();
        return;
    }


    /*
     * goob
     */
    if (strcmp_local(cmd, "goob") == 0)
    {
        terminal_print(
            "goob\n",
            COLOR_WHITE
        );

        return;
    }


    /*
     * clear
     */
    if (strcmp_local(cmd, "clear") == 0)
    {
        terminal_clear();
        return;
    }


    /*
     * ls
     */
    if (strcmp_local(cmd, "ls") == 0)
    {
        command_ls();
        return;
    }


    /*
     * cat <file>
     */
    if (starts_with(cmd, "cat "))
    {
        command_cat(cmd + 4);
        return;
    }


    /*
     * touch <file>
     */
    if (starts_with(cmd, "touch "))
    {
        command_touch(cmd + 6);
        return;
    }


    /*
     * write <file> <text>
     */
    if (starts_with(cmd, "write "))
    {
        command_write(cmd + 6);
        return;
    }

    /*
     * echo <text>
     */
    if (starts_with(cmd, "echo "))
    {
        command_echo(cmd + 5);
        return;
    }


    /*
     * Unknown command.
     */
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


/* --------------------------------------------------------------------------
 * Shell initialization
 * -------------------------------------------------------------------------- */

void shell_init(void)
{
    input_length = 0;
    input[0] = '\0';

    shell_prompt();
}


/* --------------------------------------------------------------------------
 * Keyboard input
 * -------------------------------------------------------------------------- */

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
