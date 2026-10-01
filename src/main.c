
#include "vga.h"
#include <stdint.h>
#include "font.h"
void kmain(void)
{
    initVGA();
    clear(RGB_INDEX(1,1,1));
    font_str("hey, nice balls", 50, 50, RGB_INDEX(5,5,5));
        __asm__ volatile ("hlt");
    }
