#include "mbr.h"
#include "disk.h"
#include "stdint.h"
#include "memory.h"

typedef struct
{
   uint8_t attributes;

   uint8_t chsStart[3];

   uint8_t part_type;

   uint8_t chsEnd[3];

   uint32_t lbaOfStart;

   uint32_t size;
} __attribute__((packed)) MBR_Entry;

void detectPartition(Partition * part, Disk* disk, void* partition)
{
   part->disk = disk;
   if (disk->id < 0x80) {
      part->offset = 0;
      part->size = (uint32_t)(disk->cylinders)*(uint32_t)(disk->heads)*(uint32_t)(disk->sectors);
   } else {
      MBR_Entry* entry = (MBR_Entry*)segoffset_to_linear(partition);
      part->offset = entry->lbaOfStart;
      part->size = entry->size;
   }
}

uint8_t Part_ReadSectors(Partition *part, uint32_t lba, uint8_t sectors, void *Data)
{
   return DiskRead(part->disk, lba + part->offset, sectors, Data);
}
   
