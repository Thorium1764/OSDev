#pragma once
#include "stdint.h"

void putchr(int x, int y, char c);
void putcolor(int x, int y, uint8_t color);
char getchr(int x, int y);
uint8_t getcolor(int x, int y);
void setcursor(int x, int y);
void clrscr();
void scrollback(int lines);
void putc(char c);
void puts(const char* str);
void putn(int num);
void int_to_string(int num, char* buf);

