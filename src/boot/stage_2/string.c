#include "stdint.h"

uint16_t strlen(const char* str)
{
   char* c = str;
   while(*c){
      c++; //namedrop
   }
   return (uint16_t)(c - str);
}
