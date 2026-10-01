#include <stddef.h>
#include <stdint.h>
#include "serial.h"
#include "vga.h"
#include <stddef.h>
#include <stdint.h>
#include "serial.h"
#include "vga.h"

/*
 * VGA DAC
 *
 * RGB input: 0-255
 * VGA DAC:   0-63
 */
static void set_palette_color(
    uint8_t index,
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    outb(0x3C8, index);

    outb(0x3C9, r >> 2);
    outb(0x3C9, g >> 2);
    outb(0x3C9, b >> 2);
}


/*
 * Web-safe RGB levels:
 *
 * 00
 * 33
 * 66
 * 99
 * CC
 * FF
 */



void init_palette(void)
{
    static const uint8_t web[] = {
        0x00,
        0x33,
        0x66,
        0x99,
        0xCC,
        0xFF
    };

    uint16_t index = 0;

    /*
     * 216 web-safe colors
     *
     * 6 red levels
     * 6 green levels
     * 6 blue levels
     *
     * 6 * 6 * 6 = 216
     */
    for (uint8_t r = 0; r < 6; r++)
    {
        for (uint8_t g = 0; g < 6; g++)
        {
            for (uint8_t b = 0; b < 6; b++)
            {
                set_palette_color(
                    index++,
                    web[r],
                    web[g],
                    web[b]
                );
            }
        }
    }

    /*
     * 40 grayscale colors
     *
     * Palette indexes:
     *
     * 216-255
     */
    for (uint16_t i = 0; i < 40; i++)
    {
        uint8_t gray = (uint8_t)((i * 255) / 39);

        set_palette_color(
            index++,
            gray,
            gray,
            gray
        );
    }
}
void initVGA()
{
     /*
     * VGA Mode 13h
     *
     * 320x200
     * 256 colors
     * 1 byte per pixel
     * framebuffer: 0xA0000
     */

    static const uint8_t sequencer[] = {
        0x03,
        0x01,
        0x0F,
        0x00,
        0x0E
    };

    static const uint8_t crtc[] = {
    0x5F, 0x4F, 0x50, 0x82,
    0x54, 0x80, 0xBF, 0x1F,
    0x00, 0x41, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x9C, 0x0E, 0x8F, 0x28,
    0x40, 0x96, 0xB9, 0xA3,
    0xFF
};
    static const uint8_t graphics[] = {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x40,
        0x05,
        0x0F,
        0xFF
    };

    static const uint8_t attributes[] = {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0D, 0x0E, 0x0F,
        0x41, 0x00, 0x0F, 0x00,
        0x00
    };


    /*
     * Miscellaneous output register
     */
    outb(0x3C2, 0x63);


    /*
     * Reset VGA sequencer
     */
    outb(0x3C4, 0x00);
    outb(0x3C5, 0x01);


    /*
     * Sequencer registers 1-4
     */
    for (int i = 1; i < 5; i++)
    {
        outb(0x3C4, i);
        outb(0x3C5, sequencer[i]);
    }


    /*
     * Restart sequencer
     */
    outb(0x3C4, 0x00);
    outb(0x3C5, 0x03);


    /*
     * Unlock CRTC registers
     */
    outb(0x3D4, 0x03);
    outb(0x3D5, 0x80);

    outb(0x3D4, 0x11);
    outb(0x3D5, 0x00);


    /*
     * CRTC registers
     */
    for (int i = 0; i < 25; i++)
    {
        outb(0x3D4, i);
        outb(0x3D5, crtc[i]);
    }


    /*
     * Graphics controller
     */
    for (int i = 0; i < 9; i++)
    {
        outb(0x3CE, i);
        outb(0x3CF, graphics[i]);
    }


    /*
     * Attribute controller
     */
    for (int i = 0; i < 21; i++)
    {
        inb(0x3DA);

        outb(0x3C0, i);
        outb(0x3C0, attributes[i]);
    }


    /*
     * Enable video output
     */
    inb(0x3DA);
    outb(0x3C0, 0x20);
    init_palette();
}
volatile uint8_t *vram = (volatile uint8_t *)0xA0000;
void clear(uint8_t color)
{
      for (int i = 0; i < 320*200; i++) {
      vram[i] = color;
      }
}
void drawpx(int x,int y, uint8_t color)
{
    vram[y * 320 + x] = color;
}