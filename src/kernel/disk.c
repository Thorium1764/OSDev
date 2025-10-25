#include "stdint.h"
#include "disk.h"
#include "file.h"

void lbaToChs(Disk* disk, uint32_t lba, uint16_t* cylinder, uint16_t* sector, uint16_t* head)
{
   *sector = lba % disk->sectors + 1;
   *cylinder = (lba / disk->sectors) / disk->heads;
   *head = (lba / disk->sectors) % disk->heads;
}

uint8_t DiskRead(Disk* disk, uint32_t lba, uint8_t sectors, void* Data)
{
   uint16_t cylinder, sector, head;

   lbaToChs(disk, lba, &cylinder, &sector, &head);

   for (int i = 0; i < 3; i++) //retry 3 times
   {
      if(x86_DiskRead(disk->id, cylinder, sector, head, sectors, Data))
         return 1;
   }
   
   return 0;
}

