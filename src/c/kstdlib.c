#include "kstdlib.h"

void putc(char c, char color){
   static char* video = (char*)0xB8000;
   static int offset = 0;
   video[offset++] = c;
   video[offset++] = color;
}

void puts(const char* str){
   while(*str) putc(*str++, 0x07);
}

void int_to_string(int num, char* buf){
   int negative = 0;
   char* p = buf;

   if (num == 0){
      *p++ = '0';
      *p = '\0';
      return;
   }

   if (num < 0){
      negative = 1;
      num = -num;
      }
   
   while (num > 0){
      int digit = num % 10;
      *p++ = '0' + digit;
      num /= 10;
   }

   if (negative){*p++ = '-';}

   *p = '\0';

   char* start = buf;
   char* end = p - 1;
   while (start < end){
      char temp = *start;
      *start++ = *end;
      *end-- = temp;
   }
}

void putn(int num){
   char buf[12];
   int_to_string(num, buf);
   puts(buf);
}
