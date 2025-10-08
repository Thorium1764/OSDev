#include "x86.h"
#include "bootparams.h"
#include "stdint.h"

#define MAX_REGIONS 256

MemoryRegion Regions[MAX_REGIONS];
int RegionCount;

void MemDetect(MemoryInfo* info)
{
   E820_MemBlock block;
   uint32_t id = 0;
   int next;
   RegionCount = 0;
   next = x86_GetNextE820BLock(&block, &id);

   while (next > 0 && id != 0)
   {
      Regions[RegionCount].Begin = block.base;
      Regions[RegionCount].Length = block.length;
      Regions[RegionCount].Type = block.type;
      Regions[RegionCount].ACPI = block.ACPI;
      RegionCount++;
      next = x86_GetNextE820BLock(&block, &id);
   }
   
   info->Regions = Regions;
   info->RegionCount = RegionCount;
}
