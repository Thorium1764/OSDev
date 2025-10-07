#include "stdint.h"
#include "disk.h"
#include "x86.h"

uint8_t DiskInit(Disk *disk, uint8_t drive_num)
{
   uint8_t type;
   uint16_t cylinders, sectors, heads;

   if (!x86_Disk_GetDriveParams(drive_num, &type, &cylinders, &sectors, &heads))
      return 0;

   disk->id = drive_num;
   disk->cylinders = cylinders;
   disk->sectors = sectors;
   disk->heads = heads;

   return 1;
}



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
      if (x86_DiskRead(disk->id, cylinder, sector, head, sectors, Data))
         return 1;

      x86_DiskReset(disk->id); //unreachable
   }

   return 0;
}

