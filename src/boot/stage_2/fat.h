#pragma once
#include "stdint.h"
#include "mbr.h"

typedef struct
{
   uint8_t Name[11];
   uint8_t Attributes;
   uint8_t Reserved;
   uint8_t TimeTenths;
   uint16_t CreatedTime;
   uint16_t CreatedDate;
   uint16_t AccessedDate;
   uint16_t ModifiedTime;
   uint16_t ModifiedDate;
   uint16_t FirstClusterLow;
   uint16_t FirstClusterHigh;
   uint32_t Size;
} __attribute__((packed)) FAT_DirEntry;

typedef struct
{
   int Handle;
   uint8_t isDir;
   uint32_t Pos;
   uint32_t Size;
} FAT_FILE;

enum FAT_Attributes
{
   FAT_READ_ONLY = 0x1,
   FAT_HIDDEN = 0x2,
   FAT_SYSTEM = 0x4,
   FAT_VOL_ID = 0x08,
   FAT_DIR    = 0x10,
   FAT_ARCHIVE = 0x20
};

uint8_t DISK_INIT(Partition* disk);
FAT_FILE* FAT_OPEN(Partition* disk, const char* path);
uint32_t FAT_READ(Partition* disk, FAT_FILE* file, uint32_t size, void* Data);
uint8_t FAT_READ_ENTRY(Partition* disk, FAT_FILE* file, FAT_DirEntry* entry);
void FAT_CLOSE(FAT_FILE* file);
