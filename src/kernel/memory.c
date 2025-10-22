#include "memory.h"
#include "stdint.h"

void* memcpy(void* dst, const void* src, uint16_t num)
{
   uint8_t* dest_byte = (uint8_t*)dst;
   const uint8_t* src_byte = (uint8_t*)src;

   for (uint16_t i = 0; i < num; i++){
        *(dest_byte + i) = *(src_byte + i);
   }

   return dst;
}

void* memset(void* ptr, int value, uint16_t num)
{
   uint8_t* dest_byte = (uint8_t*)ptr;
   for (uint16_t i = 0; i < num; i++)
        *(dest_byte + i) = (uint8_t)value;

   return ptr;
}

int8_t memcmp(const void* ptr1, const void* ptr2, uint16_t num)
{
   uint8_t* bytes1 = (uint8_t*)ptr1;
   uint8_t* bytes2 = (uint8_t*)ptr2;

   for (uint16_t i = 0; i < num; i++){
        if (*(bytes1 + i) > *(bytes2 + i))
            return 1;
        
        if (*(bytes1 + i) < *(bytes1 + i))
            return -1;
   }

   return 0;
}

void* segoffset_to_linear(void* addr)
{
   uint32_t offset = (uint32_t)(addr) & 0xFFFF;
   uint32_t segment = (uint32_t)(addr) >> 16;
   return (void*)(segment * 16 + offset);
}
