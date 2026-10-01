
#include <stdint.h>

#include "defs.h"
#include "x86.h"

static void play_sound(uint32_t frequency)
{
    uint32_t divisor;
    uint8_t tmp;

    if (frequency == 0)
        return;

    divisor = 1193180 / frequency;

    outb(0x43, 0xB6);

    outb(0x42, (uint8_t)(divisor & 0xFF));
    outb(0x42, (uint8_t)((divisor >> 8) & 0xFF));

    tmp = inb(0x61);

    if ((tmp & 3) != 3)
    {
        outb(0x61, tmp | 3);
    }
}

static void nosound(void)
{
    uint8_t tmp = inb(0x61);

    outb(0x61, tmp & 0xFC);
}

void beep(void)
{
    play_sound(98);

    for (volatile uint32_t i = 0; i < 7000000; i++)
    {
        ;
    }

    nosound();
}