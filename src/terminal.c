#include <stdint.h>

#include "terminal.h"
#include "vga.h"
#include "font.h"

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 200

#define FONT_HEIGHT   8
#define MAX_HISTORY   256

#define BG_COLOR      COLOR_BLACK

typedef struct
{
    int x;
    int y;

    int width;
    int height;

    int x_offset;
    int y_offset;

} char_position_t;


static int cursor_x = 0;
static int cursor_y = 0;

static char_position_t history[MAX_HISTORY];
static int history_count = 0;


/* -------------------------------------------------- */
/* History                                            */
/* -------------------------------------------------- */

static void history_clear(void)
{
    history_count = 0;
}

static void history_push(
    int x,
    int y,
    const font_glyph_t *glyph
)
{
    if (history_count >= MAX_HISTORY)
        return;

    history[history_count].x = x;
    history[history_count].y = y;

    history[history_count].width = glyph->width;
    history[history_count].height = glyph->height;

    history[history_count].x_offset = glyph->x_offset;
    history[history_count].y_offset = glyph->y_offset;

    history_count++;
}

static int history_pop(char_position_t *out)
{
    if (history_count <= 0)
        return 0;

    history_count--;

    *out = history[history_count];

    return 1;
}


/* -------------------------------------------------- */
/* Erasing                                            */
/* -------------------------------------------------- */

static void erase_glyph(const char_position_t *pos)
{
    for (int yy = 0; yy < pos->height; yy++)
    {
        for (int xx = 0; xx < pos->width; xx++)
        {
            int px =
                pos->x +
                pos->x_offset +
                xx;

            int py =
                pos->y +
                pos->y_offset +
                yy;

            if (px >= 0 && px < SCREEN_WIDTH &&
                py >= 0 && py < SCREEN_HEIGHT)
            {
                drawpx(px, py, BG_COLOR);
            }
        }
    }
}


/* -------------------------------------------------- */
/* Scrolling                                          */
/* -------------------------------------------------- */

static void scroll_up(void)
{
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

    for (int y = SCREEN_HEIGHT - FONT_HEIGHT;
         y < SCREEN_HEIGHT;
         y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            drawpx(x, y, BG_COLOR);
        }
    }

    cursor_x = 0;
    cursor_y = SCREEN_HEIGHT - FONT_HEIGHT;

    /*
     * The old character positions no longer match
     * the pixels after scrolling.
     */
    history_clear();
}


/* -------------------------------------------------- */
/* Internal line movement                             */
/* -------------------------------------------------- */

static void wrap_line(void)
{
    cursor_x = 0;
    cursor_y += FONT_HEIGHT;

    if (cursor_y + FONT_HEIGHT > SCREEN_HEIGHT)
    {
        scroll_up();
    }
}


/* -------------------------------------------------- */
/* Terminal                                           */
/* -------------------------------------------------- */

void terminal_init(void)
{
    cursor_x = 0;
    cursor_y = 0;

    history_clear();
}

void terminal_clear(void)
{
    clear(BG_COLOR);

    cursor_x = 0;
    cursor_y = 0;

    history_clear();
}

int terminal_get_x(void)
{
    return cursor_x;
}

int terminal_get_y(void)
{
    return cursor_y;
}


/*
 * This is a REAL newline.
 *
 * It starts a new shell/output line, so characters
 * before it should not be reachable with backspace.
 */
void terminal_newline(void)
{
    wrap_line();

    history_clear();
}


void terminal_putchar(char c, uint8_t color)
{
    if (c == '\n')
    {
        terminal_newline();
        return;
    }

    const font_glyph_t *glyph =
        font_get_glyph((uint32_t)(uint8_t)c);

    if (!glyph)
        return;


    /*
     * Remember where this character started.
     */
    int x = cursor_x;
    int y = cursor_y;


    /*
     * Draw it.
     */
    font_char(
        (uint32_t)(uint8_t)c,
        x,
        y,
        color
    );


    /*
     * Remember its exact position and dimensions.
     */
    history_push(
        x,
        y,
        glyph
    );


    /*
     * Advance by the font's actual advance width.
     */
    cursor_x += glyph->advance;


    /*
     * Automatic wrapping.
     *
     * IMPORTANT:
     * Do NOT clear history here.
     *
     * This allows backspace to cross the wrapped line.
     */
    if (cursor_x + glyph->advance > SCREEN_WIDTH)
    {
        wrap_line();
    }
}


void terminal_print(const char *s, uint8_t color)
{
    while (*s)
    {
        terminal_putchar(*s++, color);
    }
}


/* -------------------------------------------------- */
/* Backspace                                          */
/* -------------------------------------------------- */

void terminal_backspace(void)
{
    char_position_t pos;

    if (!history_pop(&pos))
        return;


    /*
     * Remove exactly the pixels belonging to
     * the previous glyph.
     */
    erase_glyph(&pos);


    /*
     * Restore the cursor to that glyph's position.
     *
     * This also moves the cursor back onto the
     * previous visual line after a wrap.
     */
    cursor_x = pos.x;
    cursor_y = pos.y;
}
