#pragma once 
#include "disk.h"
#include "stdint.h"

typedef struct 
{
   Disk* disk;
   uint32_t offset;
   uint32_t size;
} Partition;

void detectPartition(Partition* part, Disk* disk, void* partitionEntry);
uint8_t Part_ReadSectors(Partition* part, uint32_t lba, uint8_t sectors, void* Data);
