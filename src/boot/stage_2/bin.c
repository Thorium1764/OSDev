#include "stdint.h"
#include "mbr.h"
#include "stdio.h"
#include "memory.h"
#include "memdefs.h"
#include "fat.h"

uint8_t BIN_Read(Partition *part, const char *path, void **entryPoint)
{
   FAT_FILE* file = FAT_OPEN(part, path);
   if (!file){
      puts("BIN load error: Could not load Kernel\r\n");
      return 0;
   }

   uint8_t* load_address = (uint8_t*)MEMORY_KERNEL_ADDR;
   uint8_t* buffer = MEMORY_KERNEL_LOAD;
   uint32_t total = 0;
   uint32_t loaded;

   while ((loaded = FAT_READ(part, file, MEMORY_KERNEL_SIZE, buffer)) > 0)
   {
      memcpy(load_address, buffer, loaded);
      load_address += loaded;
      total += loaded;
   }

   FAT_CLOSE(file);
   return 1;
}


