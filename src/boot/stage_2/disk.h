#pragma once

#include "stdint.h"

typedef struct{
   uint8_t id;
   uint16_t cylinders;
   uint16_t sectors;
   uint16_t heads;
} Disk;

uint8_t DiskInit(Disk* disk, uint8_t drive_number);
uint8_t DiskRead(Disk* disk, uint32_t lba, uint8_t sectors, void* DataOut);
