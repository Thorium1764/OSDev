#include "stdio.h"
#include "printc.h"

void putc(char c)
{
    x86_Video_WriteCharTeletype(c, 0);
}

void puts(const char* str)
{
    while(*str)
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
