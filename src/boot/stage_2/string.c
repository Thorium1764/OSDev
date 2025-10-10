#include "stdint.h"

uint16_t strlen(const char* str)
{
   char* c = str;
   while(*c){
      c++; //namedrop
   }
   return (uint16_t)(c - str);
}

const char* strchr(const char* str, char c)
{
   if (str == NULL)
      return NULL;

   while (*str){
      if (*str == c)
         return str;

      str++;
   }
   return NULL;
}

char* strcpy(char* dest, const char* src)
{
   if (dest == NULL)
      return NULL;
   
   if (src == NULL){
      *dest = '\0';
      return dest;
   }

   char* start = dest;

   while (*src){
      *dest = *src;
      dest++;
      src++;
   }

   *dest = '\0';
   return start;
}


int8_t strcmp(const char* p1, const char* p2)
{
   if (p1 == NULL && p2 == NULL)
      return 0;

   if (p1 == NULL)
      return -1;

   if (p2 == NULL)
      return 1;
   
   while (*p1 && *p2){
      if (*p1 == *p2){
         p1++;
         p2++;
      }
      if (*p1 > *p2)
         return 1;
      if (p1 < p2)
         return -1;
   }
   return 0;
}
