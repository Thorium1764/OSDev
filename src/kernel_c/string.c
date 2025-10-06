#include "string.h"
#define size_t unsigned long //temporary while i dont have all the stdlib functions

size_t strlen(const char* str){
   char* p = str;
   while(*p) p++;
   return (p - str);
}

char* strcpy(char* dest, const char* src, int n){
   char* ptr = dest;
   int counter = 0;
   while (*src && counter < n){
      counter++;
      *ptr = *src;
      ptr++;
      src++;
   }
   *ptr = '\0';
   return ptr;
}
