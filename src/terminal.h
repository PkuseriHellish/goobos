
#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>

void terminal_init(void);

void terminal_putchar(char c, uint8_t color);
void terminal_print(const char *s, uint8_t color);

void terminal_newline(void);
void terminal_backspace(void);
void terminal_clear(void);

int terminal_get_x(void);
int terminal_get_y(void);

#endif