#ifndef FONT_H
#define FONT_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t codepoint;

    uint8_t width;
    uint8_t height;

    int8_t x_offset;
    int8_t y_offset;

    uint8_t advance;

    const uint8_t *bitmap;
} font_glyph_t;

const font_glyph_t *font_get_glyph(uint32_t codepoint);

void font_char(
    uint32_t codepoint,
    int x,
    int y,
    uint8_t color
);

void font_str(
    const char *str,
    int x,
    int y,
    uint8_t color
);

#endif
