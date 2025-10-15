#include "io.h"
#include "stdint.h"
#include "stdio.h"

const unsigned SCREEN_WIDTH = 80;
const unsigned SCREEN_HEIGHT = 25;
const uint8_t DEFAULT_COLOR = 0x7;

uint8_t* ScreenBuffer = (uint8_t*)0xB8000;
int ScreenX = 0, ScreenY = 0;

void putchr(int x, int y, char c)
{
   ScreenBuffer[2 * (y * SCREEN_WIDTH + x)] = c;
}

void putcolor(int x, int y, uint8_t color)
{
   ScreenBuffer[2 * (y * SCREEN_WIDTH + x) + 1] = color;
}

char getchr(int x, int y)
{
   return ScreenBuffer[2 * (y * SCREEN_WIDTH + x)];
}

uint8_t getcolor(int x, int y)
{
   return ScreenBuffer[2 * (y * SCREEN_WIDTH + x) + 1];
}

void clrscr()
{
   for (int y = 0; y < SCREEN_HEIGHT; y++)
      for (int x = 0; x < SCREEN_WIDTH; x++) 
      {
         putchr(x, y, '\0');
         putcolor(x, y, DEFAULT_COLOR);
      }

   ScreenX = 0;
   ScreenY = 0;
   setcursor(ScreenX, ScreenY);
}

void setcursor(int x, int y)
{
   int position = y * SCREEN_WIDTH + x;
   
    i686_out(0x3D4, 0x0F);
    i686_out(0x3D5, (uint8_t)(position & 0xFF));
    i686_out(0x3D4, 0x0E);
    i686_out(0x3D5, (uint8_t)((position >> 8) & 0xFF));
}

void scrollback(int lines)
{
    for (int y = lines; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            putchr(x, y - lines, getchr(x, y));
            putcolor(x, y - lines, getcolor(x, y));
        }

    for (int y = SCREEN_HEIGHT - lines; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            putchr(x, y, '\0');
            putcolor(x, y, DEFAULT_COLOR);
        }

    ScreenY -= lines;
}

void putc(char c)
{
    i686_out(0xE9, c);
    switch (c)
    {
        case '\n':
            ScreenX = 0;
            ScreenY++;
            break;
    
        case '\t':
            for (int i = 0; i < 4 - (ScreenX % 4); i++)
                putc(' ');
            break;

        case '\r':
            ScreenX = 0;
            break;

        default:
            putchr(ScreenX, ScreenY, c);
            ScreenX++;
            break;
    }

    if (ScreenX >= SCREEN_WIDTH)
    {
        ScreenY++;
        ScreenX = 0;
    }
    if (ScreenY >= SCREEN_HEIGHT)
        scrollback(1);

    setcursor(ScreenX, ScreenY);
}

void puts(const char* str)
{
   while (*str)
   {
      putc(*str);
      str++;
   }
}

void int_to_string(int num, char* buf) {
    int negative = 0;
    char* p = buf;
    char* start;
    char* end;
    char temp;
    int digit;

    if (num == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }

    if (num < 0) {
        negative = 1;
        num = -num;
    }

    while (num > 0) {
        digit = num % 10;
        *p++ = '0' + digit;
        num /= 10;
    }

    if (negative) {
        *p++ = '-';
    }

    *p = '\0';

    start = buf;
    end = p - 1;

    while (start < end) {
        temp = *start;
        *start++ = *end;
        *end-- = temp;
    }
}

void putn(int num) {
    char buf[12];
    int_to_string(num, buf);
    puts(buf);
}

