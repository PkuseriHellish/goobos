#include <stdint.h>

#include "vga.h"
#include "font.h"
#include "defs.h"
#include "multiboot.h"

uint32_t multiboot_magic;
uint32_t multiboot_info_addr;

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 200

#define FONT_WIDTH    5
#define FONT_HEIGHT   8

#define INPUT_SIZE    128

#define BG_COLOR      COLOR_BLACK
#define FG_COLOR      COLOR_WHITE

static int cursor_x = 0;
static int cursor_y = 16;

static char input[INPUT_SIZE];
static int input_length = 0;


/* -------------------------------------------------- */
/* Basic drawing helpers                              */
/* -------------------------------------------------- */

static void
erase_char(int x, int y)
{
    for (int yy = 0; yy < FONT_HEIGHT+4; yy++) {
        for (int xx = 0; xx < FONT_WIDTH; xx++) {
            drawpx(
                x + xx,
                y + yy - 4,
                BG_COLOR
            );
        }
    }
}

static void scroll_up(void)
{
    /*
     * Move every row up by one font height.
     */
    for (int y = FONT_HEIGHT; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            uint8_t pixel = getpx(x, y);

            drawpx(
                x,
                y - FONT_HEIGHT,
                pixel
            );
        }
    }

    /*
     * Clear the new bottom area.
     */
    for (int y = SCREEN_HEIGHT - FONT_HEIGHT;
         y < SCREEN_HEIGHT;
         y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            drawpx(
                x,
                y,
                COLOR_BLACK
            );
        }
    }

    /*
     * Cursor is now on the new bottom line.
     */
    cursor_x = 0;
    cursor_y = SCREEN_HEIGHT - FONT_HEIGHT;
}


static void new_line(void)
{
    cursor_x = 0;
     cursor_y += FONT_HEIGHT;

    if (cursor_y + FONT_HEIGHT > SCREEN_HEIGHT)
    {
        scroll_up();
    }
}

static void putchar_shell(char c, uint8_t color)
{
    if (c == '\n')
    {
        new_line();
        return;
    }

    const font_glyph_t *glyph =
        font_get_glyph((uint32_t)(uint8_t)c);

    if (!glyph)
        return;

    font_char(
        (uint32_t)(uint8_t)c,
        cursor_x,
        cursor_y,
        color
    );

    cursor_x += glyph->advance;

    if (cursor_x + glyph->advance > SCREEN_WIDTH)
    {
        new_line();
    }
}

static void
print(const char *s, uint8_t color)
{
    while (*s)
        putchar_shell(*s++, color);
}


/* -------------------------------------------------- */
/* Tiny string functions                              */
/* -------------------------------------------------- */

static int
strcmp_local(const char *a, const char *b)
{
    while (*a && *a == *b) {
        a++;
        b++;
    }

    return (unsigned char)*a -
           (unsigned char)*b;
}


static int
starts_with(
    const char *str,
    const char *prefix
)
{
    while (*prefix) {
        if (*str != *prefix)
            return 0;

        str++;
        prefix++;
    }

    return 1;
}


/* -------------------------------------------------- */
/* Shell                                              */
/* -------------------------------------------------- */

static void
shell_prompt(void)
{
    print("> ",COLOR_YELLOW);
}


static void
shell_execute(char *cmd)
{
    if (cmd[0] == '\0')
        return;


    /* help */
    /* clear */
    if (strcmp_local(cmd, "clear") == 0) {

        clear(BG_COLOR);

        cursor_x = 0;
        cursor_y = 0;

        return;
    }


    /* about */

    /* echo */
    if (starts_with(cmd, "echo ")) {

        print(cmd + 5,COLOR_WHITE);
        putchar_shell('\n',0);

        return;
    }


    /* unknown command */
    print("Unknown command: ",COLOR_RED);
    print(cmd, COLOR_GRAY);
    putchar_shell('\n',0);
}


/* -------------------------------------------------- */
/* Main shell                                         */
/* -------------------------------------------------- */

void
kmain(void)
{
    initVGA();
   
    clear(BG_COLOR);

    cursor_x = 0;
    cursor_y = 0;

    print("Welcome to GoobOS!\n",COLOR_CYAN);
    print("Type 'help' for commands.\n\n",COLOR_WHITE);

    shell_prompt();

    
    while (1) {

        int c = kbdgetc();

        /*
         * No keyboard character.
         */
        if (c <= 0)
            continue;


        /*
         * Enter
         */
        if (c == '\n') {

            putchar_shell('\n',0);

            input[input_length] = '\0';

            shell_execute(input);

            input_length = 0;

            input[0] = '\0';

            shell_prompt();

            continue;
        }


        /*
         * Backspace
         */
        if (c == '\b') {

            if (input_length > 0) {

                input_length--;

                input[input_length] = '\0';

                cursor_x -= FONT_WIDTH;

                if (cursor_x < 0)
                    cursor_x = 0;

                erase_char(
                    cursor_x,
                    cursor_y
                );
            }

            continue;
        }


        /*
         * Ignore characters that are outside
         * the printable ASCII range.
         */
        if (c < 32 || c > 126)
            continue;


        /*
         * Don't overflow the command buffer.
         */
        if (input_length >= INPUT_SIZE - 1)
            continue;


        /*
         * Store character.
         */
        input[input_length++] = (char)c;
        input[input_length] = '\0';


        /*
         * Display character.
         */
        putchar_shell((char)c,COLOR_WHITE);
    }
}