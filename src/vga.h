#include <stdint.h>
#define WEB_0  0x00
#define WEB_1  0x33
#define WEB_2  0x66
#define WEB_3  0x99
#define WEB_4  0xCC
#define WEB_5  0xFF


/*
 * Convert a 6x6x6 RGB coordinate into
 * a VGA palette index.
 *
 * r, g, b must be 0-5.
 */
#define RGB_INDEX(r, g, b) \
    ((r) * 36 + (g) * 6 + (b))


/*
 * Common colors
 */
#define COLOR_BLACK    RGB_INDEX(0, 0, 0)
#define COLOR_RED      RGB_INDEX(5, 0, 0)
#define COLOR_GREEN    RGB_INDEX(0, 5, 0)
#define COLOR_BLUE     RGB_INDEX(0, 0, 5)

#define COLOR_YELLOW   RGB_INDEX(5, 5, 0)
#define COLOR_CYAN     RGB_INDEX(0, 5, 5)
#define COLOR_MAGENTA  RGB_INDEX(5, 0, 5)

#define COLOR_WHITE    RGB_INDEX(5, 5, 5)

#define COLOR_GRAY     RGB_INDEX(3, 3, 3)
#define COLOR_DARK_GRAY RGB_INDEX(2, 2, 2)
#define COLOR_LIGHT_GRAY RGB_INDEX(4, 4, 4)

void initVGA();
void clear(uint8_t color);
void drawpx(int x,int y, uint8_t color);