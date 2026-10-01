
#include "vga.h"
#include "defs.h"
#include "shell.h"
#include "terminal.h"
#include "sfs.h"

void kmain(void)
{
    initVGA();
    archive_init();
    clear(0);
    terminal_init();

    terminal_print(
        "\nWelcome to GoobOS!\n",
        COLOR_CYAN
    );

    terminal_print(
        "Type 'help' for commands.\n\n",
        COLOR_WHITE
    );

    shell_init();

    while (1)
    {
        int c = kbdgetc();

        if (c > 0)
        {
            shell_input(c);
        }
    }
}
